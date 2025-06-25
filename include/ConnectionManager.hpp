#ifndef WEBSERVER_CONNECTION_MANAGER_HPP
#define WEBSERVER_CONNECTION_MANAGER_HPP

#include <map>
#include <mutex>
#include <functional>
#include <thread>
#include "Config.hpp"

namespace webserver {

/**
 * @class ConnectionManager
 * @brief 管理服务器的客户端连接
 */
class ConnectionManager {
public:
    using ConnectionHandler = std::function<void()>;

    /**
     * @brief 构造函数
     * @param config 服务器配置
     */
    explicit ConnectionManager(const Config& config);
    
    /**
     * @brief 析构函数
     */
    ~ConnectionManager();

    /**
     * @brief 添加新的连接
     * @param socket 客户端套接字描述符
     * @param handler 处理连接的函数
     */
    void addConnection(int socket, ConnectionHandler handler);

    /**
     * @brief 关闭指定连接
     * @param socket 要关闭的套接字描述符
     */
    void closeConnection(int socket);

    /**
     * @brief 停止所有连接
     */
    void stopAll();

private:
    std::map<int, std::thread> connections_;  // 连接映射表
    std::mutex connectionsMutex_;             // 保护连接映射表的互斥锁
    const Config& config_;                    // 服务器配置
    bool running_;                            // 连接管理器运行状态
};

} // namespace webserver

#endif // WEBSERVER_CONNECTION_MANAGER_HPP