#include "ssl/SSLContext.h"
#include <openssl/err.h>
#include <stdexcept>

namespace webserver {

SSLContext::SSLContext() : ctx_(nullptr) {
}

SSLContext::~SSLContext() {
    if (ctx_) {
        SSL_CTX_free(ctx_);
    }
}

bool SSLContext::init() {
    const SSL_METHOD* method = TLS_server_method();
    if (!method) {
        return false;
    }

    ctx_ = SSL_CTX_new(method);
    if (!ctx_) {
        return false;
    }

    // Set minimum TLS version to 1.2
    SSL_CTX_set_min_proto_version(ctx_, TLS1_2_VERSION);

    // Configure secure cipher suites
    if (!SSL_CTX_set_cipher_list(ctx_, "HIGH:!aNULL:!MD5:!RC4")) {
        return false;
    }

    return true;
}

bool SSLContext::loadCertificate(const std::string& certPath, const std::string& keyPath) {
    if (!ctx_) {
        return false;
    }

    // Load certificate
    if (SSL_CTX_use_certificate_file(ctx_, certPath.c_str(), SSL_FILETYPE_PEM) <= 0) {
        return false;
    }

    // Load private key
    if (SSL_CTX_use_PrivateKey_file(ctx_, keyPath.c_str(), SSL_FILETYPE_PEM) <= 0) {
        return false;
    }

    // Verify private key matches certificate
    if (!SSL_CTX_check_private_key(ctx_)) {
        return false;
    }

    return true;
}

} // namespace webserver