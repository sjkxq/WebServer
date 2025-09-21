# HTTP模块测试

# HTTP模块测试源文件
set(HTTP_TEST_SOURCES
    HttpParser_test.cpp
    HttpStatus_test.cpp
)

# 创建HTTP模块测试可执行文件
add_executable(http_tests ${HTTP_TEST_SOURCES})

target_link_libraries(http_tests PRIVATE 
    webserver_lib
    gtest_main 
    pthread
)

target_compile_definitions(http_tests PRIVATE TESTING=1)

target_include_directories(http_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../fixtures
)

# 添加到CTest
add_test(NAME run_http_tests COMMAND http_tests)