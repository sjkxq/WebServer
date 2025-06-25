#ifndef WEBSERVER_ROUTER_HPP
#define WEBSERVER_ROUTER_HPP

#include <string>
#include <map>
#include <functional>

namespace webserver {

/**
 * @class Router
 * @brief 处理HTTP请求路由的类
 */
class Router {
public:
    // 定义HTTP请求处理函数类型
    using RequestHandler = std::function<std::string(const std::map<std::string, std::string>&, const std::string&)>;
    
    /**
     * @brief 构造函数，初始化默认路由
     */
    Router();
    
    /**
     * @brief 析构函数
     */
    ~Router() = default;
    
    /**
     * @brief 添加路由处理函数
     * @param path URL路径
     * @param handler 处理该路径的函数
     */
    void addRoute(const std::string& path, RequestHandler handler);
    
    /**
     * @brief 处理HTTP请求
     * @param path 请求的URL路径
     * @param headers HTTP请求头
     * @param body HTTP请求体
     * @return 包含处理结果的pair，first表示是否找到路由，second为响应内容
     */
    std::pair<bool, std::string> handleRequest(const std::string& path, 
        const std::map<std::string, std::string>& headers, 
        const std::string& body) const;

private:
    // 路由表
    std::map<std::string, RequestHandler> routes_;
};

} // namespace webserver

#endif // WEBSERVER_ROUTER_HPP