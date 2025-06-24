#include "WebServer.hpp"
#include "Logger.hpp"
#include "HttpStatus.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <thread>
#include <vector>

WebServer::WebServer(const Config& config) 
    : port_(config.get<int>("port", 8080)), 
      running_(false), 
      config_(config) {
    connectionManager_ = std::make_unique<ConnectionManager>(config_);
    router_ = std::make_unique<Router>();
}

WebServer::~WebServer() {
    stop();
}

bool WebServer::start() {
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
}

void WebServer::addRoute(const std::string& path, Router::RequestHandler handler) {
    router_->addRoute(path, handler);
}

void WebServer::handleConnection(int clientSocket) {
    // 读取请求
    std::vector<char> buffer(4096);
    ssize_t bytesRead = recv(clientSocket, buffer.data(), buffer.size() - 1, 0);
    
    if (bytesRead <= 0) {
        close(clientSocket);
        return;
    }
    
    buffer[bytesRead] = '\0';
    std::string request(buffer.data());
    
    // 解析请求
    std::string path;
    std::map<std::string, std::string> headers;
    std::string body;
    std::tie(path, headers, body) = parseRequest(request);
    LOG_INFO("Received request for path: " + path);
    
    // 处理请求
    bool found;
    std::string content;
    std::tie(found, content) = router_->handleRequest(path, headers, body);
    
    // 构建响应
    std::string response;
    if (found) {
        response = buildResponse(HttpStatus::OK, content);
    } else {
        response = buildResponse(HttpStatus::NOT_FOUND, "<html><body><h1>404 Not Found</h1></body></html>");
    }
    
    // 发送响应
    send(clientSocket, response.c_str(), response.size(), 0);
    close(clientSocket);
}

std::tuple<std::string, std::map<std::string, std::string>, std::string> WebServer::parseRequest(const std::string& request) {
    std::istringstream iss(request);
    std::string line;
    std::string method, path, version;
    std::map<std::string, std::string> headers;
    std::string body;
    
    // 解析请求行
    std::getline(iss, line);
    std::istringstream lineStream(line);
    lineStream >> method >> path >> version;
    
    // 解析请求头
    while (std::getline(iss, line) && line != "\r") {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            // 去除前导空格
            while (!value.empty() && (value[0] == ' ' || value[0] == '\t')) {
                value.erase(0, 1);
            }
            // 去除尾部的\r
            if (!value.empty() && value.back() == '\r') {
                value.pop_back();
            }
            headers[key] = value;
        }
    }
    
    // 解析请求体
    std::ostringstream bodyStream;
    while (std::getline(iss, line)) {
        bodyStream << line << "\n";
    }
    body = bodyStream.str();
    
    return std::make_tuple(path, headers, body);
}

std::string WebServer::buildResponse(HttpStatus statusCode, const std::string& content, const std::string& contentType) {
    std::ostringstream response;
    HttpStatusHandler& statusHandler = HttpStatusHandler::getInstance();
    
    response << "HTTP/1.1 " << static_cast<int>(statusCode) << " " << statusHandler.getStatusMessage(statusCode) << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << content.size() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << content;
    
    return response.str();
}