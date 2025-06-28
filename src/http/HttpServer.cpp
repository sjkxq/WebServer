#include "http/HttpServer.hpp"
#include "http/PipelinedConnectionHandler.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <stdexcept>

HttpServer::HttpServer(int port) : port(port), running(false) {}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::start() {
    if (running) return;
    
    // 创建socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        throw std::runtime_error("Failed to create socket");
    }
    
    // 设置socket选项
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // 绑定端口
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        throw std::runtime_error("Failed to bind port");
    }
    
    // 开始监听
    if (listen(serverSocket, 10) < 0) {
        throw std::runtime_error("Failed to listen on socket");
    }
    
    running = true;
    serverThread = std::thread(&HttpServer::run, this);
}

void HttpServer::stop() {
    if (!running) return;
    
    running = false;
    close(serverSocket);
    
    if (serverThread.joinable()) {
        serverThread.join();
    }
}

void HttpServer::run() {
    while (running) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                // 严重错误，停止服务器
                running = false;
                break;
            }
            continue;
        }
        
        // 处理连接
        std::thread(&HttpServer::handleConnection, this, clientSocket).detach();
    }
}

void HttpServer::handleConnection(int clientSocket) {
    // 设置socket为非阻塞
    int flags = fcntl(clientSocket, F_GETFL, 0);
    fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);
    
    // 检查客户端是否支持管道化 (HTTP/1.1默认支持)
    bool pipeliningSupported = true;
    
    if (pipeliningSupported) {
        // 使用管道化处理器
        PipelinedConnectionHandler handler(clientSocket);
        handler.start();
        
        // 等待处理完成或超时
        std::this_thread::sleep_for(std::chrono::seconds(30));
        handler.stop();
    } else {
        // 传统单请求处理
        char buffer[8192];
        HttpParser parser;
        HttpProcessor processor;
        
        while (true) {
            ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            
            if (bytesRead <= 0) {
                if (bytesRead == 0) {
                    // 客户端关闭连接
                    break;
                } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    // 读取错误
                    break;
                }
                continue;
            }
            
            // 解析并处理请求
            std::vector<HttpRequest> requests = parser.parse(buffer, bytesRead);
            for (auto& request : requests) {
                HttpResponse response = processor.process(request);
                std::string responseStr = response.toString();
                send(clientSocket, responseStr.c_str(), responseStr.size(), 0);
            }
        }
        
        close(clientSocket);
    }
}