#ifndef CONNECTION_MANAGER_HPP
#define CONNECTION_MANAGER_HPP

#include <map>
#include <mutex>
#include <functional>
#include <thread>
#include "Config.hpp"

class ConnectionManager {
public:
    using ConnectionHandler = std::function<void()>;

    explicit ConnectionManager(const Config& config);
    ~ConnectionManager();

    // 添加新的连接
    void addConnection(int socket, ConnectionHandler handler);

    // 关闭指定连接
    void closeConnection(int socket);

    // 停止所有连接
    void stopAll();

private:
    std::map<int, std::thread> connections_;
    std::mutex connectionsMutex_;
    const Config& config_;
    bool running_;
};

#endif // CONNECTION_MANAGER_HPP