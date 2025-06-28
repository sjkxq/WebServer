#ifndef WEBSERVER_SOCKET_H
#define WEBSERVER_SOCKET_H

#include <sys/types.h>

namespace webserver {

class Socket {
public:
    explicit Socket(int fd);
    virtual ~Socket() = default;

    virtual ssize_t read(void* buf, size_t count) = 0;
    virtual ssize_t write(const void* buf, size_t count) = 0;
    virtual bool close() = 0;

    int getFd() const { return fd_; }

protected:
    int fd_;
};

} // namespace webserver

#endif // WEBSERVER_SOCKET_H