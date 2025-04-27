/*
 * @Author       : mark
 * @Date         : 2020-06-18
 * @copyleft Apache 2.0
 */ 
#include <unistd.h>
#include "server/webserver.h"

int main() {
    /* 守护进程 后台运行 */
    //daemon(1, 0); 

    // 从配置文件读取参数
    Config config;
    config.ParseConfig();
    
    WebServer server(
        config.port, config.trigMode, config.timeoutMS, config.openLinger,
        config.sqlPort, config.sqlUser.c_str(), config.sqlPwd.c_str(), config.dbName.c_str(),
        config.connPoolNum, config.threadNum, config.openLog, config.logLevel, config.logQueSize);
    server.Start();
}
  