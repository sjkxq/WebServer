#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <string>
#include <map>
#include <functional>
#include <memory>
#include <vector>
#include <memory>
#include "Config.hpp"
#include "ThreadPool.hpp"

class WebServer {
public:
    // 定义HTTP请求处理函数类型
    using RequestHandler = std::function<std::string(const std::map<std::string, std::string>&, const std::string&)>;
    
    // 构造函数，使用配置对象
    WebServer(const Config& config);
    
    // 析构函数
    ~WebServer();
    
    // 启动服务器
    bool start();
    
    // 停止服务器
    void stop();
    
    // 添加路由处理函数
    void addRoute(const std::string& path, RequestHandler handler);
    
private:
    // 服务器端口
    int port_;
    
    // 服务器socket
    int serverSocket_;
    
    // 运行标志
    bool running_;
    
    // 路由表
    std::map<std::string, RequestHandler> routes_;

    // 线程池大小
    int threadPoolSize_;

    // 超时时间(秒)
    int timeout_;

    // 线程池
    std::unique_ptr<ThreadPool> threadPool_;
    
    // 处理客户端连接
    void handleConnection(int clientSocket);
    
    // 解析HTTP请求
    bool parseRequest(const std::string& request, std::string& method, 
                     std::string& path, std::map<std::string, std::string>& headers,
                     std::string& body);
    
    // 构建HTTP响应
    std::string buildResponse(int statusCode, const std::string& contentType, 
                             const std::string& content);
};

#endif // WEBSERVER_HPP