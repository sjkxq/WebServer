#!/bin/bash
# 运行压力测试

# echo "当前工作目录: $(pwd)"

# 读取配置文件中的端口号
port=$(grep '^port=' config.ini | cut -d'=' -f2)
if [ -z "$port" ]; then
    echo "错误: 未在config.ini中找到端口配置"
    exit 1
fi

cd ./webbench-1.5
make clean
make
cd ../bin
./server &
# 等待3秒确保服务器完全启动
sleep 3
cd ../webbench-1.5
./webbench -c 10 -t 5 http://localhost:$port/