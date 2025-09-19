# 详细设计文档

## 1. 核心模块详细设计

### 1.1 WebServer 模块

#### 1.1.1 类结构设计

```cpp
class WebServer {
public:
    WebServer(const Config& config);
    ~WebServer();
    
    bool start();
    void stop();
    void addRoute(const std::string& path, Router::RequestHandler handler);

private:
    void handleConnection(int clientSocket);
    bool initSSLContext();
    void cleanupSSL();

    int port_;
    bool running_;
    std::unique_ptr<ConnectionManager> connectionManager_;
    std::unique_ptr<Router> router_;
    Config config_;
    SSL_CTX* sslContext_;
};
```

#### 1.1.2 启动流程设计

1. 初始化 SSL 上下文（如果启用 HTTPS）
2. 创建监听套接字
3. 绑定到指定端口
4. 开始监听连接
5. 启动连接管理器
6. 进入主循环等待连接

#### 1.1.3 连接处理设计

1. 接受客户端连接
2. 将连接交给连接管理器处理
3. 连接管理器分配线程处理请求
4. 处理完成后关闭连接或保持 Keep-Alive

### 1.2 ConnectionManager 模块

#### 1.2.1 类结构设计

```cpp
struct ConnectionInfo {
    std::thread thread;
    std::chrono::steady_clock::time_point lastActivity;
    int requestCount;
    bool keepAlive;
    std::string clientIP;
};

class ConnectionManager {
public:
    using ConnectionHandler = std::function<void()>;
    
    explicit ConnectionManager(const Config& config);
    ~ConnectionManager();
    
    void addConnection(int socket, const std::string& clientIP, ConnectionHandler handler);
    void closeConnection(int socket);
    void stopAll();
    void updateActivity(int socket);
    void setKeepAlive(int socket, bool keepAlive);
    size_t getConnectionCount() const;
    size_t getActiveConnectionCount() const;
    uint64_t getTotalRequestCount() const;
    std::string getConnectionStats() const;

private:
    void cleanupTask();
    
    std::map<int, ConnectionInfo> connections_;
    std::map<std::string, int> ipConnections_;
    mutable std::mutex connectionsMutex_;
    const Config& config_;
    std::atomic<bool> running_;
    std::atomic<uint64_t> totalRequests_;
    
    // 配置项
    int maxConnectionsPerClient_;
    int maxConnectionsPerIP_;
    int connectionTimeout_;
    int keepAliveTimeout_;
    int maxRequestsPerConnection_;
    int connectionCleanupInterval_;
    
    std::thread cleanupThread_;
    std::condition_variable cleanupCV_;
};
```

#### 1.2.2 连接管理策略

1. **连接数限制**：
   - 每个 IP 最大连接数限制
   - 每个客户端最大连接数限制

2. **连接超时管理**：
   - Keep-Alive 连接超时时间
   - 普通连接超时时间

3. **后台清理机制**：
   - 定期检查过期连接
   - 自动关闭超时连接

#### 1.2.3 线程安全设计

1. 使用 `std::mutex` 保护连接信息表
2. 使用 `std::atomic` 实现无锁计数
3. 使用 `std::condition_variable` 实现线程间通信

### 1.3 ThreadPool 模块

#### 1.3.1 类结构设计

```cpp
class ThreadPool {
public:
    explicit ThreadPool(size_t threads);
    ~ThreadPool();
    
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    bool stop_;
};
```

#### 1.3.2 工作线程设计

1. **线程主循环**：
   ```cpp
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
   ```

2. **任务提交机制**：
   - 使用模板支持任意可调用对象
   - 返回 `std::future` 获取执行结果
   - 线程安全的任务队列管理

#### 1.3.3 线程生命周期管理

1. 构造函数中创建指定数量的工作线程
2. 析构函数中通知停止并等待所有线程结束
3. 使用 RAII 确保资源正确释放

### 1.4 Router 模块

#### 1.4.1 类结构设计

```cpp
class Router {
public:
    using RequestHandler = std::function<std::string(const std::map<std::string, std::string>&, const std::string&)>;
    
    Router();
    ~Router() = default;
    
    void addRoute(const std::string& path, RequestHandler handler);
    std::pair<bool, std::string> handleRequest(const std::string& path, 
        const std::map<std::string, std::string>& headers, 
        const std::string& body) const;

private:
    std::map<std::string, RequestHandler> routes_;
};
```

#### 1.4.2 路由匹配机制

1. 精确路径匹配
2. 简单的路由表实现（可扩展为前缀树等高效结构）
3. 支持动态注册路由处理函数

#### 1.4.3 处理函数设计

1. 使用 `std::function` 封装处理函数
2. 支持 Lambda 表达式和函数对象
3. 传递请求头和请求体给处理函数

### 1.5 HttpParser 模块

#### 1.5.1 类结构设计

```cpp
class HttpParser {
public:
    static std::tuple<std::string, std::string, std::map<std::string, std::string>, std::string> parseRequest(const std::string& request);
    static std::string buildResponse(HttpStatus statusCode, const std::string& content, const std::string& contentType = "text/html");
    static std::string buildResponse(HttpStatus statusCode, const std::string& content, 
                                    const std::map<std::string, std::string>& headers, 
                                    const std::string& contentType = "text/html");
    static std::string buildChunkedResponse(HttpStatus statusCode, const std::string& content, const std::string& contentType = "text/html");
    static std::string buildChunkedResponse(HttpStatus statusCode, const std::string& content, 
                                          const std::map<std::string, std::string>& headers, 
                                          const std::string& contentType = "text/html");
    static HttpRequest parseRequestToObject(const std::string& request);
    static std::string buildResponse(const HttpResponse& response);
    static std::string buildChunkedResponse(const HttpResponse& response);
};
```

#### 1.5.2 请求解析设计

1. **请求行解析**：
   - 解析 HTTP 方法
   - 解析请求路径
   - 解析协议版本

2. **请求头解析**：
   - 按行分割请求头
   - 解析键值对
   - 处理多行头字段

3. **请求体解析**：
   - 根据 Content-Length 解析
   - 支持分块传输编码

#### 1.5.3 响应构建设计

1. **标准响应**：
   - 构建状态行
   - 添加响应头
   - 添加响应体

2. **分块响应**：
   - 支持分块传输编码
   - 自动计算块大小
   - 添加结束块

### 1.6 Logger 模块

#### 1.6.1 类结构设计

```cpp
class Logger {
public:
    enum class Level {
        TRACE, DEBUG_LEVEL, INFO, WARNING, ERROR, FATAL
    };

    static Logger& getInstance();
    
    void setLogFile(const std::string& filename);
    void setStream(std::ostream& stream);
    void setConsoleOutput(bool enable);
    void log(Level level, const std::string& message);
    void setLogLevel(Level level) { minLevel = level; }

private:
    Logger();
    ~Logger();
    
    std::string getLevelString(Level level) const;
    std::string getCurrentTimestamp() const;

    Level minLevel;
    std::ostream* outputStream;
    std::ofstream fileStream;
    bool consoleOutput;
    std::mutex logMutex;
};
```

#### 1.6.2 日志级别设计

1. TRACE: 跟踪信息，用于调试
2. DEBUG: 调试信息，开发阶段使用
3. INFO: 一般信息，运行状态
4. WARNING: 警告信息，潜在问题
5. ERROR: 错误信息，可恢复错误
6. FATAL: 致命错误，程序无法继续

#### 1.6.3 输出机制设计

1. **多输出目标**：
   - 文件输出
   - 控制台输出
   - 可以同时启用

2. **线程安全**：
   - 使用互斥锁保护输出操作
   - 确保日志顺序性

3. **性能优化**：
   - 缓冲输出
   - 避免频繁文件操作

### 1.7 Config 模块

#### 1.7.1 类结构设计

```cpp
class Config {
public:
    bool loadFromFile(const std::string& filePath);
    
    template<typename T>
    T get(const std::string& key, const T& defaultValue = T()) const;
    
    template<typename T>
    T getNestedValue(const std::string& path, const T& defaultValue = T()) const;
    
    template<typename T>
    void set(const std::string& key, const T& value);
    
    template<typename T>
    void setNestedValue(const std::string& path, const T& value);

private:
    const nlohmann::json* getNestedObject(const std::string& path) const;
    nlohmann::json* getNestedObjectForUpdate(const std::string& path);
    
    nlohmann::json config_;
};
```

#### 1.7.2 JSON 解析设计

1. **基于 nlohmann/json**：
   - 使用成熟的 JSON 解析库
   - 支持复杂数据结构

2. **配置项访问**：
   - 支持简单键值访问
   - 支持嵌套路径访问（如 "server.port"）

3. **类型安全**：
   - 模板方法支持类型转换
   - 默认值机制处理缺失配置

### 1.8 MemoryPool 模块

#### 1.8.1 类结构设计

```cpp
template<typename T, size_t BlockSize = 1024>
class MemoryPool {
public:
    MemoryPool();
    ~MemoryPool();
    
    T* allocate();
    void deallocate(T* ptr);

private:
    union Slot {
        T element;
        Slot* next;
    };

    struct Block {
        Slot slots[BlockSize];
        Block* next;
    };

    Block* currentBlock_;
    Slot* freeSlots_;
    std::vector<std::unique_ptr<Block>> blocks_;
    std::mutex mutex_;
};
```

#### 1.8.2 内存管理策略

1. **预分配机制**：
   - 一次性分配大块内存
   - 切分为固定大小的槽位

2. **快速分配回收**：
   - O(1) 时间复杂度的分配和回收
   - 使用链表管理空闲槽位

3. **线程安全**：
   - 使用互斥锁保护共享数据
   - 支持多线程并发访问

#### 1.8.3 内存布局设计

```
Block 结构:
┌─────────────────────────────────────────────────────────────┐
│ Slot[0] │ Slot[1] │ Slot[2] │ ... │ Slot[BlockSize-1] │ next │
└─────────────────────────────────────────────────────────────┘
                              ▲
                              │
                            链表指针

Slot 结构:
┌─────────────────────────────┬─────────────────────────────┐
│         T element           │        Slot* next           │
│         (数据区域)           │        (链表指针)           │
└─────────────────────────────┴─────────────────────────────┘
 (使用时作为对象存储)          (空闲时作为链表指针)
```

### 1.9 MultiLevelMemoryPool 模块

#### 1.9.1 类结构设计

```cpp
class MultiLevelMemoryPool {
public:
    static constexpr size_t MIN_BLOCK_SIZE = 8;
    static constexpr size_t MAX_BLOCK_SIZE = 4096;
    static constexpr size_t LEVEL_COUNT = 10;

    MultiLevelMemoryPool();
    ~MultiLevelMemoryPool();
    
    void* allocate(size_t size);
    void deallocate(void* ptr);

private:
    int findPoolLevel(size_t size) const;
    size_t getLevelBlockSize(size_t level) const;

    struct PoolLevel {
        size_t blockSize;
        std::vector<char*> blocks;
        std::vector<char*> freeSlots;
    };

    std::array<PoolLevel, LEVEL_COUNT> pools_;
    std::mutex mutex_;
};
```

#### 1.9.2 分级策略

1. **级别划分**：
   - 根据内存大小划分为不同级别
   - 每个级别处理特定范围的内存请求

2. **大小计算**：
   - 按指数增长计算各级别块大小
   - 覆盖从 8 字节到 4096 字节的范围

3. **分配策略**：
   - 根据请求大小选择合适的级别
   - 在对应级别的内存池中分配内存

### 1.10 TokenBucket 模块

#### 1.10.1 类结构设计

```cpp
class TokenBucket {
public:
    TokenBucket(size_t capacity, double tokensPerSecond);
    bool tryConsume(size_t tokens = 1);
    size_t getCurrentTokens() const;

private:
    void refill();

    mutable std::mutex mutex_;
    const size_t capacity_;
    const double tokensPerSecond_;
    size_t tokens_;
    std::chrono::steady_clock::time_point lastRefillTime_;
};
```

#### 1.10.2 令牌桶算法实现

1. **令牌生成**：
   - 按固定速率生成令牌
   - 令牌数量不超过桶容量

2. **令牌消费**：
   - 处理请求前需要获取令牌
   - 令牌不足时拒绝请求

3. **时间计算**：
   - 根据时间差计算应生成的令牌数
   - 实现精确的速率控制

## 2. HTTP 扩展模块详细设计

### 2.1 HttpRequest 模块

#### 2.1.1 类结构设计

```cpp
class HttpRequest {
public:
    HttpRequest();
    
    const std::string& getMethod() const;
    const std::string& getPath() const;
    const std::string& getVersion() const;
    const std::map<std::string, std::string>& getHeaders() const;
    const std::string& getBody() const;
    
    void setMethod(const std::string& method);
    void setPath(const std::string& path);
    void setVersion(const std::string& version);
    void addHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);

private:
    std::string method_;
    std::string path_;
    std::string version_;
    std::map<std::string, std::string> headers_;
    std::string body_;
};
```

#### 2.1.2 数据封装设计

1. **方法封装**：提供访问器和修改器方法
2. **头字段管理**：使用 map 存储键值对
3. **内存管理**：使用 std::string 自动管理内存

### 2.2 HttpResponse 模块

#### 2.2.1 类结构设计

```cpp
class HttpResponse {
public:
    HttpResponse(HttpStatus status = HttpStatus::OK);
    
    HttpStatus getStatus() const;
    const std::map<std::string, std::string>& getHeaders() const;
    const std::string& getBody() const;
    
    void setStatus(HttpStatus status);
    void addHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);
    void setContentType(const std::string& contentType);

private:
    HttpStatus status_;
    std::map<std::string, std::string> headers_;
    std::string body_;
};
```

#### 2.2.2 响应构建设计

1. **状态码管理**：使用枚举类型确保类型安全
2. **头字段扩展**：支持动态添加响应头
3. **内容类型设置**：提供便捷的内容类型设置方法

### 2.3 HttpProcessor 模块

#### 2.3.1 类结构设计

```cpp
class HttpProcessor {
public:
    HttpProcessor(std::shared_ptr<Router> router);
    
    void processRequest(const std::string& request, std::string& response);

private:
    std::shared_ptr<Router> router_;
};
```

#### 2.3.2 处理流程设计

1. 解析 HTTP 请求
2. 路由匹配和处理
3. 构建 HTTP 响应
4. 返回处理结果

### 2.4 CompressionPolicy 模块

#### 2.4.1 类结构设计

```cpp
class CompressionPolicy {
public:
    static bool shouldCompress(const std::string& contentType, size_t contentSize);
    static std::string selectCompressionAlgorithm(const std::string& acceptEncoding);
};
```

#### 2.4.2 压缩策略设计

1. **内容类型检查**：只压缩文本类型内容
2. **内容大小检查**：小内容不压缩
3. **算法选择**：根据客户端支持选择压缩算法

## 3. SSL 模块详细设计

### 3.1 SSLContext 模块

#### 3.1.1 类结构设计

```cpp
class SSLContext {
public:
    SSLContext();
    ~SSLContext();
    
    bool initialize(const std::string& certFile, const std::string& keyFile);
    SSL* createSSL(int socket);
    void freeSSL(SSL* ssl);

private:
    SSL_CTX* context_;
};
```

#### 3.2.2 SSL 初始化设计

1. **库初始化**：初始化 OpenSSL 库
2. **上下文创建**：创建 SSL 上下文
3. **证书加载**：加载服务器证书和私钥
4. **协议配置**：配置支持的 SSL/TLS 协议

### 3.2 SSLSocket 模块

#### 3.2.1 类结构设计

```cpp
class SSLSocket {
public:
    SSLSocket(SSL* ssl);
    
    int read(char* buffer, size_t length);
    int write(const char* buffer, size_t length);
    bool handshake();

private:
    SSL* ssl_;
};
```

#### 3.2.2 SSL 通信设计

1. **握手处理**：完成 SSL/TLS 握手过程
2. **数据读写**：封装 SSL 读写操作
3. **错误处理**：处理 SSL 相关错误

## 4. 工具模块详细设计

### 4.1 DateTimeUtils 模块

#### 4.1.1 类结构设计

```cpp
namespace DateTimeUtils {
    std::string getCurrentTimestamp();
    std::string formatTimePoint(const std::chrono::steady_clock::time_point& tp);
}
```

#### 4.1.2 时间处理设计

1. **时间戳生成**：生成标准格式时间戳
2. **时间点格式化**：格式化时间点为可读字符串

### 4.2 CompressionUtil 模块

#### 4.2.1 类结构设计

```cpp
namespace CompressionUtil {
    std::string compressGzip(const std::string& data);
    std::string decompressGzip(const std::string& compressedData);
    std::string compressDeflate(const std::string& data);
    std::string decompressDeflate(const std::string& compressedData);
}
```

#### 4.2.2 压缩算法设计

1. **gzip 压缩**：实现 gzip 压缩算法
2. **deflate 压缩**：实现 deflate 压缩算法
3. **错误处理**：处理压缩解压缩错误