#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>
#include <map>
#include <functional>

class Router {
public:
    // 定义HTTP请求处理函数类型
    using RequestHandler = std::function<std::string(const std::map<std::string, std::string>&, const std::string&)>;
    
    Router();
    ~Router() = default;
    
    // 添加路由处理函数
    void addRoute(const std::string& path, RequestHandler handler);
    
    // 处理请求
    std::pair<bool, std::string> handleRequest(const std::string& path, 
        const std::map<std::string, std::string>& headers, 
        const std::string& body) const;

private:
    // 路由表
    std::map<std::string, RequestHandler> routes_;
};

#endif // ROUTER_HPP