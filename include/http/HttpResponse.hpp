#pragma once

#include <string>
#include <map>
#include "HttpStatus.hpp"

namespace webserver {

/**
 * @class HttpResponse
 * @brief 封装HTTP响应数据
 */
class HttpResponse {
public:
    /**
     * @brief 构造函数
     * @param statusCode HTTP状态码
     * @param content 响应内容
     * @param contentType 内容类型，默认为"text/html"
     */
    HttpResponse(HttpStatus statusCode, 
                 const std::string& content,
                 const std::string& contentType = "text/html");

    /**
     * @brief 设置响应头
     * @param name 头部名称
     * @param value 头部值
     */
    void setHeader(const std::string& name, const std::string& value);

    /**
     * @brief 获取响应头
     * @param name 头部名称
     * @return 头部值，如果不存在则返回空字符串
     */
    std::string getHeader(const std::string& name) const;

    /**
     * @brief 构建HTTP响应字符串
     * @return 完整的HTTP响应字符串
     */
    std::string build() const;

    /**
     * @brief 构建分块传输编码的HTTP响应字符串
     * @return 使用分块传输编码的HTTP响应字符串
     */
    std::string buildChunked() const;

    // 获取方法
    HttpStatus getStatusCode() const;
    const std::string& getContent() const;
    const std::map<std::string, std::string>& getHeaders() const;

private:
    HttpStatus statusCode_;
    std::string content_;
    std::map<std::string, std::string> headers_;
};

} // namespace webserver