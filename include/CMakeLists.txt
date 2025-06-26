# 头文件安装规则
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION include/webserver
    FILES_MATCHING PATTERN "*.hpp"
)