#include <benchmark/benchmark.h>
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include <iostream>
#include <thread>
#include <chrono>

// 主函数会由benchmark库自动生成
BENCHMARK_MAIN();