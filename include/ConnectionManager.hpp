#ifndef WEBSERVER_CONNECTION_MANAGER_HPP
#define WEBSERVER_CONNECTION_MANAGER_HPP

#include <map>
#include <mutex>
#include <functional>
#include <thread>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include "Config.hpp"

namespace webserver {

/**
 * @struct ConnectionInfo
 * @brief 存储连接的详细信息
 */
struct ConnectionInfo {
    std::thread thread;                           // 处理连接的线程
    std::chrono::steady_clock::time_point lastActivity; // 最后活动时间
    int requestCount;                             // 请求计数
    bool keepAlive;                               // 是否保持连接
    std::string clientIP;                         // 客户端IP地址
};

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
    void addConnection(int socket, const std::string& clientIP, ConnectionHandler handler);

    /**
     * @brief 关闭指定连接
     * @param socket 要关闭的套接字描述符
     */
    void closeConnection(int socket);

    /**
     * @brief 停止所有连接
     */
    void stopAll();
    
    /**
     * @brief 更新连接的活动时间
     * @param socket 套接字描述符
     */
    void updateActivity(int socket);
    
    /**
     * @brief 设置连接的保活状态
     * @param socket 套接字描述符
     * @param keepAlive 是否保持连接
     */
    void setKeepAlive(int socket, bool keepAlive);
    
    /**
     * @brief 获取当前连接数量
     * @return 连接数量
     */
    size_t getConnectionCount() const;
    
    /**
     * @brief 获取活跃连接数量
     * @return 活跃连接数量
     */
    size_t getActiveConnectionCount() const;
    
    /**
     * @brief 获取总请求数量
     * @return 总请求数量
     */
    uint64_t getTotalRequestCount() const;
    
    /**
     * @brief 获取连接统计信息
     * @return 包含统计信息的JSON字符串
     */
    std::string getConnectionStats() const;

private:
    std::atomic<uint64_t> totalRequests_{0};      // 总请求计数
    /**
     * @brief 清理过期连接的线程函数
     */
    void cleanupTask();
    
    std::map<int, ConnectionInfo> connections_;   // 连接映射表
    std::map<std::string, int> ipConnections_;    // IP地址连接数映射表
    mutable std::mutex connectionsMutex_;         // 保护连接映射表的互斥锁
    const Config& config_;                        // 服务器配置
    std::atomic<bool> running_;                   // 连接管理器运行状态
    
    // 配置项
    int maxConnectionsPerClient_;                 // 每个客户端的最大连接数
    int maxConnectionsPerIP_;                     // 每个IP地址的最大连接数
    int connectionTimeout_;                       // 连接超时时间（秒）
    int keepAliveTimeout_;                        // 保活连接超时时间（秒）
    int maxRequestsPerConnection_;                // 每个连接的最大请求数
    int connectionCleanupInterval_;               // 连接清理间隔（秒）
    
    // 清理线程
    std::thread cleanupThread_;                   // 清理过期连接的线程
    std::condition_variable cleanupCV_;           // 用于通知清理线程的条件变量
};

} // namespace webserver

#endif // WEBSERVER_CONNECTION_MANAGER_HPP