# Game.Worker - 后台维护服务

## 概述

Game.Worker 是基于 .NET Worker Service 构建的后台宿主，负责运行不适合放在 HTTP 请求里的周期性维护任务。

当前包含两个后台循环：

- `MaintenanceWorker`：维护通用平台数据，例如确保每日统计行存在。
- `DedicatedServerMaintenanceWorker`：调用 `Game.ServerManagement` 的 `IDedicatedServerOrchestrator.RunMaintenanceAsync`，集中处理 Dedicated Server 启动超时、心跳超时、空闲超时和端口释放。

## 项目结构

```text
Game.Worker/
├── DedicatedServers/
│   └── DedicatedServerMaintenanceWorker.cs
├── MaintenanceWorker.cs
├── MaintenanceWorkerOptions.cs
└── Program.cs
```

## 配置

```json
{
  "WorkerJobs": {
    "IntervalSeconds": 30
  },
  "DedicatedServerMaintenanceWorker": {
    "IntervalSeconds": 15
  },
  "GameServerManager": {
    "ServerMode": "LocalProcess",
    "PublicIp": "127.0.0.1",
    "PortRangeStart": 7777,
    "PortRangeEnd": 7797,
    "UeServerExecutablePath": "",
    "BackendUrl": "http://localhost:5000"
  }
}
```

`GameServerManager` 是历史配置节名，当前由 `DedicatedServerOrchestrationOptions` 读取，用于兼容既有环境变量和 Docker Compose。

## 启动命令

```bash
cd Game.Worker
dotnet run
```

## 部署

Game.Worker 通常作为后台进程、Docker 容器或 Kubernetes Deployment 运行。生产环境必须配置数据库连接，以及 Dedicated Server 可执行文件路径或后续的容器调度参数。
