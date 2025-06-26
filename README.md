# C++ WebServer

一个基于C++的高性能Web服务器实现，支持多线程处理和彩色日志输出。

## 项目背景

开发一个基于C++的高性能Web服务器，支持多线程处理和彩色日志输出。目标用户包括C++开发者、系统架构师和性能优化工程师。

## 核心问题

- 提供高效的HTTP请求处理机制
- 支持可配置的日志系统以增强调试和监控能力
- 提供清晰的终端输出以便于快速识别日志级别

## 功能特性

- 基于C++14标准开发
- 使用现代CMake构建系统
- 支持HTTP/1.1协议
- 多线程处理请求，基于线程池实现
- 可配置的日志系统，支持多种级别(ERROR/WARNING/INFO/DEBUG)
- 彩色终端输出支持，增强日志可读性
- 内置性能基准测试
- 全面的单元测试覆盖
- 内存池管理提升性能
- URL路由功能
- JSON配置文件解析(nlohmann/json)

## 技术架构

### 设计模式
- 线程池模式：用于任务调度和并发控制
- 观察者模式：用于日志系统的事件通知
- 单例模式：用于全局配置和日志实例管理
- 工厂模式：用于创建不同类型的内存池

### 主要组件
- WebServer：负责启动服务并监听端口
- ConnectionManager：处理客户端连接
- ThreadPool：管理工作线程执行任务
- Logger：记录运行时日志
- Router：处理URL路由
- Config：加载配置文件
- MemoryPool：提供内存分配优化

## 构建说明

更多信息请参考[构建说明](BUILD.md)

## 项目文档

## 目录
- [快速开始](#快速开始)
- [构建说明](BUILD.md)
- [部署指南](DEPLOY.md)
- [开发指南](DEVELOPMENT.md)

## 快速开始

```bash
# 使用构建脚本（推荐）
./build_and_test.sh

# 手动构建 (使用Ninja - 推荐)
mkdir build && cd build
cmake -G Ninja .. && ninja

# 手动构建 (使用Make - 备选)
mkdir build && cd build
cmake .. && make
```

## 容器化部署
```bash
# 构建Docker镜像
docker build -t webserver .

# 运行容器
docker run -d -p 8080:8080 --name webserver webserver
```

> 更多部署选项请参考[部署指南](DEPLOY.md)

## 文档

详细文档请参考：

- [安装指南](docs/installation.md) - 构建方法和依赖项
- [使用指南](docs/usage.md) - 日志系统和服务器使用方法
- [性能测试](docs/benchmark.md) - 性能基准测试指南
- [单元测试](docs/testing.md) - 测试框架和覆盖率
- [彩色输出](docs/color_output.md) - 终端彩色输出功能

## 贡献

欢迎通过Issue或Pull Request贡献代码。