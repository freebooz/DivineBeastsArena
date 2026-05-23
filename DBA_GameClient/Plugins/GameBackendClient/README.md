# DBA_GameBackendClient

## 1. 插件用途
`DBA_GameBackendClient` 为 Unreal 客户端提供统一后端通信能力，覆盖：

- 登录与令牌刷新
- 版本检查、维护状态、公告与配置拉取
- 房间、匹配、会话连接
- 邮件、反馈、举报
- 埋点上报
- 崩溃与客户端日志上传

所有接口均为异步调用，不阻塞 Game Thread。

## 2. 如何启用插件
1. 打开项目。
2. 进入 `Edit -> Plugins`。
3. 确认 `Game Backend Client` 已启用。
4. 重启编辑器并重新编译。

## 3. 如何配置 BackendBaseUrl
支持在 Project Settings 或 `DefaultGame.ini` 配置：

```ini
[/Script/DBA_GameBackendClient.DBA_GameBackendClientSettings]
BackendBaseUrl=http://localhost:5000
ClientVersion=0.1.0
BuildNumber=100
ConfigVersion=bootstrap_v1
Channel=dev
Platform=Windows
Region=local
RequestTimeoutSeconds=15
TelemetryFlushIntervalSeconds=10
TelemetryMaxQueueSize=1000
HttpRetryCount=1
```

## 4. 蓝图调用示例
1. `Get Game Instance Subsystem` -> `DBA_GameBackendClientSubsystem`
2. `GetAuthService` -> `DevLogin("frontend_debug")`
3. 登录成功后调用：
   - `GetPlayerService -> GetMyProfile`
   - `GetConfigService -> GetConfigBundle`

## 5. C++ 调用示例
```cpp
UDBA_GameBackendClientSubsystem* Backend = GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>();
if (Backend && Backend->GetAuthService())
{
	FDBA_GameBackendAuthResponseDelegate LoginCallback;
	LoginCallback.BindLambda([](bool bSuccess, const FString& Error, const FString& AccessToken, const FString& RefreshToken, const FString& PlayerId)
	{
		// Handle login result
	});
	Backend->GetAuthService()->DevLogin(TEXT("frontend_debug"), LoginCallback);
}
```

## 6. 登录流程
1. 发起 `DevLogin / GuestLogin / SteamLogin / EosLogin`
2. 保存 `AccessToken / RefreshToken / PlayerId`
3. 拉取 `GetMyProfile`
4. 拉取 `GetConfigBundle`
5. 进入大厅 UI

若后端返回封禁信息，错误文本会包含封禁原因与解封时间（UTC）。

## 7. 匹配流程
1. `CreateTicket(Mode, Region)`
2. 轮询 `GetTicket(TicketId)`
3. 获取 `SessionId`
4. `GetSession + GetConnection` 后执行连接
5. 支持 `CancelTicket(TicketId)`

## 8. 连接 Dedicated Server 流程
1. `GetConnection(SessionId)` 获取 `ip/port/playerSessionToken`
2. 生成 Travel URL：
   - `{ip}:{port}?SessionId={sessionId}&PlayerSessionToken={token}&PlayerId={playerId}`
3. 调用 `PlayerController->ClientTravel(...)`
4. Dedicated Server 读取启动参数 `-sessionId -serverId -backendUrl -runtimeToken` 后会自动调用 Runtime API：
   - `/runtime/servers/register`
   - `/runtime/servers/ready`
   - `/runtime/servers/heartbeat`
   - `/runtime/servers/player-joined`
   - `/runtime/servers/player-left`

## 9. 崩溃上传流程
1. 启动扫描 `Saved/Crashes`
2. 异步上传至 `/api/crashes/upload`
3. 最近客户端日志上传至 `/api/client-logs/upload`
4. 日志上传单文件大小限制：`10MB`
5. 上传成功后写入 `.uploaded` 标记，失败保留文件重试

## 9.1 客户端埋点事件
默认接入并上报以下事件（异步）：

- `client_started`
- `login_success`
- `login_failed`
- `enter_lobby`
- `matchmaking_started`
- `matchmaking_found`
- `connect_server_started`
- `connect_server_failed`
- `match_finished_client_view`
- `client_exit`

埋点行为：

- 内存队列缓存
- 每 10 秒自动 Flush
- 队列达到 50 条立即 Flush
- 最大缓存 1000 条
- 游戏退出时会 Flush 队列

## 10. 不允许客户端实现的逻辑
客户端不得实现或决定：

- 最终胜负与权威战报
- 击杀结果、奖励结果、货币变化、背包变化
- 排行榜最终结果、付费最终结果、封禁最终结果
- 绕过版本检查或维护状态
- 明文存储敏感密钥

## 11. 常见错误排查
- `401 Unauthorized`：检查 AccessToken 是否过期，确认 RefreshToken 可用。
- 无法连接后端：检查 `BackendBaseUrl`、端口、防火墙和服务状态。
- 匹配卡住：检查 Ticket 状态与 Session 是否下发。
- 无法进服：检查 `GetConnection` 返回是否包含 ip/port/token。
- 崩溃日志未上传：检查文件权限、`10MB` 大小限制和 `.uploaded` 标记。

## 11.1 安全说明（埋点/崩溃）
- 埋点属性会过滤敏感字段，避免上传 `AccessToken`、`RefreshToken`、`PlayerSessionToken`、`Authorization`。
- 上传日志前会进行敏感行脱敏处理（例如包含上述 Token 的日志行）。

## 12. 最小接入优先级
1. DBA_GameBackendClient 插件基础 HTTP 能力  
2. DevLogin(`frontend_debug`)  
3. VersionCheck + MaintenanceStatus  
4. ConfigBundle 拉取  
5. PlayerProfile / Inventory  
6. Room / Match  
7. Session Connection + ClientTravel  
8. Mail  
9. Support / Report  
10. Telemetry / CrashUpload

## 13. 权威边界
- 客户端负责：表现、输入与请求发起（全部异步）
- DBA_GameBackend 负责：业务状态与流程规则
- Dedicated Server 负责：对局权威
- Settlement 负责：最终结算
- Inventory / Wallet 负责：资产变更
- GM / Admin 负责：运营处理

## 登录界面测试账号（默认）
- `frontend_debug`
- `test_player_001`
- `test_player_003_banned`

登录界面“调试登录”按钮默认调用：

- `DevLogin("frontend_debug")`
