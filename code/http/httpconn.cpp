/*
 * @Author       : mark
 * @Date         : 2020-06-15
 * @copyleft Apache 2.0
 */ 
#include "httpconn.h"


const char* HttpConn::srcDir;
std::atomic<int> HttpConn::userCount;
bool HttpConn::isET;

/**
 * @brief HttpConn构造函数
 * @details 初始化Http连接对象，设置文件描述符为-1，清空地址结构，标记连接为关闭状态
 */
HttpConn::HttpConn() { 
    fd_ = -1;
    memset(&addr_, 0, sizeof(addr_));
    isClose_ = true;
};

HttpConn::~HttpConn() { 
    Close(); 
};

/**
 * @brief 初始化Http连接
 * @param fd 客户端socket文件描述符
 * @param addr 客户端socket地址结构
 * @details 初始化连接参数，重置读写缓冲区，增加用户计数，记录日志
 */
void HttpConn::init(int fd, const sockaddr_in& addr) {
    assert(fd > 0);
    userCount++;
    addr_ = addr;
    fd_ = fd;
    writeBuff_.RetrieveAll();
    readBuff_.RetrieveAll();
    isClose_ = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
}

void HttpConn::Close() {
    response_.UnmapFile();
    if(isClose_ == false){
        isClose_ = true; 
        userCount--;
        close(fd_);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
    }
}

/**
 * @brief 获取客户端socket文件描述符
 * @return int 客户端socket文件描述符
 */
int HttpConn::GetFd() const {
    return fd_;
};

/**
 * @brief 获取客户端socket地址结构
 * @return sockaddr_in 客户端socket地址结构
 */
struct sockaddr_in HttpConn::GetAddr() const {
    return addr_;
}

/**
 * @brief 获取客户端IP地址
 * @return const char* 客户端IP地址字符串
 */
const char* HttpConn::GetIP() const {
    return inet_ntoa(addr_.sin_addr);
}

/**
 * @brief 获取客户端端口号
 * @return int 客户端端口号
 */
int HttpConn::GetPort() const {
    return addr_.sin_port;
}

/**
 * @brief 从客户端socket读取数据
 * @param saveErrno 保存错误码的指针
 * @return ssize_t 读取的字节数，-1表示出错
 * @details 根据ET模式决定是否循环读取
 */
ssize_t HttpConn::read(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = readBuff_.ReadFd(fd_, saveErrno);
        if (len <= 0) {
            break;
        }
    } while (isET);
    return len;
}

ssize_t HttpConn::write(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(fd_, iov_, iovCnt_);
        if(len <= 0) {
            *saveErrno = errno;
            break;
        }
        if(iov_[0].iov_len + iov_[1].iov_len  == 0) { break; } /* 传输结束 */
        else if(static_cast<size_t>(len) > iov_[0].iov_len) {
            iov_[1].iov_base = (uint8_t*) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if(iov_[0].iov_len) {
                writeBuff_.RetrieveAll();
                iov_[0].iov_len = 0;
            }
        }
        else {
            iov_[0].iov_base = (uint8_t*)iov_[0].iov_base + len; 
            iov_[0].iov_len -= len; 
            writeBuff_.Retrieve(len);
        }
    } while(isET || ToWriteBytes() > 10240);
    return len;
}

/**
 * @brief 处理HTTP请求
 * @return bool 处理是否成功
 * @details 解析请求，生成响应，设置iovec结构用于发送响应
 */
bool HttpConn::process() {
    request_.Init();
    if(readBuff_.ReadableBytes() <= 0) {
        return false;
    }
    else if(request_.parse(readBuff_)) {
        LOG_DEBUG("%s", request_.path().c_str());
        response_.Init(srcDir, request_.path(), request_.IsKeepAlive(), 200);
    } else {
        response_.Init(srcDir, request_.path(), false, 400);
    }

    response_.MakeResponse(writeBuff_);
    /* 响应头 */
    iov_[0].iov_base = const_cast<char*>(writeBuff_.Peek());
    iov_[0].iov_len = writeBuff_.ReadableBytes();
    iovCnt_ = 1;

    /* 文件 */
    if(response_.FileLen() > 0  && response_.File()) {
        iov_[1].iov_base = response_.File();
        iov_[1].iov_len = response_.FileLen();
        iovCnt_ = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", response_.FileLen() , iovCnt_, ToWriteBytes());
    return true;
}
