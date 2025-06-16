#ifndef CONNECTION_MANAGER_HPP
#define CONNECTION_MANAGER_HPP

#include <map>
#include <string>
#include <mutex>
#include <chrono>
#include <unordered_map>
#include "Config.hpp"

/**
 * @brief 连接管理器类，负责管理HTTP连接的生命周期
 * 
 * 该类处理HTTP连接的状态跟踪、超时管理和Keep-Alive功能
 */
class ConnectionManager {
public:
    /**
     * @brief 构造函数
     * @param config 配置对象，包含连接管理相关的配置参数
     */
    ConnectionManager(const Config& config);
    
    /**
     * @brief 析构函数，关闭所有活跃连接
     */
    ~ConnectionManager();
    
    /**
     * @brief 设置套接字的超时选项
     * @param clientSocket 客户端套接字描述符
     */
    void setSocketTimeout(int clientSocket);
    
    /**
     * @brief 根据HTTP头和版本判断是否应该保持连接
     * @param headers HTTP请求头
     * @param httpVersion HTTP版本
     * @return 如果应该保持连接则返回true，否则返回false
     */
    bool shouldKeepAlive(const std::map<std::string, std::string>& headers, 
                        const std::string& httpVersion) const;
    
    /**
     * @brief 更新连接状态
     * @param clientSocket 客户端套接字描述符
     * @param keepAlive 是否保持连接
     */
    void updateConnectionState(int clientSocket, bool keepAlive);
    
    /**
     * @brief 检查连接是否超时
     * @param clientSocket 客户端套接字描述符
     * @return 如果连接超时则返回true，否则返回false
     */
    bool isConnectionTimedOut(int clientSocket);
    
    /**
     * @brief 清理所有超时的连接
     */
    void cleanupTimedOutConnections();
    
    /**
     * @brief 关闭并清理指定的连接
     * @param clientSocket 客户端套接字描述符
     */
    void closeConnection(int clientSocket);

private:
    /**
     * @brief 连接状态结构体
     */
    struct ConnectionState {
        std::chrono::steady_clock::time_point lastActivity; ///< 最后活动时间
        bool keepAlive = false;                            ///< 是否保持连接
        int requestCount = 0;                              ///< 请求计数
    };
    
    std::unordered_map<int, ConnectionState> connections_; ///< 套接字到连接状态的映射
    std::mutex connectionsMutex_;                         ///< 保护连接映射的互斥锁
    
    int keepAliveTimeout_;                                ///< Keep-Alive超时时间（秒）
    int maxRequestsPerConnection_;                        ///< 每个连接的最大请求数
    int socketTimeout_;                                   ///< 套接字超时时间（秒）
};

#endif // CONNECTION_MANAGER_HPP