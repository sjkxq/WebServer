# 安装头文件
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/webserver
    FILES_MATCHING 
    PATTERN "*.hpp"
    PATTERN "*.h"
    PATTERN "CMakeLists.txt" EXCLUDE
)

# 创建别名目标以支持find_package
add_library(webserver-headers INTERFACE)
add_library(webserver::headers ALIAS webserver-headers)

target_include_directories(webserver-headers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/webserver>
)