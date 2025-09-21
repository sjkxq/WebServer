# 工具模块测试

# 工具模块测试源文件
set(UTILS_TEST_SOURCES
    ColorOutputTest.cpp
)

# 创建工具模块测试可执行文件
add_executable(utils_tests ${UTILS_TEST_SOURCES})

target_link_libraries(utils_tests PRIVATE 
    webserver_lib
    gtest_main 
    pthread
    ColorOutput
)

target_compile_definitions(utils_tests PRIVATE TESTING=1)

target_include_directories(utils_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/modules/ColorOutput/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../fixtures
)

# 添加到CTest
add_test(NAME run_utils_tests COMMAND utils_tests)