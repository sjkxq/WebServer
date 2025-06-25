#include "WebServer.hpp"
#include "Logger.hpp"
#include "../include/HttpParser.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <thread>
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
    serverAddr.sin_port = htons(port_);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
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
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        
        if (clientSocket == -1) {
            if (running_) {
                LOG_ERROR("Failed to accept connection");
            }
            continue;
        }

        // 处理连接
        connectionManager_->addConnection(clientSocket, [this, clientSocket]() {
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

    // 读取请求
    std::vector<char> buffer(4096);
    ssize_t bytesRead = ssl ? SSL_read(ssl, buffer.data(), buffer.size() - 1)
                           : recv(clientSocket, buffer.data(), buffer.size() - 1, 0);
    
    if (bytesRead <= 0) {
        if (ssl) SSL_free(ssl);
        close(clientSocket);
        return;
    }
    
    buffer[bytesRead] = '\0';
    std::string request(buffer.data());
    
    // 使用HttpParser解析请求
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;
    std::tie(path, headers, body) = HttpParser::parseRequest(request);
    LOG_INFO("Received request for path: " + path);
    
    // 处理请求
    bool found;
    std::string content;
    std::tie(found, content) = router_->handleRequest(path, headers, body);
    
    // 使用HttpParser构建响应
    std::string response;
    if (found) {
        // 检查是否需要分块传输
        bool useChunked = headers.count("Transfer-Encoding") > 0 && 
                         headers["Transfer-Encoding"] == "chunked";
        
        response = useChunked ? 
            HttpParser::buildChunkedResponse(HttpStatus::OK, content) :
            HttpParser::buildResponse(HttpStatus::OK, content);
    } else {
        response = HttpParser::buildResponse(HttpStatus::NOT_FOUND, 
            "<html><body><h1>404 Not Found</h1></body></html>");
    }
    
    // 发送响应
    if (ssl) {
        SSL_write(ssl, response.c_str(), response.size());
        SSL_shutdown(ssl);
        SSL_free(ssl);
    } else {
        send(clientSocket, response.c_str(), response.size(), 0);
    }
    close(clientSocket);
}

} // namespace webserver