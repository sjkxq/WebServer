# 测试开发指南

## 概述

本文档旨在指导开发者如何为 WebServer 项目编写和组织测试代码。项目采用模块化测试结构，便于扩展和维护。

## 测试框架

项目使用 Google Test 作为测试框架，提供了丰富的断言和测试功能。

## 测试组织结构

测试按照功能模块进行组织，每个模块有独立的测试可执行文件。

### 目录结构

```
test/
├── fixtures/                      # 测试夹具
├── modules/                       # 按模块组织的测试
│   ├── core/                      # 核心功能测试
│   │   ├── WebServer_test.cpp     # WebServer主类测试
│   │   ├── Config_test.cpp        # 配置系统测试
│   │   └── CMakeLists.txt         # 构建配置
│   ├── http/                      # HTTP相关测试
│   │   ├── HttpParser_test.cpp    # HTTP解析器测试
│   │   ├── HttpStatus_test.cpp    # HTTP状态码测试
│   │   └── CMakeLists.txt         # 构建配置
│   ├── memory/                    # 内存管理测试
│   │   ├── MemoryPool_test.cpp              # 内存池测试
│   │   ├── MultiLevelMemoryPool_test.cpp    # 多级内存池测试
│   │   ├── test_multi_level_memory_pool.cpp # 多级内存池测试
│   │   └── CMakeLists.txt                   # 构建配置
│   ├── thread/                    # 线程相关测试
│   │   ├── ThreadPool_test.cpp    # 线程池测试
│   │   └── CMakeLists.txt         # 构建配置
│   ├── logger/                    # 日志系统测试
│   │   ├── Logger_test.cpp        # 日志功能测试
│   │   └── CMakeLists.txt         # 构建配置
│   ├── utils/                     # 工具类测试
│   │   ├── ColorOutputTest.cpp    # 颜色输出测试
│   │   └── CMakeLists.txt         # 构建配置
│   └── ssl/                       # SSL功能测试
│       └── CMakeLists.txt         # 构建配置（预留）
├── main.cpp                       # 测试入口
```

## 编写测试

### 使用测试夹具

项目提供了丰富的测试夹具来简化测试编写。测试夹具封装了测试所需的常见操作，使测试代码更加简洁和可维护。

例如，使用线程池测试夹具：

```cpp
#include "fixtures/thread_pool_fixtures.h"

using namespace WebServer::test;

TEST_F(ThreadPoolFixture, SubmitTaskAndVerifyResult) {
    auto future = thread_pool_->submit([]() {
        return 42;
    });
    
    EXPECT_EQ(future.get(), 42);
}
```

### 参数化测试

对于需要在不同条件下重复的测试，使用参数化测试夹具：

```cpp
#include "fixtures/logger_fixtures.h"

using namespace WebServer::test;

TEST_P(LoggerParameterizedFixture, LogLevelFiltering) {
    LogLevel current_level = GetLogLevel();
    
    logger_->debug("Debug message");
    logger_->info("Info message");
    logger_->flush();
    
    bool should_contain_debug = current_level <= LogLevel::DEBUG;
    EXPECT_EQ(LogContains("Debug message"), should_contain_debug);
}

INSTANTIATE_TEST_SUITE_P(
    LogLevels,
    LoggerParameterizedFixture,
    ::testing::ValuesIn(LOG_LEVELS)
);
```

## 添加新的测试模块

要添加新的测试模块，请执行以下步骤：

1. 在 `test/modules/` 目录下创建新模块目录
2. 在新目录中创建 `CMakeLists.txt` 文件定义测试
   
   示例 `CMakeLists.txt`：
   ```cmake
   # 新模块测试
   
   # 新模块测试源文件
   set(NEW_MODULE_TEST_SOURCES
       NewModule_test.cpp
   )
   
   # 创建新模块测试可执行文件
   add_executable(new_module_tests ${NEW_MODULE_TEST_SOURCES})
   
   target_link_libraries(new_module_tests PRIVATE 
       webserver_lib
       gtest_main 
       pthread
   )
   
   target_compile_definitions(new_module_tests PRIVATE TESTING=1)
   
   target_include_directories(new_module_tests PRIVATE 
       ${CMAKE_SOURCE_DIR}/src 
       ${CMAKE_SOURCE_DIR}/include
       ${CMAKE_CURRENT_SOURCE_DIR}/../../fixtures
   )
   
   # 添加到CTest
   add_test(NAME run_new_module_tests COMMAND new_module_tests)
   ```

3. 在 `test/CMakeLists.txt` 中添加 `add_subdirectory` 调用
4. 编写相应的测试代码

## 运行测试

### 运行所有测试

```bash
ctest
```

### 运行特定模块测试

```bash
# 运行核心模块测试
ctest -R run_core_tests

# 运行HTTP模块测试
ctest -R run_http_tests

# 运行内存模块测试
ctest -R run_memory_tests

# 运行线程模块测试
ctest -R run_thread_tests

# 运行日志模块测试
ctest -R run_logger_tests

# 运行工具模块测试
ctest -R run_utils_tests
```

### 调试测试

可以直接运行生成的测试可执行文件进行调试：

```bash
# 运行核心测试
./test/modules/core/core_tests

# 运行HTTP测试
./test/modules/http/http_tests

# 运行工具模块测试
./test/modules/utils/utils_tests

# 使用GDB调试
gdb ./test/modules/utils/utils_tests
```

## 最佳实践

1. **选择合适的夹具**：根据测试需求选择最合适的测试夹具
2. **重用而非重写**：尽量重用现有夹具，而不是重写相似功能
3. **参数化测试**：对于需要在不同条件下重复的测试，使用参数化测试夹具
4. **测试命名**：使用清晰、描述性的测试名称
5. **单一职责**：每个测试应该只验证一个功能点
6. **测试独立性**：确保测试之间没有依赖关系
7. **覆盖率**：尽量提高测试覆盖率，但不要为了覆盖率而编写无意义的测试