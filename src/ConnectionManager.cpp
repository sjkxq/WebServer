#include "ConnectionManager.hpp"
#include "Logger.hpp"
#include <unistd.h>

namespace webserver {

ConnectionManager::ConnectionManager(const Config& config)
    : config_(config), running_(true) {
}

ConnectionManager::~ConnectionManager() {
    stopAll();
}

void ConnectionManager::addConnection(int socket, ConnectionHandler handler) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    if (!running_) {
        close(socket);
        return;
    }
    
    // 创建新线程处理连接
    connections_[socket] = std::thread([this, socket, handler]() {
        handler();
        closeConnection(socket);
    });
    connections_[socket].detach();
}

void ConnectionManager::closeConnection(int socket) {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    auto it = connections_.find(socket);
    if (it != connections_.end()) {
        close(socket);
        connections_.erase(it);
    }
}

void ConnectionManager::stopAll() {
    std::lock_guard<std::mutex> lock(connectionsMutex_);
    running_ = false;
    
    // 关闭所有连接
    for (const auto& pair : connections_) {
        close(pair.first);
    }
    
    // 清空连接列表
    connections_.clear();
}

} // namespace webserver