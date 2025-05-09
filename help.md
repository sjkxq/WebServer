# 项目运行可能需要执行的代码
```bash
sudo apt update
sudo apt install libmysqlclient-dev

sudo apt update
sudo apt install libtirpc-dev

sudo ln -s /usr/include/tirpc/rpc/types.h /usr/include/rpc/types.h

sudo apt update
sudo apt install libtirpc-dev

sudo ln -s /usr/include/tirpc/netconfig.h /usr/include/netconfig.h

chmod +x dev.sh

chmod +x scripts/benchmark.sh
chmod +x scripts/build.sh
chmod +x scripts/test.sh
chmod +x scripts/init_db.sh
```

# 参考脚本
## dev.sh
```bash
#!/bin/bash
clear

# 调用构建脚本
./scripts/build.sh

echo "当前工作目录: $(pwd)"

# 调用测试脚本

./scripts/test.sh
```
## build.sh
```bash
#!/bin/bash
# 构建项目
cd build && make clean
cd .. && make all
```
## test.sh
```bash
#!/bin/bash
# 运行测试
cd test
make clean
make all
./test
```

