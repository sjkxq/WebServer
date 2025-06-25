# 测试配置

# 测试源文件列表
set(TEST_SOURCES
    main.cpp
    WebServer_test.cpp
    ThreadPool_test.cpp
    Logger_test.cpp
    Config_test.cpp
    MemoryPool_test.cpp
    MultiLevelMemoryPool_test.cpp
    HttpStatus_test.cpp
)

# 主要测试可执行文件
add_executable(webserver_tests ${TEST_SOURCES})

target_link_libraries(webserver_tests PRIVATE 
    webserver_lib
    gtest_main 
    pthread
    color_output
)

target_include_directories(webserver_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/fixtures
)

# 添加到CTest
add_test(NAME run_webserver_tests COMMAND webserver_tests)

# ColorOutput测试
add_executable(color_output_test
    ColorOutputTest.cpp
)

target_link_libraries(color_output_test PRIVATE 
    color_output
    gtest_main
)

target_include_directories(color_output_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
)

# 添加到CTest
add_test(NAME run_color_output_test COMMAND color_output_test)

# 设置所有测试的工作目录
set_tests_properties(run_webserver_tests run_color_output_test
    PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# 添加测试覆盖率支持（如果启用）
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_COMPILER_IS_GNUCXX)
        target_compile_options(webserver_tests PRIVATE --coverage)
        target_link_options(webserver_tests PRIVATE --coverage)
    endif()
endif()