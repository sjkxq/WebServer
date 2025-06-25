#include "HttpParser.hpp"
#include <sstream>
#include "Logger.hpp"

std::tuple<std::string, std::map<std::string, std::string>, std::string> HttpParser::parseRequest(const std::string& request) {
    std::istringstream iss(request);
    std::string line;
    std::string method, path, version;
    std::map<std::string, std::string> headers;
    std::string body;
    
    // 解析请求行
    std::getline(iss, line);
    std::istringstream lineStream(line);
    lineStream >> method >> path >> version;
    
    // 解析请求头
    while (std::getline(iss, line) && line != "\r") {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            // 去除前导空格
            while (!value.empty() && (value[0] == ' ' || value[0] == '\t')) {
                value.erase(0, 1);
            }
            // 去除尾部的\r
            if (!value.empty() && value.back() == '\r') {
                value.pop_back();
            }
            headers[key] = value;
        }
    }
    
    // 解析请求体
    std::ostringstream bodyStream;
    while (std::getline(iss, line)) {
        bodyStream << line << "\n";
    }
    body = bodyStream.str();
    
    return std::make_tuple(path, headers, body);
}

std::string HttpParser::buildResponse(HttpStatus statusCode, const std::string& content, const std::string& contentType) {
    std::ostringstream response;
    HttpStatusHandler& statusHandler = HttpStatusHandler::getInstance();
    
    // 获取状态消息
    std::string statusMessage = statusHandler.getStatusMessage(statusCode);
    
    // 根据状态码类型记录不同级别的日志
    if (HttpStatusHandler::isSuccessful(statusCode)) {
        LOG_INFO("Sending successful response: " + std::to_string(static_cast<int>(statusCode)) + " " + statusMessage);
    } else if (HttpStatusHandler::isClientError(statusCode)) {
        LOG_WARNING("Sending client error response: " + std::to_string(static_cast<int>(statusCode)) + " " + statusMessage);
    } else if (HttpStatusHandler::isServerError(statusCode)) {
        LOG_ERROR("Sending server error response: " + std::to_string(static_cast<int>(statusCode)) + " " + statusMessage);
    }
    
    // 构建响应
    response << "HTTP/1.1 " << static_cast<int>(statusCode) << " " << statusMessage << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << content.size() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << content;
    
    return response.str();
}