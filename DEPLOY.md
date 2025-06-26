# 部署指南

## 容器化部署

### 构建Docker镜像
```bash
docker build -t webserver .
```

### 运行容器
```bash
docker run -d \
  -p 8080:8080 \
  --name webserver \
  webserver
```

### 高级选项
- 设置环境变量：
```bash
docker run -d \
  -e PORT=8080 \
  -p 8080:8080 \
  webserver
```

- 挂载配置文件：
```bash
docker run -d \
  -v ./config:/app/config \
  webserver
```

## 本地部署

### 运行服务
```bash
./build/webserver
```

### 配置选项
通过环境变量配置：

| 变量名 | 描述 | 默认值 |
|-------|------|--------|
| PORT | 服务监听端口 | 8080 |
| LOG_LEVEL | 日志级别 (debug/info/warn/error) | info |

## 健康检查

### 容器健康检查
Docker容器内置健康检查，可通过以下命令查看状态：
```bash
docker inspect --format='{{json .State.Health}}' webserver
```

### 本地运行健康检查
```bash
curl http://localhost:8080/health
```

## 监控和日志

### 查看容器日志
```bash
docker logs webserver
```

### 实时日志
```bash
docker logs -f webserver
```

### 资源监控
```bash
docker stats webserver
```

## 更新部署
1. 停止旧容器：
```bash
docker stop webserver
```
2. 删除旧容器：
```bash
docker rm webserver
```
3. 构建新镜像并运行