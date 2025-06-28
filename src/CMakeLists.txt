# 源文件列表
set(WEBSERVER_SOURCES
    main.cpp
    WebServer.cpp
    Logger.cpp
    Config.cpp
    ConnectionManager.cpp
    http/HttpRequest.cpp
    http/HttpResponse.cpp
    HttpParser.cpp
    HttpStatus.cpp
    MemoryPool.cpp
    MultiLevelMemoryPool.cpp
    Router.cpp
    ThreadPool.cpp
    utils/DateTimeUtils.cpp
)

# 创建主要的可执行文件
add_executable(webserver ${WEBSERVER_SOURCES})

# 创建源代码库（供测试和基准测试使用）
find_package(OpenSSL REQUIRED)

add_library(webserver_lib STATIC
    WebServer.cpp
    Logger.cpp
    Config.cpp
    ConnectionManager.cpp
    http/HttpRequest.cpp
    http/HttpResponse.cpp
    HttpParser.cpp
    HttpStatus.cpp
    MemoryPool.cpp
    MultiLevelMemoryPool.cpp
    Router.cpp
    ThreadPool.cpp
    utils/DateTimeUtils.cpp
    ssl/SSLContext.cpp
    ssl/SSLSocket.cpp
)

target_link_libraries(webserver_lib PRIVATE OpenSSL::SSL)

target_include_directories(webserver_lib PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(webserver_lib PRIVATE OpenSSL::SSL OpenSSL::Crypto)

# 设置库的属性
set_target_properties(webserver_lib PROPERTIES
    OUTPUT_NAME "webserver"
    POSITION_INDEPENDENT_CODE ON
)

# 设置包含目录
target_include_directories(webserver_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/modules/ColorOutput/include
)

# 设置可执行文件的包含目录
target_include_directories(webserver PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# 链接依赖库
target_link_libraries(webserver_lib PUBLIC
    nlohmann_json::nlohmann_json
    ColorOutput
    pthread
    ZLIB::ZLIB
    OpenSSL::SSL
    OpenSSL::Crypto
)

target_link_libraries(webserver PRIVATE
    webserver_lib
    ZLIB::ZLIB
)

# 在Debug模式下添加调试信息
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(webserver PRIVATE -g -O0)
    target_compile_options(webserver_lib PRIVATE -g -O0)
endif()