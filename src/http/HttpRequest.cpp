#include "http/HttpRequest.hpp"
#include "utils/DateTimeUtils.hpp"
#include <algorithm>
#include <vector>

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

bool HttpRequest::checkIfModifiedSince(time_t lastModified) const {
    std::string ifModifiedSince = getHeader("If-Modified-Since");
    if (ifModifiedSince.empty()) return false;
    
    time_t headerTime = DateTimeUtils::parseHttpDate(ifModifiedSince);
    if (headerTime == 0) return false;
    
    return lastModified <= headerTime;
}

bool HttpRequest::checkIfUnmodifiedSince(time_t lastModified) const {
    std::string ifUnmodifiedSince = getHeader("If-Unmodified-Since");
    if (ifUnmodifiedSince.empty()) return false;
    
    time_t headerTime = DateTimeUtils::parseHttpDate(ifUnmodifiedSince);
    if (headerTime == 0) return false;
    
    return lastModified > headerTime;
}

bool HttpRequest::checkIfNoneMatch(const std::string& etag) const {
    std::string ifNoneMatch = getHeader("If-None-Match");
    if (ifNoneMatch.empty()) return false;
    
    // 处理通配符*
    if (ifNoneMatch == "*") return true;
    
    // 处理多个ETag的情况 (逗号分隔)
    std::vector<std::string> etags;
    size_t pos = 0;
    while ((pos = ifNoneMatch.find(',')) != std::string::npos) {
        std::string tag = ifNoneMatch.substr(0, pos);
        etags.push_back(tag);
        ifNoneMatch.erase(0, pos + 1);
    }
    etags.push_back(ifNoneMatch);
    
    // 检查是否有匹配的ETag
    for (const auto& tag : etags) {
        if (tag == etag) return true;
    }
    
    return false;
}

bool HttpRequest::checkIfMatch(const std::string& etag) const {
    std::string ifMatch = getHeader("If-Match");
    if (ifMatch.empty()) return false;
    
    // 处理通配符*
    if (ifMatch == "*") return true;
    
    // 处理多个ETag的情况 (逗号分隔)
    std::vector<std::string> etags;
    size_t pos = 0;
    while ((pos = ifMatch.find(',')) != std::string::npos) {
        std::string tag = ifMatch.substr(0, pos);
        etags.push_back(tag);
        ifMatch.erase(0, pos + 1);
    }
    etags.push_back(ifMatch);
    
    // 检查是否有匹配的ETag
    for (const auto& tag : etags) {
        if (tag == etag) return true;
    }
    
    return false;
}

} // namespace webserver