#ifndef WEBSERVER_HTTP_PARSER_HPP
#define WEBSERVER_HTTP_PARSER_HPP

#include <string>
#include <map>
#include <tuple>
#include "HttpStatus.hpp"

namespace webserver {

/**
 * @class HttpParser
 * @brief 处理HTTP请求和响应的解析与构建
 */
class HttpParser {
public:
    /**
     * @brief 解析HTTP请求
     * @param request HTTP请求字符串
     * @return 包含路径、头部和正文的元组
     */
    static std::tuple<std::string, std::map<std::string, std::string>, std::string> parseRequest(const std::string& request);
    
    /**
     * @brief 构建HTTP响应
     * @param statusCode HTTP状态码
     * @param content 响应内容
     * @param contentType 内容类型，默认为"text/html"
     * @return 完整的HTTP响应字符串
     */
    static std::string buildResponse(HttpStatus statusCode, const std::string& content, const std::string& contentType = "text/html");

    /**
     * @brief 构建HTTP响应（带自定义头部）
     * @param statusCode HTTP状态码
     * @param content 响应内容
     * @param headers 自定义HTTP头部
     * @param contentType 内容类型，默认为"text/html"
     * @return 完整的HTTP响应字符串
     */
    static std::string buildResponse(HttpStatus statusCode, const std::string& content, 
                                    const std::map<std::string, std::string>& headers, 
                                    const std::string& contentType = "text/html");

    /**
     * @brief 构建分块传输编码的HTTP响应
     * @param statusCode HTTP状态码
     * @param content 响应内容
     * @param contentType 内容类型，默认为"text/html"
     * @return 使用分块传输编码的HTTP响应字符串
     */
    static std::string buildChunkedResponse(HttpStatus statusCode, const std::string& content, const std::string& contentType = "text/html");

    /**
     * @brief 构建分块传输编码的HTTP响应（带自定义头部）
     * @param statusCode HTTP状态码
     * @param content 响应内容
     * @param headers 自定义HTTP头部
     * @param contentType 内容类型，默认为"text/html"
     * @return 使用分块传输编码的HTTP响应字符串
     */
    static std::string buildChunkedResponse(HttpStatus statusCode, const std::string& content, 
                                          const std::map<std::string, std::string>& headers, 
                                          const std::string& contentType = "text/html");
};

} // namespace webserver

#endif // WEBSERVER_HTTP_PARSER_HPP