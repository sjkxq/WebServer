cmake_minimum_required(VERSION 3.14)
project(WebServer VERSION 1.0 LANGUAGES CXX)

# 包含配置文件
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompilerOptions.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Dependencies.cmake)

# 启用测试
enable_testing()

# 添加子目录
add_subdirectory(modules/ColorOutput)
add_subdirectory(include)
add_subdirectory(src)

if(BUILD_TESTS)
    add_subdirectory(test)
endif()

if(BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif()

# 安装配置
include(GNUInstallDirs)
install(EXPORT webserver-targets
    FILE webserver-targets.cmake
    NAMESPACE webserver::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/webserver
)

# 生成并安装配置文件
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/webserver-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/webserver-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/webserver
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/webserver-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/webserver-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/webserver-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/webserver
)