/*
 * @Author       : mark
 * @Date         : 2020-06-18
 * @copyleft Apache 2.0
 */ 
#include <unistd.h>
#include "server/webserver.h"
#include "./config/config.h"

int main() {
    /* 守护进程 后台运行 */
    //daemon(1, 0); 

    // 从配置文件读取参数
    Config config("../config.ini");
    
    WebServer server(
        config.GetInt("port"), config.GetInt("trigMode"), config.GetInt("timeoutMS"), config.GetBool("openLinger"),
        config.GetInt("sqlPort"), config.GetString("sqlUser").c_str(), config.GetString("sqlPwd").c_str(), config.GetString("dbName").c_str(),
        config.GetInt("connPoolNum"), config.GetInt("threadNum"), config.GetBool("openLog"), config.GetInt("logLevel"), config.GetInt("logQueSize"));
    server.Start();
}
  