#!/bin/bash
clear

# 构建项目
cd build && make clean
cd .. && make all

# 运行测试
cd test
make clean
make all
# ./test

# 运行压力测试
cd ../webbench-1.5
make clean
make
cd ../bin
./server &
# 等待3秒确保服务器完全启动
sleep 3
cd ../webbench-1.5
./webbench -c 10 -t 5 http://localhost:1316/
# ./webbench-1.5/webbench -c 100 -t 10 http://localhost:1316/
# ./webbench-1.5/webbench -c 1000 -t 10 http://localhost:1316/
# ./webbench-1.5/webbench -c 5000 -t 10 http://localhost:1316/
# ./webbench-1.5/webbench -c 10000 -t 10 http://localhost:1316/