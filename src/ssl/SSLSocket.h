#ifndef WEBSERVER_SSLSOCKET_H
#define WEBSERVER_SSLSOCKET_H

#include "Socket.h"
#include <openssl/ssl.h>

namespace webserver {

class SSLSocket : public Socket {
public:
    SSLSocket(int fd, SSL* ssl);
    ~SSLSocket();

    ssize_t read(void* buf, size_t count) override;
    ssize_t write(const void* buf, size_t count) override;
    bool close() override;

    static SSLSocket* create(int fd, SSL_CTX* ctx);

private:
    SSL* ssl_;
};

} // namespace webserver

#endif // WEBSERVER_SSLSOCKET_H