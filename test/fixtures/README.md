# WebServer 测试夹具

本目录包含 WebServer 项目的测试夹具（Test Fixtures），用于简化测试编写和提高测试代码的可重用性。

## 概述

测试夹具是一种特殊的类，用于为测试提供必要的环境设置和清理功能。它们封装了测试所需的常见操作，使测试代码更加简洁和可维护。

## 可用的测试夹具

### 基础测试夹具

- `ServerTestFixture`：所有测试夹具的基类，提供通用功能如计时、临时目录创建等

### 组件特定测试夹具

- `ThreadPoolFixture`：用于线程池测试
- `MemoryPoolFixture`：用于内存池测试
- `MultiLevelMemoryPoolFixture`：用于多级内存池测试
- `LoggerFixture`：用于日志系统测试
- `NetworkFixture`：用于网络通信测试
- `HttpFixture`：用于HTTP请求处理测试

### 参数化测试夹具

- `ThreadPoolParameterizedFixture`：参数化线程池测试
- `MultiLevelMemoryPoolParameterizedFixture`：参数化多级内存池测试
- `LoggerParameterizedFixture`：参数化日志测试
- `HttpParameterizedFixture`：参数化HTTP测试

### 测试夹具工厂

- `ServerTestFixtureFactory`：用于创建和配置各种测试夹具的工厂类

## 使用示例

### 基本用法

```cpp
#include "fixtures/thread_pool_fixtures.h"

using namespace WebServer::test;

// 使用线程池测试夹具
TEST_F(ThreadPoolFixture, SubmitTaskAndVerifyResult) {
    auto future = thread_pool_->submit([]() {
        return 42;
    });
    
    EXPECT_EQ(future.get(), 42);
}
```

### 参数化测试

```cpp
#include "fixtures/logger_fixtures.h"

using namespace WebServer::test;

// 参数化日志测试
TEST_P(LoggerParameterizedFixture, LogLevelFiltering) {
    LogLevel current_level = GetLogLevel();
    
    logger_->debug("Debug message");
    logger_->info("Info message");
    logger_->flush();
    
    bool should_contain_debug = current_level <= LogLevel::DEBUG;
    EXPECT_EQ(LogContains("Debug message"), should_contain_debug);
}

// 实例化参数化测试
INSTANTIATE_TEST_SUITE_P(
    LogLevels,
    LoggerParameterizedFixture,
    ::testing::ValuesIn(LOG_LEVELS)
);
```

### 使用测试夹具工厂

```cpp
#include "fixtures/fixture_factory.h"

using namespace WebServer::test;

TEST(FixtureFactoryTest, CreateAndUseFixture) {
    // 创建带有特定配置的日志测试夹具
    FixtureOptions options;
    options.log_level = LogLevel::DEBUG;
    
    auto logger_fixture = ServerTestFixtureFactory::CreateLoggerFixture(options);
    logger_fixture->SetUp();
    
    logger_fixture->logger_->info("Test message");
    EXPECT_TRUE(logger_fixture->LogContains("Test message"));
    
    logger_fixture->TearDown();
}
```

## 最佳实践

1. **选择合适的夹具**：根据测试需求选择最合适的测试夹具
2. **重用而非重写**：尽量重用现有夹具，而不是重写相似功能
3. **参数化测试**：对于需要在不同条件下重复的测试，使用参数化测试夹具
4. **组合夹具**：对于复杂测试场景，可以使用 `ServerTestFixtureFactory::CreateCompositeFixture` 组合多个夹具

## 扩展测试夹具

如果需要创建新的测试夹具，请遵循以下步骤：

1. 从 `ServerTestFixture` 或其他适当的基类继承
2. 实现 `SetUp` 和 `TearDown` 方法
3. 添加测试所需的辅助方法
4. 在 `fixture_factory.h` 中添加相应的工厂方法

## 注意事项

- 所有测试夹具都会自动清理它们创建的资源
- 参数化测试夹具需要使用 `INSTANTIATE_TEST_SUITE_P` 宏进行实例化
- 测试夹具工厂创建的夹具需要手动调用 `SetUp` 和 `TearDown` 方法