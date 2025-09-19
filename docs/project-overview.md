# WebServer 项目概述

## 1. 项目简介

WebServer 是一个基于 C++14 标准开发的高性能 Web 服务器，专为处理 HTTP/1.1 请求而设计。该项目旨在提供一个轻量级、可扩展且高效的 Web 服务器解决方案，适用于对性能敏感的服务场景。

## 2. 核心特性

- **高性能**: 基于线程池的并发处理机制，支持高并发请求处理
- **模块化设计**: 组件之间松耦合，易于扩展和维护
- **配置灵活**: 支持 JSON 格式的配置文件，便于部署和调整
- **日志系统**: 内置多级别日志系统，支持彩色终端输出
- **内存优化**: 集成内存池管理，减少动态内存分配开销
- **安全机制**: 支持 HTTPS/SSL 加密通信和请求限流
- **连接管理**: 智能连接管理，支持 keep-alive 和连接超时控制

## 3. 技术架构

### 3.1 核心组件

WebServer 项目由以下核心组件构成：

1. **WebServer**: 主服务器类，协调各个组件工作
2. **ConnectionManager**: 连接管理器，负责处理客户端连接
3. **ThreadPool**: 线程池，用于并发处理请求
4. **Router**: 路由管理器，处理 URL 路由映射
5. **HttpParser**: HTTP 协议解析器，处理请求解析和响应构建
6. **Logger**: 日志系统，提供多级别日志记录
7. **Config**: 配置管理器，处理 JSON 配置文件
8. **MemoryPool/MultiLevelMemoryPool**: 内存池，优化内存分配
9. **TokenBucket**: 令牌桶算法实现，用于请求限流

### 3.2 工作流程

1. WebServer 启动并监听指定端口
2. ConnectionManager 接收客户端连接请求
3. ThreadPool 分配工作线程处理连接
4. HttpParser 解析 HTTP 请求
5. Router 根据 URL 路径匹配处理函数
6. 处理函数生成响应内容
7. HttpParser 构建 HTTP 响应并返回给客户端

## 4. 适用场景

- 高性能 Web 服务
- RESTful API 服务
- 微服务架构中的服务节点
- 静态文件服务器
- 反向代理服务器（扩展功能）

## 5. 技术栈

- **语言**: C++14
- **构建系统**: CMake
- **依赖管理**: JSON for Modern C++ (nlohmann/json)
- **测试框架**: Google Test
- **性能测试**: Google Benchmark
- **网络通信**: POSIX Socket API
- **加密支持**: OpenSSL

## 6. 目标用户

- C++ 开发者
- 系统架构师
- 性能优化工程师
- 对底层网络编程感兴趣的开发者