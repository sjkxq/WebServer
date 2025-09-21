# 测试配置

# 测试源文件列表
set(TEST_SOURCES
    main.cpp
)

# 主要测试可执行文件
add_executable(webserver_tests ${TEST_SOURCES})

target_link_libraries(webserver_tests PRIVATE 
    webserver_lib
    gtest_main 
    pthread
    ColorOutput
)

target_compile_definitions(webserver_tests PRIVATE TESTING=1)

target_include_directories(webserver_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/modules/ColorOutput/include
    ${CMAKE_CURRENT_SOURCE_DIR}/fixtures
)

# 添加到CTest
add_test(NAME run_webserver_tests COMMAND webserver_tests)

# 添加模块化测试
add_subdirectory(modules/core)
add_subdirectory(modules/http)
add_subdirectory(modules/memory)
add_subdirectory(modules/thread)
add_subdirectory(modules/logger)
add_subdirectory(modules/utils)
add_subdirectory(modules/ssl)

# 设置所有测试的工作目录
set_tests_properties(run_webserver_tests
    PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# 添加测试覆盖率支持（如果启用）
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_COMPILER_IS_GNUCXX)
        target_compile_options(webserver_tests PRIVATE --coverage)
        target_link_options(webserver_tests PRIVATE --coverage)
    endif()
endif()