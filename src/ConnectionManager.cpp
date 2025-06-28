#include "ConnectionManager.hpp"
#include "Logger.hpp"
#include <unistd.h>
#include <chrono>

namespace webserver {

ConnectionManager::ConnectionManager(const Config& config)
    : config_(config), running_(true) {
    // 从配置中读取连接管理相关的配置项
    maxConnectionsPerClient_ = config.get<int>("server.max_connections_per_client", 1000);
    maxConnectionsPerIP_ = config.get<int>("server.max_connections_per_ip", 100);
    connectionTimeout_ = config.get<int>("server.timeout", 60);
    keepAliveTimeout_ = config.get<int>("server.keep_alive_timeout", 5);
    maxRequestsPerConnection_ = config.get<int>("server.max_requests_per_connection", 100);
    connectionCleanupInterval_ = config.get<int>("server.connection_cleanup_interval", 1);

    // 启动清理线程
    cleanupThread_ = std::thread(&ConnectionManager::cleanupTask, this);
}

ConnectionManager::~ConnectionManager() {
    stopAll();
    
    // 等待清理线程结束
    if (cleanupThread_.joinable()) {
        cleanupCV_.notify_one();
        cleanupThread_.join();
    }
}

void ConnectionManager::addConnection(int socket, const std::string& clientIP, ConnectionHandler handler) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    if (!running_) {
        close(socket);
        return;
    }

    // 检查连接数量限制
    if (connections_.size() >= static_cast<size_t>(maxConnectionsPerClient_)) {
        LOG_ERROR("Maximum connection limit reached");
        close(socket);
        return;
    }
    
    // 检查IP地址的连接数是否达到限制
    auto& ipCount = ipConnections_[clientIP];
    if (ipCount >= maxConnectionsPerIP_) {
        LOG_ERROR("Maximum connection per IP limit reached for IP: " + clientIP);
        close(socket);
        return;
    }
    
    // 更新IP地址的连接数
    ipCount++;
    
    // 创建新的连接信息
    ConnectionInfo connInfo;
    connInfo.lastActivity = std::chrono::steady_clock::now();
    connInfo.requestCount = 0;
    connInfo.keepAlive = false;
    connInfo.clientIP = clientIP;
    
    // 创建新线程处理连接
    connInfo.thread = std::thread([this, socket, handler]() {
        handler();
        closeConnection(socket);
    });
    connInfo.thread.detach();
    
    // 保存连接信息
    connections_[socket] = std::move(connInfo);
}

void ConnectionManager::closeConnection(int socket) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    auto it = connections_.find(socket);
    if (it != connections_.end()) {
        // 获取连接对应的IP地址
        const std::string& clientIP = it->second.clientIP;
        auto ipIt = ipConnections_.find(clientIP);
        
        if (ipIt != ipConnections_.end()) {
            // 减少IP地址的连接计数
            ipIt->second--;
            
            // 如果连接计数为0，从映射表中移除该IP地址
            if (ipIt->second == 0) {
                ipConnections_.erase(ipIt);
            }
        }
        
        close(socket);
        connections_.erase(it);
    }
}

void ConnectionManager::stopAll() {
    {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        running_ = false;
        
        // 关闭所有连接
        for (const auto& pair : connections_) {
            close(pair.first);
        }
        
        // 清空连接列表
        connections_.clear();
    }
    
    // 通知清理线程退出
    cleanupCV_.notify_one();
}

void ConnectionManager::updateActivity(int socket) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    auto it = connections_.find(socket);
    if (it != connections_.end()) {
        it->second.lastActivity = std::chrono::steady_clock::now();
        it->second.requestCount++;
        totalRequests_++;
        
        // 检查请求数量限制
        if (it->second.requestCount >= maxRequestsPerConnection_) {
            it->second.keepAlive = false;
        }
    }
}

size_t ConnectionManager::getActiveConnectionCount() const {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    size_t activeCount = 0;
    auto now = std::chrono::steady_clock::now();
    
    for (const auto& pair : connections_) {
        const auto& connInfo = pair.second;
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            now - connInfo.lastActivity).count();
            
        if (duration < 5) { // 5秒内活跃的连接
            activeCount++;
        }
    }
    return activeCount;
}

uint64_t ConnectionManager::getTotalRequestCount() const {
    return totalRequests_.load();
}

std::string ConnectionManager::getConnectionStats() const {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    
    std::stringstream ss;
    ss << "{";
    ss << "\"total_connections\": " << connections_.size() << ",";
    ss << "\"active_connections\": " << getActiveConnectionCount() << ",";
    ss << "\"total_requests\": " << totalRequests_.load() << ",";
    ss << "\"unique_ips\": " << ipConnections_.size() << ",";
    ss << "\"max_connections_per_ip\": " << maxConnectionsPerIP_ << ",";
    ss << "\"max_connections_per_client\": " << maxConnectionsPerClient_;
    ss << "}";
    
    return ss.str();
}

void ConnectionManager::setKeepAlive(int socket, bool keepAlive) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    auto it = connections_.find(socket);
    if (it != connections_.end()) {
        it->second.keepAlive = keepAlive;
    }
}

size_t ConnectionManager::getConnectionCount() const {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    return connections_.size();
}

void ConnectionManager::cleanupTask() {
    int cleanupCount = 0;
    while (running_) {
        std::unique_lock<std::mutex> lock(connectionsMutex_);
        
        // 等待清理间隔或者被通知退出
        cleanupCV_.wait_for(lock, 
            std::chrono::seconds(connectionCleanupInterval_),
            [this] { return !running_; });
            
        if (!running_) {
            break;
        }
        
        auto now = std::chrono::steady_clock::now();
        
        // 遍历所有连接，关闭超时的连接
        for (auto it = connections_.begin(); it != connections_.end();) {
            const auto& connInfo = it->second;
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                now - connInfo.lastActivity).count();
                
            bool shouldClose = false;
            
            // 检查是否超时
            if (connInfo.keepAlive) {
                shouldClose = duration > keepAliveTimeout_;
            } else {
                shouldClose = duration > connectionTimeout_;
            }
            
            if (shouldClose) {
                LOG_DEBUG("Closing inactive connection: " + std::to_string(it->first));
                close(it->first);
                it = connections_.erase(it);
            } else {
                ++it;
            }
        }

        // 每10次清理（约10分钟）输出一次统计信息
        if (++cleanupCount % 10 == 0) {
            LOG_INFO("Connection statistics: " + getConnectionStats());
        }
    }
}

} // namespace webserver