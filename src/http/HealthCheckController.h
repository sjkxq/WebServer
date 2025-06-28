#ifndef WEBSERVER_HEALTHCHECK_CONTROLLER_H
#define WEBSERVER_HEALTHCHECK_CONTROLLER_H

#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include <memory>

namespace webserver {

class HealthCheckController {
public:
    static std::shared_ptr<HttpResponse> checkHealth(const HttpRequest& request);
    
    // 系统资源监控
    static double getCpuUsage();
    static double getMemoryUsage();
    static double getDiskUsage();
    
    // 组件状态检查
    static bool checkDatabaseConnection();
    static bool checkCacheConnection();
    static bool checkExternalService();
};

} // namespace webserver

#endif // WEBSERVER_HEALTHCHECK_CONTROLLER_H