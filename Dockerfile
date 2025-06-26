# 构建阶段
FROM ubuntu:22.04 as builder

# 安装构建工具
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    && rm -rf /var/lib/apt/lists/*

# 拷贝源代码
WORKDIR /app
COPY . .

# 构建项目
RUN mkdir build && cd build \
    && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release .. \
    && ninja

# 运行时阶段
FROM alpine:3.19

# 安装运行时依赖
RUN apk add --no-cache libstdc++

# 从构建阶段拷贝二进制文件
WORKDIR /app
COPY --from=builder /app/build/webserver .

# 健康检查
HEALTHCHECK --interval=30s --timeout=3s \
    CMD ps | grep webserver || exit 1

# 暴露端口
EXPOSE 8080

# 启动命令
CMD ["./webserver"]