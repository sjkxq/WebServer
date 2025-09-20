#include "Router.hpp"
#include "Logger.hpp"
#include "http/HealthCheckController.h"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"

namespace webserver {

Router::Router() {
    // 添加默认路由
    addRoute("/", [](const std::map<std::string, std::string>& headers, const std::string& body) {
        return "<html><body><h1>Welcome to C++ WebServer</h1></body></html>";
    });
    
    // 添加健康检查路由
    addRoute("/health", [](const std::map<std::string, std::string>& headers, const std::string& body) {
        HttpRequest request;
        request.setHeaders(headers);
        request.setBody(body);
        
        auto response = HealthCheckController::checkHealth(request);
        return response->getBody();
    });
}

void Router::addRoute(const std::string& path, RequestHandler handler) {
    routes_[path] = handler;
}

std::pair<bool, std::string> Router::handleRequest(const std::string& path,
    const std::map<std::string, std::string>& headers,
    const std::string& body) const {
    
    auto it = routes_.find(path);
    if (it != routes_.end()) {
        LOG_INFO("Found route handler for path: " + path);
        return {true, it->second(headers, body)};
    }
    
    LOG_WARNING("No route handler found for path: " + path);
    return {false, ""};
}

} // namespace webserver