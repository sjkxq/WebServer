# 性能基准测试指南

## 最新测试结果示例

```
--------------------------------------------------------------------
Benchmark                          Time             CPU   Iterations
--------------------------------------------------------------------
BM_LoggerPerformance/0           11208 ns        11208 ns        62607
BM_LoggerPerformance/1            8415 ns         8415 ns        83476  
BM_LoggerPerformance/2            5684 ns         5683 ns       123231
BM_LoggerPerformance/3            2979 ns         2979 ns       232739
```

## 性能测试最佳实践

1. 在性能测试前配置日志系统以获得准确结果：
```cpp
// 禁用控制台日志输出
webserver::Logger::getInstance().setConsoleOutput(false);

// 将日志输出重定向到文件
webserver::Logger::getInstance().setLogFile("benchmark.log");
```

2. 使用Release模式构建以获得真实性能数据：
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

3. 确保测试环境稳定，没有其他高负载进程

## 运行性能测试

```bash
# 构建时启用benchmark
cmake .. -DBUILD_BENCHMARK=ON
make

# 运行benchmark测试
./webserver_benchmarks
```