cmake_minimum_required(VERSION 3.14)
project(WebServer VERSION 1.0)

# 引入Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.11.0
)

# 引入Google Benchmark并禁用测试
include(FetchContent)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.7.1
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark tests")
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable Google Test")

# 引入nlohmann_json
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)
set(JSON_BuildTests OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(googletest benchmark nlohmann_json)

# 创建测试和benchmark目录
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/benchmark)

# 创建color_output库（仅头文件）
add_library(color_output INTERFACE)
target_include_directories(color_output INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 测试可执行文件配置
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_executable(webserver_tests
        test/main.cpp
        test/WebServer_test.cpp
        test/ThreadPool_test.cpp
        test/Logger_test.cpp
        test/Config_test.cpp
        test/MemoryPool_test.cpp
        test/MultiLevelMemoryPool_test.cpp
        test/HttpStatus_test.cpp
        src/WebServer.cpp
        src/Logger.cpp
        src/Config.cpp
        src/ThreadPool.cpp
        src/MemoryPool.cpp
        src/MultiLevelMemoryPool.cpp
        src/ConnectionManager.cpp
        src/HttpStatus.cpp
        src/Router.cpp
    )
    target_link_libraries(webserver_tests PRIVATE 
        gtest_main 
        nlohmann_json::nlohmann_json 
        pthread
        color_output
    )
    target_include_directories(webserver_tests PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    add_test(NAME run_webserver_tests COMMAND webserver_tests)

    # Add color output test executable
    add_executable(color_output_test
        test/ColorOutputTest.cpp
    )
    target_link_libraries(color_output_test PRIVATE color_output)
    target_include_directories(color_output_test PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    add_test(NAME run_color_output_test COMMAND color_output_test)
endif()
add_test(NAME webserver_tests COMMAND webserver_tests)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找nlohmann/json库
# nlohmann/json库已通过FetchContent引入

# 添加可执行文件
add_executable(webserver
    src/main.cpp
    src/WebServer.cpp
    src/Logger.cpp
    src/Config.cpp
    src/ThreadPool.cpp
    src/ConnectionManager.cpp
    src/HttpStatus.cpp
    src/Router.cpp
)

# 链接库
target_link_libraries(webserver PRIVATE 
    nlohmann_json::nlohmann_json
    color_output
)

# 添加头文件搜索路径
target_include_directories(webserver PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 在Debug模式下添加调试信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(webserver PRIVATE -g -O0)
endif()

# 添加必要的系统库
if(UNIX)
    target_link_libraries(webserver PRIVATE pthread)
endif()