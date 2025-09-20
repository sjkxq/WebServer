#include "WebServer.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "http/HttpServer.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "ConnectionManager.hpp"
#include "http/HealthCheckController.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <vector>
#include <openssl/err.h>

namespace webserver {

bool WebServer::initSSLContext() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    const SSL_METHOD* method = TLS_server_method();
    sslContext_ = SSL_CTX_new(method);
    if (!sslContext_) {
        LOG_ERROR("Failed to create SSL context");
        return false;
    }

    // 加载证书和私钥
    if (SSL_CTX_use_certificate_file(sslContext_, config_.get<std::string>("ssl_cert", "").c_str(), SSL_FILETYPE_PEM) <= 0) {
        LOG_ERROR("Failed to load SSL certificate");
        ERR_print_errors_fp(stderr);
        return false;
    }

    if (SSL_CTX_use_PrivateKey_file(sslContext_, config_.get<std::string>("ssl_key", "").c_str(), SSL_FILETYPE_PEM) <= 0) {
        LOG_ERROR("Failed to load SSL private key");
        ERR_print_errors_fp(stderr);
        return false;
    }

    if (!SSL_CTX_check_private_key(sslContext_)) {
        LOG_ERROR("Private key does not match the certificate");
        return false;
    }

    return true;
}

void WebServer::cleanupSSL() {
    if (sslContext_) {
        SSL_CTX_free(sslContext_);
        sslContext_ = nullptr;
    }
    EVP_cleanup();
}

WebServer::WebServer(const Config& config) 
    : port_(config.get<int>("port", 8080)), 
      running_(false), 
      config_(config),
      sslContext_(nullptr) {
    connectionManager_ = std::make_unique<ConnectionManager>(config_);
    router_ = std::make_unique<Router>();
}

WebServer::~WebServer() {
    stop();
}

bool WebServer::start() {
    // 如果配置了HTTPS，初始化SSL
    if (config_.get<bool>("https_enabled", false)) {
        if (!initSSLContext()) {
            LOG_ERROR("Failed to initialize SSL context");
            return false;
        }
        LOG_INFO("HTTPS enabled with SSL/TLS");
    }

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        LOG_ERROR("Failed to create socket");
        return false;
    }

    // 设置socket选项
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        LOG_ERROR("Failed to set socket options");
        close(serverSocket);
        return false;
    }

    // 绑定地址和端口
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<uint16_t>(port_));
    if (bind(serverSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        LOG_ERROR("Failed to bind socket");
        close(serverSocket);
        return false;
    }

    // 监听连接
    if (listen(serverSocket, 10) == -1) {
        LOG_ERROR("Failed to listen on socket");
        close(serverSocket);
        return false;
    }

    LOG_INFO("Server started on port " + std::to_string(port_));
    running_ = true;

    // 接受连接
    while (running_) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
        
        if (clientSocket == -1) {
            if (running_) {
                LOG_ERROR("Failed to accept connection");
            }
            continue;
        }

        // 获取客户端IP地址
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
        
        // 处理连接
        connectionManager_->addConnection(clientSocket, std::string(clientIP), [this, clientSocket]() {
            this->handleConnection(clientSocket);
        });
    }

    close(serverSocket);
    return true;
}

void WebServer::stop() {
    running_ = false;
    connectionManager_->stopAll();
    cleanupSSL();
}

void WebServer::addRoute(const std::string& path, Router::RequestHandler handler) {
    router_->addRoute(path, handler);
}

void WebServer::handleConnection(int clientSocket) {
    SSL* ssl = nullptr;
    if (sslContext_) {
        ssl = SSL_new(sslContext_);
        SSL_set_fd(ssl, clientSocket);
        
        if (SSL_accept(ssl) <= 0) {
            LOG_ERROR("SSL handshake failed");
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            close(clientSocket);
            return;
        }
    }

    // 初始化连接状态
    bool keepAlive = false;
    int maxRequests = config_.get<int>("server.max_requests_per_connection", 100);
    int requestCount = 0;
    
    // 更新连接活动时间
    connectionManager_->updateActivity(clientSocket);

    // 处理连接上的多个请求
    while (requestCount < maxRequests) {
        // 设置非阻塞模式以支持超时
        struct timeval tv;
        tv.tv_sec = config_.get<int>("server.timeout", 60);
        tv.tv_usec = 0;
        
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(clientSocket, &readfds);
        
        // 等待数据可读或超时
        int selectResult = select(clientSocket + 1, &readfds, NULL, NULL, &tv);
        if (selectResult <= 0) {
            // 超时或错误
            break;
        }
        
        // 读取请求
        std::vector<char> buffer(4096);
        const auto readSize = static_cast<int>(std::min(buffer.size() - 1, static_cast<size_t>(INT_MAX)));
        ssize_t bytesRead = ssl ? SSL_read(ssl, buffer.data(), readSize)
                                : recv(clientSocket, buffer.data(), static_cast<size_t>(readSize), 0);
        if (bytesRead <= 0) {
            break;
        }
        buffer[static_cast<size_t>(bytesRead)] = '\0';
        std::string request(buffer.data());
        
        // 更新连接活动时间
        connectionManager_->updateActivity(clientSocket);
        requestCount++;
        
        // 使用HttpParser解析请求
        std::string path;
        std::map<std::string, std::string> headers;
        std::string body;
        std::tie(std::ignore, path, headers, body) = HttpParser::parseRequest(request);
        LOG_INFO("Received request for path: " + path);
        
        // 检查Connection头，确定是否保持连接
        keepAlive = false;
        if (headers.count("Connection") > 0) {
            keepAlive = (headers["Connection"] == "keep-alive");
        }
        
        // 设置连接的保活状态
        connectionManager_->setKeepAlive(clientSocket, keepAlive);
        
        // 处理请求
        bool found;
        std::string content;
        std::tie(found, content) = router_->handleRequest(path, headers, body);
        
        // 使用HttpParser构建响应
        std::string response;
        std::map<std::string, std::string> responseHeaders;
        
        // 添加Connection头
        responseHeaders["Connection"] = keepAlive ? "keep-alive" : "close";
        
        // 如果是保活连接，添加Keep-Alive头
        if (keepAlive) {
            int timeout = config_.get<int>("server.keep_alive_timeout", 5);
            int max = maxRequests - requestCount;
            responseHeaders["Keep-Alive"] = "timeout=" + std::to_string(timeout) + 
                                          ", max=" + std::to_string(max);
        }
        
        if (found) {
            // 检查是否需要分块传输
            bool useChunked = headers.count("Transfer-Encoding") > 0 && 
                             headers["Transfer-Encoding"] == "chunked";
            
            response = useChunked ? 
                HttpParser::buildChunkedResponse(HttpStatus::OK, content, responseHeaders) :
                HttpParser::buildResponse(HttpStatus::OK, content, responseHeaders);
        } else {
            response = HttpParser::buildResponse(HttpStatus::NOT_FOUND, 
                "<html><body><h1>404 Not Found</h1></body></html>", responseHeaders);
        }
        
        // 发送响应
        if (ssl) {
            const auto writeSize = static_cast<int>(std::min(response.size(), static_cast<size_t>(INT_MAX)));
            SSL_write(ssl, response.c_str(), writeSize);
        } else {
            send(clientSocket, response.c_str(), static_cast<size_t>(response.size()), 0);
        }
        
        // 如果不保持连接，退出循环
        if (!keepAlive) {
            break;
        }
    }
    
    // 关闭连接
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    close(clientSocket);
}

} // namespace webserver