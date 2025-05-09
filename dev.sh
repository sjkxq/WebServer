#!/bin/bash
clear

# 调用构建脚本
./scripts/build.sh

echo "当前工作目录: $(pwd)"

# 调用测试脚本

./scripts/test.sh