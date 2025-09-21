# 日志模块测试

# 日志模块测试源文件
set(LOGGER_TEST_SOURCES
    ../../Logger_test.cpp
)

# 创建日志模块测试可执行文件
add_executable(logger_tests ${LOGGER_TEST_SOURCES})

target_link_libraries(logger_tests PRIVATE 
    webserver_lib
    gtest_main 
    pthread
    ColorOutput
)

target_compile_definitions(logger_tests PRIVATE TESTING=1)

target_include_directories(logger_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/modules/ColorOutput/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../fixtures
)

# 添加到CTest
add_test(NAME run_logger_tests COMMAND logger_tests)