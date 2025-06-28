#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H

#include "Config.h"
#include "ConnectionManager.h"
#include "Router.h"
#include <memory>
#include <openssl/ssl.h>

namespace webserver {

class WebServer {
public:
    explicit WebServer(const Config& config);
    ~WebServer();

    bool start();
    void stop();
    void addRoute(const std::string& path, Router::RequestHandler handler);

private:
    bool initSSLContext();
    void cleanupSSL();
    void handleConnection(int clientSocket);

    int port_;
    bool running_;
    Config config_;
    std::unique_ptr<ConnectionManager> connectionManager_;
    std::unique_ptr<Router> router_;
    SSL_CTX* sslContext_;
};

} // namespace webserver

#endif // WEBSERVER_WEBSERVER_H