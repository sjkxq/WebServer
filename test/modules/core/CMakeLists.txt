# 核心模块测试

# 核心模块测试源文件
set(CORE_TEST_SOURCES
    WebServer_test.cpp
    Config_test.cpp
)

# 创建核心模块测试可执行文件
add_executable(core_tests ${CORE_TEST_SOURCES})

target_link_libraries(core_tests PRIVATE 
    webserver_lib
    gtest_main 
    pthread
)

target_compile_definitions(core_tests PRIVATE TESTING=1)

target_include_directories(core_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../fixtures
)

# 添加到CTest
add_test(NAME run_core_tests COMMAND core_tests)