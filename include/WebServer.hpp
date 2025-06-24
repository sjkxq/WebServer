#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <string>
#include <map>
#include <functional>
#include <memory>
#include "ConnectionManager.hpp"
#include "Router.hpp"
#include "Config.hpp"
#include "HttpStatus.hpp"

class WebServer {
public:
    WebServer(const Config& config);
    ~WebServer();

    // 启动服务器
    bool start();
    
    // 停止服务器
    void stop();
    
    // 添加路由处理函数
    void addRoute(const std::string& path, Router::RequestHandler handler);

private:
    // 处理客户端连接
    void handleConnection(int clientSocket);
    
    // 解析HTTP请求
    std::tuple<std::string, std::map<std::string, std::string>, std::string> parseRequest(const std::string& request);
    
    // 构建HTTP响应
    std::string buildResponse(HttpStatus statusCode, const std::string& content, const std::string& contentType = "text/html");

    int port_;
    bool running_;
    std::unique_ptr<ConnectionManager> connectionManager_;
    std::unique_ptr<Router> router_;
    Config config_;
};

#endif // WEBSERVER_HPP