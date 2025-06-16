#include "HttpStatus.hpp"

HttpStatusHandler& HttpStatusHandler::getInstance() {
    static HttpStatusHandler instance;
    return instance;
}

HttpStatusHandler::HttpStatusHandler() {
    // 初始化状态码到状态消息的映射
    statusMessages_ = {
        // 1xx: 信息性状态码
        {100, "Continue"},
        {101, "Switching Protocols"},

        // 2xx: 成功状态码
        {200, "OK"},
        {201, "Created"},
        {202, "Accepted"},
        {204, "No Content"},
        {206, "Partial Content"},

        // 3xx: 重定向状态码
        {300, "Multiple Choices"},
        {301, "Moved Permanently"},
        {302, "Found"},
        {303, "See Other"},
        {304, "Not Modified"},
        {307, "Temporary Redirect"},
        {308, "Permanent Redirect"},

        // 4xx: 客户端错误状态码
        {400, "Bad Request"},
        {401, "Unauthorized"},
        {403, "Forbidden"},
        {404, "Not Found"},
        {405, "Method Not Allowed"},
        {406, "Not Acceptable"},
        {408, "Request Timeout"},
        {409, "Conflict"},
        {410, "Gone"},
        {411, "Length Required"},
        {413, "Payload Too Large"},
        {414, "URI Too Long"},
        {415, "Unsupported Media Type"},
        {416, "Range Not Satisfiable"},
        {417, "Expectation Failed"},
        {426, "Upgrade Required"},
        {429, "Too Many Requests"},

        // 5xx: 服务器错误状态码
        {500, "Internal Server Error"},
        {501, "Not Implemented"},
        {502, "Bad Gateway"},
        {503, "Service Unavailable"},
        {504, "Gateway Timeout"},
        {505, "HTTP Version Not Supported"}
    };
}

std::string HttpStatusHandler::getStatusMessage(HttpStatus status) const {
    return getStatusMessage(static_cast<int>(status));
}

std::string HttpStatusHandler::getStatusMessage(int statusCode) const {
    auto it = statusMessages_.find(statusCode);
    if (it != statusMessages_.end()) {
        return it->second;
    }
    return "Unknown Status";
}

bool HttpStatusHandler::isInformational(HttpStatus status) {
    int code = static_cast<int>(status);
    return code >= 100 && code < 200;
}

bool HttpStatusHandler::isSuccessful(HttpStatus status) {
    int code = static_cast<int>(status);
    return code >= 200 && code < 300;
}

bool HttpStatusHandler::isRedirection(HttpStatus status) {
    int code = static_cast<int>(status);
    return code >= 300 && code < 400;
}

bool HttpStatusHandler::isClientError(HttpStatus status) {
    int code = static_cast<int>(status);
    return code >= 400 && code < 500;
}

bool HttpStatusHandler::isServerError(HttpStatus status) {
    int code = static_cast<int>(status);
    return code >= 500 && code < 600;
}