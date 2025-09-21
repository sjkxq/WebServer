# 线程模块测试

# 线程模块测试源文件
set(THREAD_TEST_SOURCES
    ../../ThreadPool_test.cpp
)

# 创建线程模块测试可执行文件
add_executable(thread_tests ${THREAD_TEST_SOURCES})

target_link_libraries(thread_tests PRIVATE 
    webserver_lib
    gtest_main 
    pthread
)

target_compile_definitions(thread_tests PRIVATE TESTING=1)

target_include_directories(thread_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../fixtures
)

# 添加到CTest
add_test(NAME run_thread_tests COMMAND thread_tests)