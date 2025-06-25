# 单元测试指南

## 运行测试

```bash
# 构建时启用测试
cmake .. -DBUILD_TESTS=ON
make

# 运行所有测试
ctest --output-on-failure

# 运行特定测试
./tests/<test_name>
```

## 测试覆盖率

项目支持生成测试覆盖率报告：

```bash
# 构建时启用覆盖率
cmake .. -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON
make
ctest

# 生成HTML覆盖率报告
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

## 测试框架

项目使用Google Test框架编写单元测试，测试代码位于`tests/`目录下。