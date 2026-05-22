# Dedicated Server Production Strategy

首发 Dedicated Server 管理模式：`LocalProcess`。

原因：

- 个人开发和单机房首发部署更容易排查问题。
- 当前 UE Dedicated Server 包可以直接以进程方式启动，和现有 `GameServerManager__UeServerExecutablePath` 配置匹配。
- Docker / Agones / Kubernetes 仍保留为扩展方向，但不作为首发阻塞项。

首发运行要求：

- 在生产服务器上放置已打包的 UE Dedicated Server 可执行文件。
- `.env` 中设置 `GAME_SERVER_MODE=LocalProcess`。
- `.env` 中设置 `UE_SERVER_EXECUTABLE_PATH` 为真实可执行文件路径。
- 配置 `GAME_SERVER_PUBLIC_IP` 为客户端可连接的公网 IP 或内网穿透地址。
- 开放 `GAME_SERVER_PORT_START` 到 `GAME_SERVER_PORT_END` 的 UDP 端口。

Game Server Manager 启动参数：

```text
-sessionId=<sessionId>
-serverId=<serverId>
-port=<udpPort>
-mapId=<mapId>
-mode=<mode>
-backendUrl=<backendUrl>
-runtimeToken=<runtimeToken>
```

已自动化覆盖的规则：

- 同一个 session 重复分配只返回同一个 server。
- 释放服务器会释放 UDP 端口。
- STARTING 超时会标记为 TIMEOUT。
- IN_PROGRESS 心跳超时会标记为 STOPPED。

仍需人工联调：

- 使用真实 UE Dedicated Server 包验证进程启动。
- 验证 Runtime API register / ready / heartbeat。
- 验证客户端拿到 IP、端口和 PlayerSessionToken 后可进入对局。
- 验证进程崩溃退出码和日志采集。
