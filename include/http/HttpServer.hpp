#pragma once

#include <thread>
#include <atomic>

class HttpServer {
public:
    /**
     * @brief 构造函数
     * @param port 监听端口
     */
    explicit HttpServer(int port);
    
    /**
     * @brief 析构函数
     */
    ~HttpServer();
    
    /**
     * @brief 启动HTTP服务器
     * @throws std::runtime_error 如果启动失败
     */
    void start();
    
    /**
     * @brief 停止HTTP服务器
     */
    void stop();
    
private:
    /**
     * @brief 服务器主循环
     */
    void run();
    
    /**
     * @brief 处理客户端连接
     * @param clientSocket 客户端socket
     */
    void handleConnection(int clientSocket);
    
    int port;
    int serverSocket;
    std::atomic<bool> running;
    std::thread serverThread;
};