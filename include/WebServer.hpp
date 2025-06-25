#ifndef WEBSERVER_WEBSERVER_HPP
#define WEBSERVER_WEBSERVER_HPP

#include <string>
#include <map>
#include <functional>
#include <memory>
#include "ConnectionManager.hpp"
#include "Router.hpp"
#include "Config.hpp"
#include "HttpStatus.hpp"
#include "HttpParser.hpp"

namespace webserver {

/**
 * @class WebServer
 * @brief Web服务器的主类，负责处理HTTP请求和管理连接
 */
class WebServer {
public:
    /**
     * @brief 构造函数
     * @param config 服务器配置
     */
    WebServer(const Config& config);

    /**
     * @brief 析构函数
     */
    ~WebServer();

    /**
     * @brief 启动服务器
     * @return 启动成功返回true，否则返回false
     */
    bool start();
    
    /**
     * @brief 停止服务器
     */
    void stop();
    
    /**
     * @brief 添加路由处理函数
     * @param path URL路径
     * @param handler 处理该路径的函数
     */
    void addRoute(const std::string& path, Router::RequestHandler handler);

private:
    /**
     * @brief 处理客户端连接
     * @param clientSocket 客户端套接字描述符
     */
    void handleConnection(int clientSocket);

    int port_;                   // 服务器端口
    bool running_;               // 服务器运行状态
    std::unique_ptr<ConnectionManager> connectionManager_;  // 连接管理器
    std::unique_ptr<Router> router_;                       // 路由器
    Config config_;              // 服务器配置
};

} // namespace webserver

#endif // WEBSERVER_WEBSERVER_HPP