# Benchmark测试配置

# 添加基准测试可执行文件
add_executable(webserver_benchmark
    thread_pool_benchmark.cpp
    logger_benchmark.cpp
)

# 链接主项目和benchmark库
target_link_libraries(webserver_benchmark
    PRIVATE 
    webserver_lib
    benchmark::benchmark
    benchmark::benchmark_main
)

# 添加头文件搜索路径
target_include_directories(webserver_benchmark
    PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/modules/ColorOutput/include
)

# 在Linux上链接pthread
if(UNIX)
    target_link_libraries(webserver_benchmark PRIVATE pthread)
endif()