#include "ssl/SSLSocket.h"
#include <openssl/err.h>
#include <unistd.h>

namespace webserver {

SSLSocket::SSLSocket(int fd, SSL* ssl) : Socket(fd), ssl_(ssl) {
    SSL_set_fd(ssl_, fd);
}

SSLSocket::~SSLSocket() {
    if (ssl_) {
        SSL_shutdown(ssl_);
        SSL_free(ssl_);
    }
}

ssize_t SSLSocket::read(void* buf, size_t count) {
    int ret = SSL_read(ssl_, buf, static_cast<int>(count));
    if (ret <= 0) {
        int err = SSL_get_error(ssl_, ret);
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
            return 0; // Would block
        }
        return -1; // Error
    }
    return ret;
}

ssize_t SSLSocket::write(const void* buf, size_t count) {
    int ret = SSL_write(ssl_, buf, static_cast<int>(count));
    if (ret <= 0) {
        int err = SSL_get_error(ssl_, ret);
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
            return 0; // Would block
        }
        return -1; // Error
    }
    return ret;
}

bool SSLSocket::close() {
    if (ssl_) {
        SSL_shutdown(ssl_);
    }
    return Socket::close();
}

SSLSocket* SSLSocket::create(int fd, SSL_CTX* ctx) {
    SSL* ssl = SSL_new(ctx);
    if (!ssl) {
        return nullptr;
    }

    if (SSL_accept(ssl) <= 0) {
        SSL_free(ssl);
        return nullptr;
    }

    return new SSLSocket(fd, ssl);
}

} // namespace webserver