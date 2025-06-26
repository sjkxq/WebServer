cmake_minimum_required(VERSION 3.10)

project(ColorOutput LANGUAGES CXX)

# 创建库目标
add_library(ColorOutput
    src/output.cpp
)

# 设置包含目录
target_include_directories(ColorOutput PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 设置安装规则
install(TARGETS ColorOutput EXPORT webserver-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 安装头文件
install(DIRECTORY include/ DESTINATION include)

# 导出配置
install(EXPORT webserver-targets
    FILE webserver-targets.cmake
    NAMESPACE webserver::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/webserver
)