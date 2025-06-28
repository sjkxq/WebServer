#ifndef WEBSERVER_SSLCONTEXT_H
#define WEBSERVER_SSLCONTEXT_H

#include <openssl/ssl.h>
#include <string>

namespace webserver {

class SSLContext {
public:
    SSLContext();
    ~SSLContext();

    bool init();
    bool loadCertificate(const std::string& certPath, const std::string& keyPath);
    SSL_CTX* get() const { return ctx_; }

private:
    SSL_CTX* ctx_;
};

} // namespace webserver

#endif // WEBSERVER_SSLCONTEXT_H