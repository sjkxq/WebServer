#include "http/PipelinedConnectionHandler.hpp"
#include "http/HttpParser.hpp"
#include "http/HttpProcessor.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <stdexcept>

PipelinedConnectionHandler::PipelinedConnectionHandler(int socket) 
    : clientSocket(socket), running(false) {}

PipelinedConnectionHandler::~PipelinedConnectionHandler() {
    stop();
}

void PipelinedConnectionHandler::start() {
    if (running) return;
    
    running = true;
    requestThread = std::thread(&PipelinedConnectionHandler::requestProcessingThread, this);
    responseThread = std::thread(&PipelinedConnectionHandler::responseProcessingThread, this);
}

void PipelinedConnectionHandler::stop() {
    if (!running) return;
    
    running = false;
    
    // 通知所有等待的线程
    requestCV.notify_all();
    responseCV.notify_all();
    
    // 等待线程结束
    if (requestThread.joinable()) requestThread.join();
    if (responseThread.joinable()) responseThread.join();
    
    close(clientSocket);
}

void PipelinedConnectionHandler::requestProcessingThread() {
    HttpParser parser;
    char buffer[8192];
    
    while (running) {
        ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        
        if (bytesRead <= 0) {
            if (bytesRead == 0) {
                // 客户端关闭连接
                running = false;
                break;
            } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                // 读取错误
                running = false;
                break;
            }
            continue;
        }
        
        // 解析请求
        std::vector<HttpRequest> requests = parser.parse(buffer, bytesRead);
        
        // 将请求加入队列
        {
            std::lock_guard<std::mutex> lock(requestMutex);
            for (auto& req : requests) {
                requestQueue.push(std::move(req));
            }
            requestCV.notify_one();
        }
    }
    
    // 通知处理结束
    responseCV.notify_one();
}

void PipelinedConnectionHandler::responseProcessingThread() {
    HttpProcessor processor;
    
    while (running || !requestQueue.empty()) {
        HttpRequest request;
        
        // 获取下一个请求
        {
            std::unique_lock<std::mutex> lock(requestMutex);
            requestCV.wait(lock, [this]() { 
                return !running || !requestQueue.empty(); 
            });
            
            if (!running && requestQueue.empty()) break;
            
            request = std::move(requestQueue.front());
            requestQueue.pop();
        }
        
        // 处理请求
        HttpResponse response = processor.process(request);
        
        // 将响应加入队列
        {
            std::lock_guard<std::mutex> lock(responseMutex);
            responseQueue.push(std::move(response));
            responseCV.notify_one();
        }
    }
    
    // 发送所有剩余响应
    while (!responseQueue.empty()) {
        HttpResponse response;
        {
            std::lock_guard<std::mutex> lock(responseMutex);
            response = std::move(responseQueue.front());
            responseQueue.pop();
        }
        
        std::string responseStr = response.toString();
        send(clientSocket, responseStr.c_str(), responseStr.size(), 0);
    }
}