#include "http/HttpRequest.hpp"
#include <algorithm>

namespace webserver {

HttpRequest::HttpRequest(const std::string& method, 
                         const std::string& path,
                         const std::map<std::string, std::string>& headers,
                         const std::string& body,
                         const std::map<std::string, std::string>& queryParams)
    : method_(method), path_(path), headers_(headers), 
      body_(body), queryParams_(queryParams) {
    // 确保方法名大写
    std::transform(method_.begin(), method_.end(), method_.begin(), ::toupper);
}

const std::string& HttpRequest::getMethod() const {
    return method_;
}

const std::string& HttpRequest::getPath() const {
    return path_;
}

const std::map<std::string, std::string>& HttpRequest::getHeaders() const {
    return headers_;
}

const std::string& HttpRequest::getBody() const {
    return body_;
}

const std::map<std::string, std::string>& HttpRequest::getQueryParams() const {
    return queryParams_;
}

std::string HttpRequest::getHeader(const std::string& name) const {
    auto it = headers_.find(name);
    return it != headers_.end() ? it->second : "";
}

std::string HttpRequest::getQueryParam(const std::string& name) const {
    auto it = queryParams_.find(name);
    return it != queryParams_.end() ? it->second : "";
}

} // namespace webserver