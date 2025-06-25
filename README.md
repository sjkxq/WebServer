# C++ WebServer

一个基于C++的高性能Web服务器实现，支持多线程处理和彩色日志输出。

## 功能特性

- 基于C++14标准开发
- 使用现代CMake构建系统
- 支持HTTP/1.1协议
- 多线程处理请求，基于线程池实现
- 可配置的日志系统，支持多种级别(ERROR/WARNING/INFO/DEBUG)
- 彩色终端输出支持，增强日志可读性
- 内置性能基准测试
- 全面的单元测试覆盖

## 快速开始

```bash
# 使用构建脚本（推荐）
./build.sh

# 手动构建
mkdir build && cd build
cmake .. && make
```

## 文档

详细文档请参考：

- [安装指南](docs/installation.md) - 构建方法和依赖项
- [使用指南](docs/usage.md) - 日志系统和服务器使用方法
- [性能测试](docs/benchmark.md) - 性能基准测试指南
- [单元测试](docs/testing.md) - 测试框架和覆盖率
- [彩色输出](docs/color_output.md) - 终端彩色输出功能

## 贡献

欢迎通过Issue或Pull Request贡献代码。