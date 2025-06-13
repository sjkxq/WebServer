#include <benchmark/benchmark.h>
#include "ThreadPool.hpp"

// 测试线程池任务提交性能
static void BM_ThreadPoolSubmit(benchmark::State& state) {
    ThreadPool pool(state.range(0));
    for (auto _ : state) {
        pool.submit([](){});
    }
}
BENCHMARK(BM_ThreadPoolSubmit)
    ->Arg(1)    // 1线程
    ->Arg(4)    // 4线程
    ->Arg(8);   // 8线程

// 测试线程池任务执行吞吐量
static void BM_ThreadPoolThroughput(benchmark::State& state) {
    ThreadPool pool(state.range(0));
    std::atomic<int> counter{0};
    for (auto _ : state) {
        for (int i = 0; i < state.range(1); ++i) {
            pool.submit([&counter](){ counter++; });
        }
        pool.waitAll();
    }
}
BENCHMARK(BM_ThreadPoolThroughput)
    ->Args({4, 1000})    // 4线程, 1000任务
    ->Args({8, 10000});  // 8线程, 10000任务

BENCHMARK_MAIN();