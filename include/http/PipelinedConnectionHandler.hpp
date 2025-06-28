#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

class PipelinedConnectionHandler {
public:
    PipelinedConnectionHandler(int socket);
    ~PipelinedConnectionHandler();

    /**
     * @brief 启动管道化请求处理
     */
    void start();

    /**
     * @brief 停止管道化请求处理
     */
    void stop();

private:
    // 请求处理线程函数
    void requestProcessingThread();
    void responseProcessingThread();

    int clientSocket;
    std::atomic<bool> running;

    // 请求队列
    std::queue<HttpRequest> requestQueue;
    std::mutex requestMutex;
    std::condition_variable requestCV;

    // 响应队列
    std::queue<HttpResponse> responseQueue;
    std::mutex responseMutex;
    std::condition_variable responseCV;

    // 线程
    std::thread requestThread;
    std::thread responseThread;
};