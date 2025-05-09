/*
 * @Author       : mark
 * @Date         : 2020-06-17
 * @copyleft Apache 2.0
 */

#include "webserver.h"
#include <iostream>

#include "../config/config.h"

WebServer::WebServer(
            int port, int trigMode, int timeoutMS, bool OptLinger,
            int sqlPort, const char* sqlUser, const char* sqlPwd,
            const char* dbName, int connPoolNum, int threadNum,
            bool openLog, int logLevel, int logQueSize):
            port_(port), openLinger_(OptLinger), timeoutMS_(timeoutMS), isClose_(false),
            timer_(new HeapTimer()), threadpool_(new ThreadPool(threadNum)), epoller_(new Epoller()) {
    (void)trigMode;
    (void)sqlPort;
    (void)sqlUser;
    (void)sqlPwd;
    (void)dbName;
    (void)connPoolNum;
    (void)openLog;
    (void)logLevel;
    (void)logQueSize;
}

WebServer::WebServer(const char* configFile):
            isClose_(false), timer_(new HeapTimer()), epoller_(new Epoller()) {
    
    // 从配置文件读取参数
    Config config(configFile);
    port_ = config.GetInt("port", 9006);
    int trigMode = config.GetInt("trigMode", 3);
    timeoutMS_ = config.GetInt("timeoutMS", 60000);
    openLinger_ = config.GetBool("openLinger", false);
    int sqlPort = config.GetInt("sqlPort", 3306);
    std::string sqlUser = config.GetString("sqlUser", "root");
    std::string sqlPwd = config.GetString("sqlPwd", "123456");
    std::string dbName = config.GetString("dbName", "webserver");
    int connPoolNum = config.GetInt("connPoolNum", 12);
    int threadNum = config.GetInt("threadNum", 6);
    bool openLog = config.GetBool("openLog", true);
    int logLevel = config.GetInt("logLevel", 1);
    int logQueSize = config.GetInt("logQueSize", 1024);

    srcDir_ = getcwd(nullptr, 256);
    assert(srcDir_);
    strncat(srcDir_, "/resources/", 16);
    HttpConn::userCount = 0;
    HttpConn::srcDir = srcDir_;
    SqlConnPool::Instance()->Init("localhost", sqlPort, sqlUser.c_str(), sqlPwd.c_str(), dbName.c_str(), connPoolNum);

    InitEventMode_(trigMode);
    if(!InitSocket_()) { isClose_ = true;}

    if(openLog) {
        Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
        if(isClose_) { LOG_ERROR("========== Server init error!=========="); }
        else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", port_, openLinger_? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (listenEvent_ & EPOLLET ? "ET": "LT"),
                            (connEvent_ & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }
}

/**
 * @brief WebServer析构函数
 * @details 关闭监听socket，释放资源，关闭数据库连接池
 */
WebServer::~WebServer() {
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
    SqlConnPool::Instance()->ClosePool();
}

void WebServer::InitEventMode_(int trigMode) {
    listenEvent_ = EPOLLRDHUP;
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode) {
    case 0:
        break;
    case 1:
        connEvent_ |= EPOLLET;
        break;
    case 2:
        listenEvent_ |= EPOLLET;
        break;
    case 3:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    default:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    }
}

void WebServer::Start() {
    int timeMS = -1;
    if(!isClose_) { LOG_INFO("========== Server start =========="); }
    std::cout << "服务器启动成功，监听端口" << port_ << std::endl;
    LOG_INFO("服务器启动成功，监听端口%d", port_);
    while(!isClose_) {
        if(timeoutMS_ > 0) {
            timeMS = timer_->GetNextTick();
        }
        auto eventStart = std::chrono::steady_clock::now();
        int eventCnt = epoller_->Wait(timeMS);
        auto eventEnd = std::chrono::steady_clock::now();
        auto eventDuration = std::chrono::duration_cast<std::chrono::milliseconds>(eventEnd - eventStart);
        LOG_DEBUG("Event loop processed in %lld ms", eventDuration.count());
        for(int i = 0; i < eventCnt; i++) {
            int fd = epoller_->GetEventFd(i);
            uint32_t events = epoller_->GetEvents(i);
            if(fd == listenFd_) {
                DealListen_();
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                CloseConn_(&users_[fd]);
            }
            else if(events & EPOLLIN) {
                DealRead_(&users_[fd]);
            }
            else if(events & EPOLLOUT) {
                DealWrite_(&users_[fd]);
            } else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void WebServer::SendError_(int fd, const char*info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
}
void WebServer::CloseConn_(HttpConn* client) {
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd());
    epoller_->DelFd(client->GetFd());
    client->Close();
}
void WebServer::AddClient_(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].init(fd, addr);
    if(timeoutMS_ > 0) {
        timer_->add(fd, timeoutMS_, std::bind(&WebServer::CloseConn_, this, &users_[fd]));
    }
    epoller_->AddFd(fd, EPOLLIN | connEvent_);
    SetFdNonblock(fd);
    LOG_INFO("Client[%d] in!", users_[fd].GetFd());
}
void WebServer::DealListen_() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    while(true) {
        int fd = accept(listenFd_, (struct sockaddr*)&addr, &len);
        if(fd <= 0) { break; }
        else if(HttpConn::userCount >= MAX_FD) {
            SendError_(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            continue;
        }
        AddClient_(fd, addr);
    }
}

/**
 * @brief 处理客户端读事件
 * @param client HTTP连接对象指针
 */
void WebServer::DealRead_(HttpConn* client) {
    assert(client);
    auto start = std::chrono::steady_clock::now();
    ExtentTime_(client);
    threadpool_->AddTask(std::bind(&WebServer::OnRead_, this, client));
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    LOG_DEBUG("Read request processed in %lld ms", duration.count());
}

/**
 * @brief 处理客户端写事件
 * @param client HTTP连接对象指针
 */
/**
 * @brief 处理客户端写事件
 * @param client HTTP连接对象指针
 */
void WebServer::DealWrite_(HttpConn* client) {
    assert(client);
    auto start = std::chrono::steady_clock::now();
    ExtentTime_(client);
    threadpool_->AddTask(std::bind(&WebServer::OnWrite_, this, client));
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    LOG_DEBUG("Write request processed in %lld ms", duration.count());
}

void WebServer::ExtentTime_(HttpConn* client) {
    assert(client);
    if(timeoutMS_ > 0) { timer_->adjust(client->GetFd(), timeoutMS_); }
}

void WebServer::OnRead_(HttpConn* client) {
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN) {
        CloseConn_(client);
        return;
    }
    OnProcess(client);
}

/**
 * @brief 处理HTTP请求并统计耗时
 * @param client HTTP连接对象指针
 * @details 记录请求处理开始和结束时间，计算耗时并记录日志
 */
void WebServer::OnProcess(HttpConn* client) {
    auto start = std::chrono::steady_clock::now();
    bool processResult = client->process();
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    LOG_DEBUG("Request processed in %lld ms", duration.count());
    
    if(processResult) {
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
    } else {
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLIN);
    }
}

void WebServer::OnWrite_(HttpConn* client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if(client->ToWriteBytes() == 0) {
        /* 传输完成 */
        if(client->IsKeepAlive()) {
            OnProcess(client);
            return;
        }
    }
    else if(ret < 0) {
        if(writeErrno == EAGAIN) {
            /* 继续传输 */
            epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
            return;
        }
    }
    CloseConn_(client);
}

/* Create listenFd */
bool WebServer::InitSocket_() {
    int ret;
    struct sockaddr_in addr;
    
    /**
     * @brief 验证端口号有效性
     * @details 检查端口号是否在1024-65535范围内
     */
    if(port_ > 65535 || port_ < 1024) {
        LOG_ERROR("Port:%d error! Port must be between 1024 and 65535", port_);
        return false;
    }
    
    /**
     * @brief 检查端口是否被占用
     * @details 尝试绑定临时socket来检测端口是否可用
     */
    int testFd = socket(AF_INET, SOCK_STREAM, 0);
    if(testFd < 0) {
        LOG_ERROR("Create test socket error!");
        return false;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);
    
    ret = bind(testFd, (struct sockaddr *)&addr, sizeof(addr));
    close(testFd);
    if(ret < 0) {
        LOG_ERROR("Port:%d is already in use!", port_);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);
    struct linger optLinger = {};
    memset(&optLinger, 0, sizeof(optLinger));
    if(openLinger_) {
        /* 优雅关闭: 直到所剩数据发送完毕或超时 */
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ < 0) {
        LOG_ERROR("Create socket error!", port_);
        return false;
    }

    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(listenFd_);
        LOG_ERROR("Init linger error!", port_);
        return false;
    }

    int optval = 1;
    /* 端口复用 */
    /* 只有最后一个套接字会正常接收数据。 */
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        LOG_ERROR("set socket setsockopt error !");
        close(listenFd_);
        return false;
    }

    ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        LOG_ERROR("Bind Port:%d error!", port_);
        close(listenFd_);
        return false;
    }

    ret = listen(listenFd_, 6);
    if(ret < 0) {
        LOG_ERROR("Listen port:%d error!", port_);
        close(listenFd_);
        return false;
    }
    ret = epoller_->AddFd(listenFd_,  listenEvent_ | EPOLLIN);
    if(ret == 0) {
        LOG_ERROR("Add listen error!");
        close(listenFd_);
        return false;
    }
    SetFdNonblock(listenFd_);
    LOG_INFO("Server port:%d", port_);
    return true;
}

int WebServer::SetFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}


