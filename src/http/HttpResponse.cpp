#include "HttpResponse.hpp"
#include "../HttpStatus.hpp"
#include <sstream>
#include <iomanip>

namespace webserver {

HttpResponse::HttpResponse(HttpStatus statusCode,
                           const std::string& content,
                           const std::string& contentType)
    : statusCode_(statusCode), content_(content) {
    setHeader("Content-Type", contentType);
    setHeader("Content-Length", std::to_string(content.size()));
}

void HttpResponse::setHeader(const std::string& name, const std::string& value) {
    headers_[name] = value;
}

std::string HttpResponse::getHeader(const std::string& name) const {
    auto it = headers_.find(name);
    return it != headers_.end() ? it->second : "";
}

std::string HttpResponse::build() const {
    std::ostringstream response;
    response << "HTTP/1.1 " << static_cast<int>(statusCode_) 
                           << " " << HttpStatusHandler::getInstance().getStatusMessage(statusCode_) << "\r\n";

    for (const auto& header : headers_) {
        response << header.first << ": " << header.second << "\r\n";
    }
    response << "\r\n" << content_;

    return response.str();
}

std::string HttpResponse::buildChunked() const {
    std::ostringstream response;
    response << "HTTP/1.1 " << static_cast<int>(statusCode_) 
                           << " " << HttpStatusHandler::getInstance().getStatusMessage(statusCode_) << "\r\n";

    for (const auto& header : headers_) {
        if (header.first != "Content-Length") {  // 分块传输不需要Content-Length
            response << header.first << ": " << header.second << "\r\n";
        }
    }
    response << "Transfer-Encoding: chunked\r\n\r\n";

    // 添加分块数据
    std::stringstream chunk;
    chunk << std::hex << content_.size() << "\r\n" << content_ << "\r\n";
    response << chunk.str() << "0\r\n\r\n";  // 结束块

    return response.str();
}

HttpStatus HttpResponse::getStatusCode() const {
    return statusCode_;
}

const std::string& HttpResponse::getContent() const {
    return content_;
}

const std::map<std::string, std::string>& HttpResponse::getHeaders() const {
    return headers_;
}

std::string HttpResponse::getBody() const {
    return content_;
}

HttpResponse HttpResponse::create(int statusCode, 
                                const std::string& content,
                                const std::map<std::string, std::string>& headers) {
    HttpResponse response(static_cast<HttpStatus>(statusCode), content);
    for (const auto& header : headers) {
        response.setHeader(header.first, header.second);
    }
    return response;
}

} // namespace webserver