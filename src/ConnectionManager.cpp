#include "ConnectionManager.hpp"
#include "Logger.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

ConnectionManager::ConnectionManager(const Config& config) :
    keepAliveTimeout_(config.getNestedValue<int>("server.keep_alive_timeout", 5)),
    maxRequestsPerConnection_(config.getNestedValue<int>("server.max_requests_per_connection", 100)),
    socketTimeout_(config.getNestedValue<int>("server.socket_timeout", 30)) {
}

ConnectionManager::~ConnectionManager() {
    // 关闭所有连接
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    for (const auto& pair : connections_) {
        close(pair.first);
    }
    connections_.clear();
}

void ConnectionManager::setSocketTimeout(int clientSocket) {
    struct timeval tv;
    tv.tv_sec = socketTimeout_;
    tv.tv_usec = 0;
    
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        LOG_WARNING("Failed to set socket receive timeout");
    }
    if (setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
        LOG_WARNING("Failed to set socket send timeout");
    }
}

bool ConnectionManager::shouldKeepAlive(const std::map<std::string, std::string>& headers, 
                                      const std::string& httpVersion) const {
    // 检查 Connection 头
    auto it = headers.find("Connection");
    if (httpVersion == "HTTP/1.1") {
        // HTTP/1.1 默认是 keep-alive，除非明确指定 close
        return (it == headers.end() || 
                (it->second != "close" && it->second != "Close"));
    } else {
        // HTTP/1.0 默认是 close，除非明确指定 keep-alive
        return (it != headers.end() && 
                (it->second == "keep-alive" || it->second == "Keep-Alive"));
    }
}

void ConnectionManager::updateConnectionState(int clientSocket, bool keepAlive) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    auto& state = connections_[clientSocket];
    state.lastActivity = std::chrono::steady_clock::now();
    state.keepAlive = keepAlive;
    state.requestCount++;
}

bool ConnectionManager::isConnectionTimedOut(int clientSocket) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    auto it = connections_.find(clientSocket);
    if (it == connections_.end()) {
        return true;  // 如果找不到连接状态，认为已超时
    }

    auto& state = it->second;
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(
        now - state.lastActivity).count();
    
    // 检查是否超时或超过最大请求数
    return duration > keepAliveTimeout_ || state.requestCount >= maxRequestsPerConnection_;
}

void ConnectionManager::cleanupTimedOutConnections() {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    auto now = std::chrono::steady_clock::now();
    
    for (auto it = connections_.begin(); it != connections_.end();) {
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            now - it->second.lastActivity).count();
        
        if (duration > keepAliveTimeout_ || it->second.requestCount >= maxRequestsPerConnection_) {
            LOG_INFO("Closing timed out connection (socket: " + std::to_string(it->first) + 
                    ", duration: " + std::to_string(duration) + "s, requests: " + 
                    std::to_string(it->second.requestCount) + ")");
            close(it->first);  // 关闭超时的连接
            it = connections_.erase(it);  // 从映射中移除并获取下一个迭代器
        } else {
            ++it;
        }
    }
}

void ConnectionManager::closeConnection(int clientSocket) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    auto it = connections_.find(clientSocket);
    if (it != connections_.end()) {
        close(clientSocket);
        connections_.erase(it);
    }
}