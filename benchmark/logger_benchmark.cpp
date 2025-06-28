#include <benchmark/benchmark.h>
#include "Logger.hpp"
#include <thread>

class LoggerBenchmarkConfig {
public:
    LoggerBenchmarkConfig() {
        webserver::Logger::getInstance().setLogFile("benchmark_logger.log");
        webserver::Logger::getInstance().setConsoleOutput(false);
    }
};

static LoggerBenchmarkConfig loggerBenchmarkConfig;

// 测试不同日志级别的性能
static void BM_LoggerPerformance(benchmark::State& state) {
    webserver::Logger::getInstance().setConsoleOutput(false);
    webserver::Logger::getInstance().setLogLevel(static_cast<webserver::Logger::Level>(state.range(0)));
    for (auto _ : state) {
        LOG_DEBUG("This is a debug message");
        LOG_INFO("This is an info message");
        LOG_WARNING("This is a warning message");
        LOG_ERROR("This is an error message");
    }
}
BENCHMARK(BM_LoggerPerformance)
    ->Arg(0)    // DEBUG
    ->Arg(1)    // INFO
    ->Arg(2)    // WARNING
    ->Arg(3);   // ERROR

// 测试多线程日志性能
static void BM_MultiThreadLogger(benchmark::State& state) {
    webserver::Logger::getInstance().setConsoleOutput(false);
    webserver::Logger::getInstance().setLogLevel(webserver::Logger::Level::INFO);
    const auto thread_count = static_cast<int>(state.range(0));
    const int logs_per_thread = 1000;
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&]() {
                for (int j = 0; j < logs_per_thread; ++j) {
                    LOG_INFO("Thread log message");
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
}
BENCHMARK(BM_MultiThreadLogger)
    ->Arg(1)    // 1线程
    ->Arg(4)    // 4线程
    ->Arg(8);   // 8线程