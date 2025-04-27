#!/bin/bash
clear

# 调用构建脚本
./scripts/build.sh

# 调用测试脚本
./scripts/test.sh

# 调用压力测试脚本
./scripts/benchmark.sh