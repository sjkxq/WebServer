# 头文件库配置

# 创建头文件库
add_library(color_output INTERFACE)

# 设置包含目录 - 使用相对路径
target_include_directories(color_output INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/webserver>
)

# 安装规则
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/webserver
    FILES_MATCHING PATTERN "*.hpp"
)

# 导出目标
install(TARGETS color_output
    EXPORT webserver-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)