# 内存模块测试

# 内存模块测试源文件
set(MEMORY_TEST_SOURCES
    MemoryPool_test.cpp
    MultiLevelMemoryPool_test.cpp
    test_multi_level_memory_pool.cpp
)

# 创建内存模块测试可执行文件
add_executable(memory_tests ${MEMORY_TEST_SOURCES})

target_link_libraries(memory_tests PRIVATE 
    webserver_lib
    gtest_main 
    pthread
)

target_compile_definitions(memory_tests PRIVATE TESTING=1)

target_include_directories(memory_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../fixtures
)

# 添加到CTest
add_test(NAME run_memory_tests COMMAND memory_tests)