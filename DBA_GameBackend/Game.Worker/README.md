# Game.Worker - 游戏服务器管理服务

## 概述

Game.Worker 是基于 .NET Worker Service 构建的后台服务，负责管理游戏服务器的启动、监控和调度。

## 项目结构

```
Game.Worker/
├── Services/           # 后台服务实现
│   └── GameServerMonitorService.cs # 游戏服务器监控服务
├── Workers/            # Worker类
└── Program.cs          # 入口点
```

## 服务

### GameServerMonitorService

游戏服务器监控服务，负责：

1. **服务器健康检查**
   - 定期检查所有注册的游戏服务器
   - 心跳超时检测（默认超时时间：30秒）
   - 自动标记不活跃的服务器

2. **会话状态管理**
   - 监控活跃的游戏会话
   - 处理会话超时和断开连接

3. **资源分配**
   - 管理游戏服务器的端口分配
   - 协调多个游戏服务器的负载均衡

4. **事件处理**
   - 处理游戏服务器事件日志
   - 记录服务器状态变更

## 配置

### appsettings.json

```json
{
  "Logging": {
    "LogLevel": {
      "Default": "Information",
      "Microsoft.Hosting.Lifetime": "Information"
    }
  },
  "GameServer": {
    "HeartbeatTimeoutSeconds": 30,
    "CleanupIntervalSeconds": 60,
    "MaxConcurrentServers": 100
  }
}
```

## 启动命令

```bash
cd Game.Worker
dotnet run
```

## 与Game.Api的交互

Game.Worker 通过数据库和Redis与 Game.Api 进行交互：

1. **数据库**
   - 读取游戏服务器注册信息
   - 更新服务器状态
   - 管理会话数据

2. **Redis**
   - 发布/订阅游戏服务器事件
   - 缓存实时状态数据

## 部署

Game.Worker 通常部署为后台服务，可以：

1. **作为Windows服务运行**
2. **作为Docker容器运行**
3. **作为Kubernetes Deployment运行**

示例 Dockerfile：

```dockerfile
FROM mcr.microsoft.com/dotnet/aspnet:10.0
WORKDIR /app
COPY bin/Release/net10.0/publish/ .
ENTRYPOINT ["dotnet", "Game.Worker.dll"]
```