# C++ 实现详解

## 1. C++ 特性应用

### 1.1 C++14 标准特性

WebServer 项目充分利用了 C++14 标准提供的现代 C++ 特性，以提高代码质量和开发效率。

#### 1.1.1 自动类型推导
```cpp
// 使用 auto 简化复杂类型声明
auto task = std::make_shared<std::packaged_task<return_type()>>(
    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
);
```

#### 1.1.2 泛型 Lambda 表达式
在测试和示例代码中广泛使用 Lambda 表达式：
```cpp
server->addRoute("/hello", [](const std::map<std::string, std::string>& headers, const std::string& body) {
    return "<html><body><h1>Hello, World!</h1></body></html>";
});
```

#### 1.1.3 二进制字面量和数字分隔符
提高数值可读性：
```cpp
auto largeNumber = 1'000'000;  // C++14 数字分隔符
```

### 1.2 智能指针应用

项目中广泛使用智能指针管理动态内存，确保内存安全：

#### 1.2.1 unique_ptr
用于独占所有权的资源管理：
```cpp
std::unique_ptr<ConnectionManager> connectionManager_;
std::unique_ptr<Router> router_;
```

#### 1.2.2 shared_ptr
用于共享所有权的场景：
```cpp
auto task = std::make_shared<std::packaged_task<return_type()>>(
    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
);
```

### 1.3 移动语义和完美转发

#### 1.3.1 移动构造和移动赋值
在 ThreadPool 中使用移动语义优化性能：
```cpp
template<class F, class... Args>
auto enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>
```

#### 1.3.2 完美转发
通过 std::forward 保持参数的值类别：
```cpp
std::bind(std::forward<F>(f), std::forward<Args>(args)...)
```

## 2. 核心组件实现细节

### 2.1 WebServer 类实现

#### 2.1.1 构造与初始化
```cpp
WebServer::WebServer(const Config& config)
    : port_(config.get<int>("server.port", 8080))
    , running_(false)
    , config_(config)
    , sslContext_(nullptr) {
    // 初始化组件
    router_ = std::make_unique<Router>();
    connectionManager_ = std::make_unique<ConnectionManager>(config);
}
```

#### 2.1.2 RAII 资源管理
通过构造函数和析构函数自动管理资源：
```cpp
WebServer::~WebServer() {
    stop();
    cleanupSSL();
}
```

### 2.2 ThreadPool 实现详解

线程池是 WebServer 并发处理的核心组件。

#### 2.2.1 构造函数实现
```cpp
ThreadPool::ThreadPool(size_t threads)
    : stop_(false) {
    // 创建指定数量的工作线程
    for(size_t i = 0; i < threads; ++i) {
        workers_.emplace_back([this] {
            // 工作线程主循环
            for(;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queueMutex_);
                    this->condition_.wait(lock, [this]{ return this->stop_ || !this->tasks_.empty(); });
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
```

#### 2.2.2 任务提交机制
```cpp
template<class F, class... Args>
auto enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        if(stop_)
            throw std::runtime_error("enqueue on stopped ThreadPool");
        tasks_.emplace([task](){ (*task)(); });
    }
    condition_.notify_one();
    return res;
}
```

### 2.3 MemoryPool 实现详解

内存池通过预分配和复用内存块来提高内存分配效率。

#### 2.3.1 模板设计
```cpp
template<typename T, size_t BlockSize = 1024>
class MemoryPool {
    // ...
};
```

#### 2.3.2 联合体优化
```cpp
union Slot {
    T element;      // 存储实际对象
    Slot* next;     // 指向下一个空闲槽位
};
```

#### 2.3.3 内存分配实现
```cpp
template<typename T, size_t BlockSize>
T* MemoryPool<T, BlockSize>::allocate() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!freeSlots_) {
        // 分配新的内存块
        auto newBlock = std::make_unique<Block>();
        newBlock->next = currentBlock_;
        currentBlock_ = newBlock.get();
        blocks_.push_back(std::move(newBlock));
        
        // 将新块中的槽位加入空闲列表
        for (size_t i = 0; i < BlockSize - 1; ++i) {
            currentBlock_->slots[i].next = &currentBlock_->slots[i + 1];
        }
        currentBlock_->slots[BlockSize - 1].next = nullptr;
        freeSlots_ = &currentBlock_->slots[0];
    }
    
    Slot* slot = freeSlots_;
    freeSlots_ = freeSlots_->next;
    return reinterpret_cast<T*>(slot);
}
```

### 2.4 Logger 实现详解

日志系统采用单例模式，提供线程安全的日志记录功能。

#### 2.4.1 单例模式实现
```cpp
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}
```

#### 2.4.2 线程安全保证
```cpp
void Logger::log(Level level, const std::string& message) {
    if (level < minLevel) return;
    
    std::lock_guard<std::mutex> lock(logMutex);
    
    std::string logMessage = getCurrentTimestamp() + " [" + getLevelString(level) + "] " + message + "\n";
    
    if (outputStream) {
        *outputStream << logMessage;
        outputStream->flush();
    }
    
    if (consoleOutput) {
        std::cout << logMessage;
        std::cout.flush();
    }
}
```

### 2.5 Config 实现详解

配置管理器基于 nlohmann/json 库实现，支持嵌套配置项访问。

#### 2.5.1 模板方法设计
```cpp
template<typename T>
T Config::get(const std::string& key, const T& defaultValue) const {
    try {
        return config_.at(key).get<T>();
    } catch (...) {
        return defaultValue;
    }
}
```

#### 2.5.2 嵌套配置访问
```cpp
template<typename T>
T Config::getNestedValue(const std::string& path, const T& defaultValue) const {
    const nlohmann::json* obj = getNestedObject(path);
    if (obj) {
        try {
            return obj->get<T>();
        } catch (...) {
            // 类型转换失败，返回默认值
        }
    }
    return defaultValue;
}
```

## 3. STL 容器和算法应用

### 3.1 常用容器选择

#### 3.1.1 std::vector
用于存储连续内存数据，如线程池中的工作线程列表：
```cpp
std::vector<std::thread> workers_;
```

#### 3.1.2 std::map
用于键值对映射，如路由表和 HTTP 头部：
```cpp
std::map<std::string, RequestHandler> routes_;
std::map<std::string, std::string> headers;
```

#### 3.1.3 std::queue
用于任务队列管理：
```cpp
std::queue<std::function<void()>> tasks_;
```

### 3.2 算法应用

#### 3.2.1 std::bind 和 std::function
用于实现回调机制和任务封装：
```cpp
using RequestHandler = std::function<std::string(const std::map<std::string, std::string>&, const std::string&)>;
```

#### 3.2.2 std::mutex 和 std::condition_variable
用于线程同步：
```cpp
std::mutex queueMutex_;
std::condition_variable condition_;
```

## 4. 异常安全设计

### 4.1 异常处理策略

#### 4.1.1 异常边界
在关键位置设置异常处理边界，防止异常传播导致系统崩溃：
```cpp
try {
    // 服务器启动逻辑
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    delete server;
    return 1;
}
```

#### 4.1.2 资源安全释放
使用 RAII 确保异常发生时资源能够正确释放：
```cpp
std::lock_guard<std::mutex> lock(mutex_);
// 即使发生异常，锁也会自动释放
```

## 5. 性能优化技术

### 5.1 内存优化

#### 5.1.1 内存池技术
通过预分配和复用内存块减少动态分配开销。

#### 5.1.2 对象复用
在连接管理中复用 ConnectionInfo 结构体。

### 5.2 并发优化

#### 5.2.1 锁粒度控制
尽量减小锁的粒度，提高并发性能：
```cpp
{
    std::unique_lock<std::mutex> lock(queueMutex_);
    // 仅在关键部分持有锁
    // ...
}
```

#### 5.2.2 无锁编程
在原子操作中使用 std::atomic：
```cpp
std::atomic<uint64_t> totalRequests_{0};
std::atomic<bool> running_;
```

## 6. 网络编程实现

### 6.1 Socket 编程

#### 6.1.1 非阻塞 I/O
使用 POSIX Socket API 实现网络通信。

#### 6.1.2 连接管理
通过 ConnectionManager 管理所有客户端连接。

### 6.2 SSL/TLS 支持

#### 6.2.1 OpenSSL 集成
通过 OpenSSL 库实现 HTTPS 支持。

#### 6.2.2 证书管理
支持 SSL 证书和私钥配置。