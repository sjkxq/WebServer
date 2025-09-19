# API 设计文档

## 1. 核心类 API

### 1.1 WebServer 类

WebServer 是整个 Web 服务器的核心类，负责协调各个组件。

#### 构造函数
```cpp
WebServer(const Config& config);
```
- **功能**：创建 WebServer 实例
- **参数**：[config](../include/Config.hpp#L22-L22) - 服务器配置对象
- **异常**：可能抛出配置相关的异常

#### 析构函数
```cpp
~WebServer();
```
- **功能**：销毁 WebServer 实例，释放资源

#### start 方法
```cpp
bool start();
```
- **功能**：启动 Web 服务器
- **返回值**：启动成功返回 true，否则返回 false

#### stop 方法
```cpp
void stop();
```
- **功能**：停止 Web 服务器

#### addRoute 方法
```cpp
void addRoute(const std::string& path, Router::RequestHandler handler);
```
- **功能**：添加路由处理函数
- **参数**：
  - [path](../test/fixtures/network_fixtures.h#L42-L42) - URL 路径
  - [handler](../include/Router.hpp#L31-L31) - 处理该路径的函数

### 1.2 Config 类

Config 类负责处理服务器配置。

#### loadFromFile 方法
```cpp
bool loadFromFile(const std::string& filePath);
```
- **功能**：从文件加载配置
- **参数**：[filePath](../include/Config.hpp#L24-L24) - 配置文件路径
- **返回值**：加载成功返回 true，否则返回 false

#### get 方法
```cpp
template<typename T>
T get(const std::string& key, const T& defaultValue = T()) const;
```
- **功能**：获取配置值
- **参数**：
  - [key](../include/Config.hpp#L32-L32) - 配置键
  - [defaultValue](../include/Config.hpp#L33-L33) - 默认值
- **返回值**：配置值，如果不存在则返回默认值

#### getNestedValue 方法
```cpp
template<typename T>
T getNestedValue(const std::string& path, const T& defaultValue = T()) const;
```
- **功能**：获取嵌套配置值（如 "server.port"）
- **参数**：
  - [path](../test/fixtures/network_fixtures.h#L42-L42) - 嵌套路径（使用点号分隔）
  - [defaultValue](../include/Config.hpp#L33-L33) - 默认值
- **返回值**：配置值，如果不存在则返回默认值

### 1.3 Logger 类

Logger 类提供线程安全的日志记录功能。

#### getInstance 方法
```cpp
static Logger& getInstance();
```
- **功能**：获取 Logger 单例实例
- **返回值**：Logger 单例的引用

#### setLogFile 方法
```cpp
void setLogFile(const std::string& filename);
```
- **功能**：设置日志输出文件
- **参数**：[filename](../include/Logger.hpp#L42-L42) - 日志文件路径

#### setStream 方法
```cpp
void setStream(std::ostream& stream);
```
- **功能**：设置日志输出流
- **参数**：[stream](../include/Logger.hpp#L47-L47) - 输出流引用

#### log 方法
```cpp
void log(Level level, const std::string& message);
```
- **功能**：记录日志
- **参数**：
  - [level](../include/Logger.hpp#L53-L53) - 日志级别
  - [message](../include/Logger.hpp#L54-L54) - 日志消息

#### setLogLevel 方法
```cpp
void setLogLevel(Level level);
```
- **功能**：设置最低日志级别
- **参数**：[level](../include/Logger.hpp#L53-L53) - 日志级别

### 1.4 Router 类

Router 类处理 HTTP 请求路由。

#### addRoute 方法
```cpp
void addRoute(const std::string& path, RequestHandler handler);
```
- **功能**：添加路由处理函数
- **参数**：
  - [path](../test/fixtures/network_fixtures.h#L42-L42) - URL 路径
  - [handler](../include/Router.hpp#L31-L31) - 处理该路径的函数

#### handleRequest 方法
```cpp
std::pair<bool, std::string> handleRequest(const std::string& path, 
    const std::map<std::string, std::string>& headers, 
    const std::string& body) const;
```
- **功能**：处理 HTTP 请求
- **参数**：
  - [path](../test/fixtures/network_fixtures.h#L42-L42) - 请求的 URL 路径
  - [headers](../include/HttpParser.hpp#L33-L33) - HTTP 请求头
  - [body](../include/HttpParser.hpp#L33-L33) - HTTP 请求体
- **返回值**：包含处理结果的 pair，first 表示是否找到路由，second 为响应内容

### 1.5 ThreadPool 类

ThreadPool 类管理多个工作线程并分配异步任务。

#### 构造函数
```cpp
explicit ThreadPool(size_t threads);
```
- **功能**：初始化指定数量的工作线程
- **参数**：[threads](../include/ThreadPool.hpp#L24-L24) - 要创建的工作线程数量

#### enqueue 方法
```cpp
template<class F, class... Args>
auto enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>
```
- **功能**：提交任务到线程池的队列中
- **参数**：
  - [f](../include/ThreadPool.hpp#L34-L34) - 要执行的任务函数或可调用对象
  - [args](../include/ThreadPool.hpp#L35-L35) - 传递给 f 的参数列表
- **返回值**：表示异步结果的 std::future 对象

### 1.6 MemoryPool 类

MemoryPool 类为固定大小内存块提供线程安全的内存池。

#### allocate 方法
```cpp
T* allocate();
```
- **功能**：为对象分配内存
- **返回值**：指向已分配内存的指针

#### deallocate 方法
```cpp
void deallocate(T* ptr);
```
- **功能**：释放内存
- **参数**：[ptr](../include/MemoryPool.hpp#L37-L37) - 指向要释放的内存的指针

### 1.7 MultiLevelMemoryPool 类

MultiLevelMemoryPool 类支持不同大小内存分配的线程安全多级内存池。

#### allocate 方法
```cpp
void* allocate(size_t size);
```
- **功能**：分配指定大小的内存
- **参数**：[size](../include/MultiLevelMemoryPool.hpp#L46-L46) - 要分配的内存大小（字节）
- **返回值**：指向已分配内存的指针，如果大小过大则返回 nullptr

#### deallocate 方法
```cpp
void deallocate(void* ptr);
```
- **功能**：释放内存
- **参数**：[ptr](../include/MultiLevelMemoryPool.hpp#L53-L53) - 指向要释放的内存的指针

### 1.8 ConnectionManager 类

ConnectionManager 类管理服务器的客户端连接。

#### addConnection 方法
```cpp
void addConnection(int socket, const std::string& clientIP, ConnectionHandler handler);
```
- **功能**：添加新的连接
- **参数**：
  - [socket](../include/ConnectionManager.hpp#L53-L53) - 客户端套接字描述符
  - [clientIP](../include/ConnectionManager.hpp#L54-L54) - 客户端 IP 地址
  - [handler](../include/Router.hpp#L31-L31) - 处理连接的函数

#### closeConnection 方法
```cpp
void closeConnection(int socket);
```
- **功能**：关闭指定连接
- **参数**：[socket](../include/ConnectionManager.hpp#L53-L53) - 要关闭的套接字描述符

### 1.9 HttpParser 类

HttpParser 类处理 HTTP 请求和响应的解析与构建。

#### parseRequest 方法
```cpp
static std::tuple<std::string, std::string, std::map<std::string, std::string>, std::string> parseRequest(const std::string& request);
```
- **功能**：解析 HTTP 请求
- **参数**：[request](../include/HttpParser.hpp#L33-L33) - HTTP 请求字符串
- **返回值**：包含路径、头部和正文的元组

#### buildResponse 方法
```cpp
static std::string buildResponse(HttpStatus statusCode, const std::string& content, const std::string& contentType = "text/html");
```
- **功能**：构建 HTTP 响应
- **参数**：
  - [statusCode](../include/HttpParser.hpp#L39-L39) - HTTP 状态码
  - [content](../include/HttpParser.hpp#L40-L40) - 响应内容
  - [contentType](../include/HttpParser.hpp#L41-L41) - 内容类型，默认为 "text/html"
- **返回值**：完整的 HTTP 响应字符串

## 2. HTTP 相关类 API

### 2.1 HttpRequest 类

HttpRequest 类封装 HTTP 请求信息。

#### 构造函数
```cpp
HttpRequest();
```
- **功能**：创建 HttpRequest 实例

#### 方法
```cpp
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
```

### 2.2 HttpResponse 类

HttpResponse 类封装 HTTP 响应信息。

#### 构造函数
```cpp
HttpResponse(HttpStatus status = HttpStatus::OK);
```
- **功能**：创建 HttpResponse 实例
- **参数**：[status](../include/http/HttpResponse.hpp#L31-L31) - HTTP 状态码，默认为 OK

#### 方法
```cpp
HttpStatus getStatus() const;
const std::map<std::string, std::string>& getHeaders() const;
const std::string& getBody() const;
void setStatus(HttpStatus status);
void addHeader(const std::string& key, const std::string& value);
void setBody(const std::string& body);
void setContentType(const std::string& contentType);
```

## 3. 工具类 API

### 3.1 TokenBucket 类

TokenBucket 类实现令牌桶算法，用于请求限流。

#### 构造函数
```cpp
TokenBucket(size_t capacity, double tokensPerSecond);
```
- **功能**：创建 TokenBucket 实例
- **参数**：
  - [capacity](../include/TokenBucket.hpp#L17-L17) - 桶的最大容量
  - [tokensPerSecond](../include/TokenBucket.hpp#L18-L18) - 每秒添加的令牌数

#### tryConsume 方法
```cpp
bool tryConsume(size_t tokens = 1);
```
- **功能**：尝试消费指定数量的令牌
- **参数**：[tokens](../include/TokenBucket.hpp#L24-L24) - 请求的令牌数量
- **返回值**：是否成功获取令牌

## 4. 日志宏 API

项目提供了一组日志宏，方便记录不同级别的日志：

```cpp
#define LOG_TRACE(msg)    // 跟踪信息
#define LOG_DEBUG(msg)    // 调试信息
#define LOG_INFO(msg)     // 一般信息
#define LOG_WARNING(msg)  // 警告信息
#define LOG_ERROR(msg)    // 错误信息
#define LOG_FATAL(msg)    // 致命错误
```

## 5. 使用示例

### 5.1 创建和启动服务器
```cpp
webserver::Config config;
config.loadFromFile("config.json");

webserver::WebServer server(config);
server.addRoute("/hello", [](const std::map<std::string, std::string>& headers, const std::string& body) {
    return "<html><body><h1>Hello, World!</h1></body></html>";
});

server.start();
```

### 5.2 使用线程池
```cpp
ThreadPool pool(4);
auto result = pool.enqueue([](int answer) { return answer; }, 42);
std::cout << result.get() << std::endl;
```

### 5.3 使用内存池
```cpp
WebServer::MemoryPool<int, 1024> pool;
int* ptr = pool.allocate();
// 使用 ptr
pool.deallocate(ptr);
```