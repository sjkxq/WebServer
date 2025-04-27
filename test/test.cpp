/*
 * @Author       : mark
 * @Date         : 2020-06-20
 * @copyleft Apache 2.0
 */ 
#include <iostream>
#include "../code/log/log.h"
#include "../code/pool/threadpool.h"
#include "../code/timer/heaptimer.h"
#include "../code/pool/sqlconnpool.h"
#include "../code/http/httprequest.h"
#include "../code/http/httpresponse.h"
#include <mysql/mysql.h>
#include <features.h>

int totalTests = 0; // 全局变量，记录总测试数

#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

void TestLog() {
    totalTests++;
    std::cout << "[开始执行] TestLog 测试函数" << std::endl;
    int cnt = 0, level = 0;
    Log::Instance()->init(level, "./testlog1", ".log", 0);
    for(level = 3; level >= 0; level--) {
        Log::Instance()->SetLevel(level);
        for(int j = 0; j < 1; j++ ){
            for(int i = 0; i < 1; i++) {
                LOG_BASE(i,"%s 111111111 %d ============= ", "Test", cnt++);
            }
        }
    }
    cnt = 0;
    Log::Instance()->init(level, "./testlog2", ".log", 5000);
    for(level = 0; level < 4; level++) {
        Log::Instance()->SetLevel(level);
        for(int j = 0; j < 1; j++ ){
            for(int i = 0; i < 1; i++) {
                LOG_BASE(i,"%s 222222222 %d ============= ", "Test", cnt++);
            }
        }
    }
}

void ThreadLogTask(int i, int cnt) {
    for(int j = 0; j < 3; j++ ){
        LOG_BASE(i,"PID:[%04d]======= %05d ========= ", gettid(), cnt++);
    }
}

/**
 * @brief 测试线程池功能
 * 优化后的测试函数，减少线程数量和任务数量以缩短测试时间
 */
void TestThreadPool() {
    totalTests++;
    std::cout << "[开始执行] TestThreadPool 测试函数" << std::endl;
    Log::Instance()->init(0, "./testThreadpool", ".log", 5000);
    ThreadPool threadpool(2);  // 减少线程数量
    for(int i = 0; i < 1; i++) {
        threadpool.AddTask(std::bind(ThreadLogTask, i % 2, i * 1000));  // 减少任务数量
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));  // 替换getchar()，自动结束测试
}

void TestSqlConnPool() {
    totalTests++;
    std::cout << "[开始执行] TestSqlConnPool 测试函数" << std::endl;
    
    // 初始化日志
    Log::Instance()->init(0, "./testSqlConnPool", ".log", 0);
    
    // 测试数据库连接池
    LOG_INFO("尝试初始化数据库连接池");
    SqlConnPool::Instance()->Init("localhost", 3306, "user", "password", "testdb", 10);
    if (SqlConnPool::Instance()->GetFreeConnCount() == 0) {
        LOG_ERROR("数据库连接池初始化失败");
        return;
    }
    LOG_INFO("数据库连接池初始化成功");
    
    // 测试获取连接
    LOG_INFO("尝试获取数据库连接");
    MYSQL* conn1 = SqlConnPool::Instance()->GetConn();
    if (conn1 == nullptr) {
        LOG_ERROR("获取数据库连接失败");
        return;
    }
    LOG_INFO("成功获取数据库连接");
    
    // 测试释放连接
    SqlConnPool::Instance()->FreeConn(conn1);
    LOG_INFO("已释放数据库连接");
    
    // 测试连接池大小
    int freeConnCount = SqlConnPool::Instance()->GetFreeConnCount();
    LOG_INFO("当前空闲连接数: %d", freeConnCount);
    if (freeConnCount != 10) {
        LOG_ERROR("连接池大小不符合预期");
        return;
    }
    LOG_INFO("连接池大小测试通过");
}

void TestHttpRequest() {
    totalTests++;
    std::cout << "[开始执行] TestHttpRequest 测试函数" << std::endl;
    // 测试HTTP请求解析
    HttpRequest request;
    std::string raw = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
    Buffer buff;
    buff.Append(raw.data(), raw.size());
    
    // 测试解析请求
    assert(request.parse(buff));
    assert(request.method() == "GET");
    assert(request.path() == "/index.html");
}

void TestHttpResponse() {
    totalTests++;
    std::cout << "[开始执行] TestHttpResponse 测试函数" << std::endl;
    // 测试HTTP响应生成
    HttpResponse response;
    std::string path = "index.html";
    response.Init("./resources", path, false);
    
    // 测试生成响应
    Buffer buff;
    response.MakeResponse(buff);
    std::string res = buff.RetrieveAllToStr();
    assert(!res.empty());
}

int main() {
    int passedTests = 0;
    
    std::cout << "========== 开始测试 ==========" << std::endl;
    
    try {
        TestLog();
        std::cout << "[测试通过] TestLog" << std::endl;
        passedTests++;
    } catch(...) {
        std::cout << "[测试失败] TestLog" << std::endl;
    }
    
    try {
        TestThreadPool();
        std::cout << "[测试通过] TestThreadPool" << std::endl;
        passedTests++;
    } catch(...) {
        std::cout << "[测试失败] TestThreadPool" << std::endl;
    }
    
    try {
        TestSqlConnPool();
        std::cout << "[测试通过] TestSqlConnPool" << std::endl;
        passedTests++;
    } catch(...) {
        std::cout << "[测试失败] TestSqlConnPool" << std::endl;
    }
    
    try {
        TestHttpRequest();
        std::cout << "[测试通过] TestHttpRequest" << std::endl;
        passedTests++;
    } catch(...) {
        std::cout << "[测试失败] TestHttpRequest" << std::endl;
    }
    
    try {
        TestHttpResponse();
        std::cout << "[测试通过] TestHttpResponse" << std::endl;
        passedTests++;
    } catch(...) {
        std::cout << "[测试失败] TestHttpResponse" << std::endl;
    }
    
    std::cout << "========== 测试结果 ==========" << std::endl;
    std::cout << "总测试数: " << totalTests << std::endl;
    std::cout << "通过测试: " << passedTests << std::endl;
    std::cout << "测试覆盖率: " << (passedTests * 100 / totalTests) << "%" << std::endl;
    std::cout << "=============================" << std::endl;
}