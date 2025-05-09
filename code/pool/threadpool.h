/*
 * @Author       : mark
 * @Date         : 2020-06-15
 * @copyleft Apache 2.0
 */ 

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
class ThreadPool {
public:
    /**
 * @brief 线程池构造函数
 * @param threadCount 线程数量，默认为8
 * @param maxQueueSize 任务队列最大长度，0表示无限制
 */
explicit ThreadPool(size_t threadCount = 8, size_t maxQueueSize = 0): 
    pool_(std::make_shared<Pool>()),
    maxQueueSize_(maxQueueSize) {
    assert(threadCount > 0);
    for(size_t i = 0; i < threadCount; i++) {
        std::thread([pool = pool_] {
            std::unique_lock<std::mutex> locker(pool->mtx);
            while(true) {
                if(!pool->tasks.empty()) {
                    auto task = std::move(pool->tasks.front());
                    pool->tasks.pop();
                    locker.unlock();
                    task();
                    locker.lock();
                } 
                else if(pool->isClosed) break;
                else pool->cond.wait(locker);
            }
        }).detach();
    }
    }

    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;
    
    ~ThreadPool() {
        if(static_cast<bool>(pool_)) {
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;
            }
            pool_->cond.notify_all();
        }
    }

    /**
 * @brief 添加任务到线程池
 * @param task 要执行的任务
 * @return bool 添加成功返回true，队列已满返回false
 */
template<class F>
bool AddTask(F&& task) {
    if(maxQueueSize_ > 0 && pool_->tasks.size() >= maxQueueSize_) {
        return false;
    }
    {
        std::lock_guard<std::mutex> locker(pool_->mtx);
        pool_->tasks.emplace(std::forward<F>(task));
    }
    pool_->cond.notify_one();
    return true;
}

private:
    struct Pool {
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed;
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<Pool> pool_;
    size_t maxQueueSize_ = 0;
};


#endif //THREADPOOL_H