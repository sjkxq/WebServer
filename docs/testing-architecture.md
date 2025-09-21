# 测试架构说明

## 概述

本文档描述了 WebServer 项目的测试架构和组织方式。为了提高测试的可维护性和可扩展性，我们将测试按照功能模块进行了重新组织。

## 测试结构

```
test/
├── fixtures/                 # 测试夹具
├── modules/                  # 按模块组织的测试
│   ├── core/                 # 核心功能测试
│   ├── http/                 # HTTP相关测试
│   ├── memory/               # 内存管理测试
│   ├── thread/               # 线程相关测试
│   ├── logger/               # 日志系统测试
│   ├── utils/                # 工具类测试
│   └── ssl/                  # SSL功能测试
├── ColorOutputTest.cpp       # 颜色输出测试
└── main.cpp                  # 测试入口
```

## 模块划分说明

### 核心模块 (core)
包含 WebServer 核心功能的测试，如：
- WebServer 主类测试
- 配置系统测试

### HTTP模块 (http)
包含所有与 HTTP 协议处理相关的测试，如：
- HTTP 请求解析测试
- HTTP 状态码测试

### 内存模块 (memory)
包含内存管理组件的测试，如：
- 内存池测试
- 多级内存池测试

### 线程模块 (thread)
包含线程和并发相关的测试，如：
- 线程池测试

### 日志模块 (logger)
包含日志系统的测试，如：
- 日志功能测试

### 工具模块 (utils)
包含各种工具类的测试。

### SSL模块 (ssl)
包含 SSL/TLS 相关功能的测试。

## 测试运行方式

每个模块都有独立的测试可执行文件，可以通过以下方式运行：

```bash
# 运行所有测试
ctest

# 运行特定模块测试
ctest -R run_core_tests
ctest -R run_http_tests
ctest -R run_memory_tests
ctest -R run_thread_tests
ctest -R run_logger_tests
```

## 扩展测试模块

要添加新的测试模块，请执行以下步骤：

1. 在 `test/modules/` 目录下创建新模块目录
2. 在新目录中创建 `CMakeLists.txt` 文件定义测试
3. 在 `test/CMakeLists.txt` 中添加 `add_subdirectory` 调用
4. 编写相应的测试代码

## 测试夹具

项目提供了一套丰富的测试夹具来简化测试编写，详见 `test/fixtures/` 目录及其中的 README.md 文件。