#include "WebServer.hpp"
#include "Logger.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <thread>

WebServer::WebServer(const Config& config) : 
    port_(config.getNestedValue<int>("server.port", 8080)),
    threadPoolSize_(config.getNestedValue<int>("server.thread_pool_size", 4)),
    timeout_(config.getNestedValue<int>("server.timeout", 30)),
    serverSocket_(-1), 
    running_(false),
    threadPool_(std::make_unique<ThreadPool>(threadPoolSize_)),
    connectionManager_(std::make_unique<ConnectionManager>(config)) {
    // 添加默认路由
    addRoute("/", [](const std::map<std::string, std::string>& headers, const std::string& body) {
        return "<html><body><h1>Welcome to C++ WebServer</h1></body></html>";
    });
}

WebServer::~WebServer() {
    stop();
}

bool WebServer::start() {
    // 创建服务器socket
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ < 0) {
        LOG_ERROR("Failed to create socket");
        return false;
    }

    // 设置socket选项
    int opt = 1;
    if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        LOG_ERROR("Failed to set socket options");
        return false;
    }

    // 绑定地址和端口
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(serverSocket_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        LOG_ERROR("Failed to bind to port " + std::to_string(port_));
        return false;
    }

    // 开始监听
    if (listen(serverSocket_, 3) < 0) {
        LOG_ERROR("Failed to listen on socket");
        return false;
    }

    running_ = true;
    LOG_INFO("Server started on port " + std::to_string(port_));

    // 主接受循环
    while (running_) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        
        int clientSocket = accept(serverSocket_, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            if (!running_) break;
            LOG_ERROR("Failed to accept connection");
            continue;
        }

        LOG_INFO("New client connected (socket: " + std::to_string(clientSocket) + ")");

        // 使用线程池处理连接
        threadPool_->enqueue([this, clientSocket]() {
            handleConnection(clientSocket);
        });
    }

    return true;
}

void WebServer::stop() {
    running_ = false;
    if (serverSocket_ >= 0) {
        close(serverSocket_);
        serverSocket_ = -1;
    }
}

// 连接管理相关方法已移至ConnectionManager类

void WebServer::addRoute(const std::string& path, RequestHandler handler) {
    routes_[path] = handler;
}

void WebServer::handleConnection(int clientSocket) {
    LOG_INFO("Handling connection (socket: " + std::to_string(clientSocket) + ")");
    
    bool keepAlive = false;
    std::string httpVersion = "HTTP/1.1"; // 默认假设为HTTP/1.1
    
    do {
        // 设置读取超时
        connectionManager_->setSocketTimeout(clientSocket);
        
        char buffer[4096] = {0};
        ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        
        if (bytesRead <= 0) {
            LOG_INFO("Connection closed or timed out (socket: " + std::to_string(clientSocket) + ")");
            break; // 连接已关闭或出错
        }
        
        LOG_INFO("Received request (socket: " + std::to_string(clientSocket) + ", bytes: " + std::to_string(bytesRead) + ")");
        std::string method, path, body;
        std::map<std::string, std::string> headers;
        
        if (parseRequest(buffer, method, path, headers, body)) {
            // 从请求行中提取HTTP版本
            std::istringstream requestLine(buffer);
            std::string temp;
            requestLine >> temp >> temp >> httpVersion;
            
            // 使用ConnectionManager确定是否保持连接
            keepAlive = connectionManager_->shouldKeepAlive(headers, httpVersion);
            
            // 更新连接状态
            connectionManager_->updateConnectionState(clientSocket, keepAlive);
            
            // 检查连接是否超时或超过最大请求数
            if (connectionManager_->isConnectionTimedOut(clientSocket)) {
                LOG_INFO("Connection timed out or reached max requests (socket: " + std::to_string(clientSocket) + ")");
                keepAlive = false;
            }
            
            LOG_INFO("Connection will be " + std::string(keepAlive ? "kept alive" : "closed") + 
                     " after response (socket: " + std::to_string(clientSocket) + ")");
            
            std::string response;
            
            // 查找路由处理函数
            auto it = routes_.find(path);
            if (it != routes_.end()) {
                LOG_INFO("Found route handler for path: " + path);
                std::string content = it->second(headers, body);
                response = buildResponse(200, "text/html", content, keepAlive);
            } else {
                LOG_WARNING("No route handler found for path: " + path);
                response = buildResponse(404, "text/plain", "404 Not Found", keepAlive);
            }
            
            ssize_t bytesSent = write(clientSocket, response.c_str(), response.length());
            LOG_INFO("Sent response (socket: " + std::to_string(clientSocket) + 
                     ", bytes: " + std::to_string(bytesSent) + ")");
        } else {
            keepAlive = false; // 解析失败，关闭连接
        }
        
        // 定期清理超时连接
        connectionManager_->cleanupTimedOutConnections();
        
        // 如果不保持连接，则退出循环
    } while (keepAlive && running_);
    
    // 关闭并清理连接
    connectionManager_->closeConnection(clientSocket);
    LOG_INFO("Connection closed (socket: " + std::to_string(clientSocket) + ")");
}

bool WebServer::parseRequest(const std::string& request, std::string& method, 
                           std::string& path, std::map<std::string, std::string>& headers,
                           std::string& body) const {
    std::istringstream iss(request);
    std::string line;
    
    // 解析请求行
    if (std::getline(iss, line)) {
        std::istringstream lineStream(line);
        lineStream >> method >> path;
        
        // 确保方法和路径都被正确解析
        if (method.empty() || path.empty()) {
            LOG_WARNING("Failed to parse request line: " + line);
            return false;
        }
    } else {
        LOG_WARNING("Empty request");
        return false;
    }
    
    // 解析头部
    while (std::getline(iss, line) && line != "\r") {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 2); // 跳过": "
            if (!value.empty() && value.back() == '\r') {
                value.pop_back();
            }
            headers[key] = value;
            
            // 记录日志，显示识别到的Connection头部
            if (key == "Connection") {
                LOG_INFO("Detected Connection header: " + value);
            }
        }
    }
    
    // 读取请求体
    std::stringstream bodyStream;
    while (std::getline(iss, line)) {
        bodyStream << line << "\n";
    }
    body = bodyStream.str();
    
    return true;
}

std::string WebServer::buildResponse(int statusCode, const std::string& contentType, 
                                   const std::string& content, bool keepAlive) const {
    std::stringstream response;
    
    // 状态行
    response << "HTTP/1.1 ";
    switch (statusCode) {
        case 200: response << "200 OK"; break;
        case 404: response << "404 Not Found"; break;
        default: response << statusCode << " Status";
    }
    response << "\r\n";
    
    // 响应头
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << content.length() << "\r\n";
    response << "Connection: " << (keepAlive ? "keep-alive" : "close") << "\r\n";
    response << "\r\n";
    
    // 响应体
    response << content;
    
    return response.str();
}