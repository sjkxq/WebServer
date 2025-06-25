# 使用指南

## 日志系统使用示例

```cpp
#include "Logger.hpp"

// 设置日志级别(默认为INFO)
Logger::setMinLevel(Logger::Level::DEBUG);

// 记录不同级别日志
LOG_ERROR("This is an error message");
LOG_WARNING("This is a warning message"); 
LOG_INFO("This is an info message");
LOG_DEBUG("This is a debug message");
```

日志格式示例：
```
2023-11-15 14:30:45.123 [ERROR] This is an error message
2023-11-15 14:30:45.124 [WARNING] This is a warning message
```

## 服务器运行方法

运行编译后的可执行文件：
```bash
./webserver
```

默认服务器将在本地8080端口启动。

## 日志级别说明

- ERROR: 错误信息
- WARNING: 警告信息
- INFO: 一般运行信息
- DEBUG: 调试信息(默认禁用)