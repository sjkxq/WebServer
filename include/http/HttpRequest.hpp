#pragma once

#include <string>
#include <map>

namespace webserver {

/**
 * @class HttpRequest
 * @brief 封装HTTP请求数据
 */
class HttpRequest {
public:
    /**
     * @brief 构造函数
     * @param method HTTP方法（GET/POST等）
     * @param path 请求路径
     * @param headers 请求头
     * @param body 请求体
     * @param queryParams 查询参数
     */
    HttpRequest(const std::string& method, 
                const std::string& path,
                const std::map<std::string, std::string>& headers,
                const std::string& body,
                const std::map<std::string, std::string>& queryParams = {});

    // 获取方法
    const std::string& getMethod() const;
    const std::string& getPath() const;
    const std::map<std::string, std::string>& getHeaders() const;
    const std::string& getBody() const;
    const std::map<std::string, std::string>& getQueryParams() const;

    /**
     * @brief 获取指定头部的值
     * @param name 头部名称
     * @return 头部值，如果不存在则返回空字符串
     */
    std::string getHeader(const std::string& name) const;

    /**
     * @brief 获取指定查询参数的值
     * @param name 参数名称
     * @return 参数值，如果不存在则返回空字符串
     */
    std::string getQueryParam(const std::string& name) const;

private:
    std::string method_;
    std::string path_;
    std::map<std::string, std::string> headers_;
    std::string body_;
    std::map<std::string, std::string> queryParams_;
};

} // namespace webserver