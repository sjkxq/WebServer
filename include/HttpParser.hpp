#ifndef HTTP_PARSER_HPP
#define HTTP_PARSER_HPP

#include <string>
#include <map>
#include <tuple>
#include "HttpStatus.hpp"

class HttpParser {
public:
    // 解析HTTP请求，返回路径、头部和正文
    static std::tuple<std::string, std::map<std::string, std::string>, std::string> parseRequest(const std::string& request);
    
    // 构建HTTP响应
    static std::string buildResponse(HttpStatus statusCode, const std::string& content, const std::string& contentType = "text/html");
};

#endif // HTTP_PARSER_HPP