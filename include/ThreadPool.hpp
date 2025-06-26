#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>

// 线程池类，用于管理多个工作线程并分配异步任务
// 禁止拷贝和赋值操作
class ThreadPool {
    // 禁止拷贝构造
    ThreadPool(const ThreadPool&) = delete;
    // 禁止赋值操作
    ThreadPool& operator=(const ThreadPool&) = delete;
public:
    // 构造函数：初始化指定数量的工作线程
    // threads: 要创建的工作线程数量
    explicit ThreadPool(size_t threads);
    // 析构函数：销毁线程池，释放所有资源
    ~ThreadPool();

    // 提交任务到线程池的队列中
    // F: 可调用对象类型（如函数、lambda表达式）
    // Args: 参数包类型
    // f: 要执行的任务函数或可调用对象
    // args: 传递给f的参数列表
    // 返回值：表示异步结果的std::future对象
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;

        // 将任务封装为packaged_task，绑定参数后获取其future结果
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        // 获取与任务关联的future，用于返回结果
        std::future<return_type> res = task->get_future();
        {
            // 加锁确保队列访问的互斥性
            std::unique_lock<std::mutex> lock(queueMutex_);
            if(stop_)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            // 将任务添加到队列中
            tasks_.emplace([task](){ (*task)(); });
        }
        // 唤醒一个等待的线程来处理新任务
        condition_.notify_one();
        return res;
    }

private:
    // 工作线程列表
    std::vector<std::thread> workers_;
    // 待处理任务队列
    std::queue<std::function<void()>> tasks_;
    
    // 保护任务队列的互斥量
    std::mutex queueMutex_;
    // 条件变量，用于线程间通信
    std::condition_variable condition_;
    // 停止标志，指示线程池是否已关闭
    bool stop_;
};