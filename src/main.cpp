#include "WebServer.hpp"
#include "Config.hpp"
#include <iostream>
#include <csignal>
#include <cstdlib>

// 全局WebServer指针，用于信号处理
webserver::WebServer* server = nullptr;

// 信号处理函数
void signalHandler(int signum) {
    std::cout << "\nReceived signal (" << signum << "). Shutting down..." << std::endl;
    if (server) {
        server->stop();
    }
}

int main(int argc, char* argv[]) {
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        // 加载配置文件
        webserver::Config config;
        if (!config.loadFromFile("config.json")) {
            std::cerr << "Warning: Failed to load config.json, using default settings" << std::endl;
        }

        // 创建服务器实例
        server = new webserver::WebServer(config);

        // 添加一些示例路由
        server->addRoute("/hello", [](const std::map<std::string, std::string>& headers, const std::string& body) {
            return "<html><body><h1>Hello, World!</h1></body></html>";
        });

        server->addRoute("/about", [](const std::map<std::string, std::string>& headers, const std::string& body) {
            return "<html><body>"
                   "<h1>About This Server</h1>"
                   "<p>This is a simple C++ WebServer implementation.</p>"
                   "</body></html>";
        });

        // 启动服务器
        std::cout << "Starting WebServer on port 8080..." << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        
        if (!server->start()) {
            std::cerr << "Failed to start server" << std::endl;
            delete server;
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        delete server;
        return 1;
    }

    // 清理
    delete server;
    return 0;
}