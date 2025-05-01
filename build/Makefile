# 检测操作系统类型
UNAME := $(shell uname)

CXX = g++
CFLAGS = -std=c++17 -O2 -Wall -Wextra -g 

# 根据操作系统设置不同的目标文件名和清理命令
ifeq ($(UNAME), Linux)
    TARGET = server
    RM = rm -rf
else
    TARGET = server.exe
    RM = del /Q
endif

OBJS = ../code/log/*.cpp ../code/pool/*.cpp ../code/timer/*.cpp \
       ../code/http/*.cpp ../code/server/*.cpp \
       ../code/buffer/*.cpp ../code/main.cpp ../code/config/config.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)  -pthread -lmysqlclient

clean:
	$(RM) ../bin/$(TARGET) ../code/*/*.o




