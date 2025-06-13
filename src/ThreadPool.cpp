#include "ThreadPool.hpp"
#include "Logger.hpp"

ThreadPool::ThreadPool(size_t threads) 
    : stop_(false) {
    LOG_INFO("Creating ThreadPool with " + std::to_string(threads) + " threads");
    for(size_t i = 0; i < threads; ++i) {
        workers_.emplace_back([this] {
            for(;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queueMutex_);
                    this->condition_.wait(lock, [this] { 
                        return this->stop_ || !this->tasks_.empty(); 
                    });
                    if(this->stop_ && this->tasks_.empty())
                        return;
                    task = std::move(this->tasks_.front());
                    this->tasks_.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    LOG_INFO("Destroying ThreadPool");
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        LOG_DEBUG("Setting stop flag and notifying workers");
        stop_ = true;
    }
    condition_.notify_all();
    for(std::thread &worker: workers_) {
        if(worker.joinable()) {
            LOG_DEBUG("Joining worker thread");
            worker.join();
        }
    }
    LOG_INFO("ThreadPool destroyed");
}