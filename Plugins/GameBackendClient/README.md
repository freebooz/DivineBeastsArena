# GameBackendClient

## 1) 插件用途
`GameBackendClient` 用于 Unreal 客户端与游戏后端通信，覆盖：

- 登录与令牌刷新
- 版本检查与维护状态
- 配置拉取
- 房间、匹配、会话连接
- 邮件、客服、举报
- 客户端埋点
- 崩溃与日志上传

所有接口均为异步，不阻塞 Game Thread。

## 2) 如何启用插件
1. 打开 Unreal 工程。
2. 在 `Edit -> Plugins` 中确认 `Game Backend Client` 已启用。
3. 重启编辑器。

## 3) 如何配置 BackendBaseUrl
可在 Project Settings 或 `DefaultGame.ini` 配置：

```ini
[/Script/GameBackendClient.GameBackendClientSettings]
BackendBaseUrl=http://localhost:5000
ClientVersion=0.1.0
BuildNumber=100
Channel=dev
Platform=Windows
Region=local
RequestTimeoutSeconds=15
TelemetryFlushIntervalSeconds=10
TelemetryMaxQueueSize=1000
```

## 4) 蓝图调用示例
在蓝图中通过 `Get Game Instance Subsystem` 获取 `GameBackendClientSubsystem`：

1. `GetAuthService -> DevLogin("frontend_debug")`
2. 登录成功后调用：
   - `GetPlayerService -> GetMyProfile`
   - `GetConfigService -> GetConfigBundle`
3. 成功后切换到大厅 UI。

## 5) C++ 调用示例
```cpp
UGameBackendClientSubsystem* Backend = GetGameInstance()->GetSubsystem<UGameBackendClientSubsystem>();
Backend->GetAuthService()->DevLogin(
	TEXT("frontend_debug"),
	FGameBackendAuthResponseDelegate::CreateLambda(
		[](bool bSuccess, const FString& Error, const FString& AccessToken, const FString& RefreshToken, const FString& PlayerId)
		{
			// handle result
		}));
```

## 6) 登录流程
1. `DevLogin / GuestLogin / SteamLogin / EosLogin`
2. 保存 `AccessToken / RefreshToken / PlayerId`
3. 拉取 `GetMyProfile` 与 `GetConfigBundle`
4. 进入大厅

若后端返回封禁错误，客户端展示封禁原因与解封时间（UTC）。

## 7) 匹配流程
1. `CreateTicket(Mode, Region)`
2. 每 2 秒轮询 `GetTicket(TicketId)`
3. 匹配成功后停止轮询，调用会话接口
4. 可调用 `CancelTicket(TicketId)` 取消并停止轮询

## 8) 连接 Dedicated Server 流程
1. `GetSession(SessionId)`
2. `GetConnection(SessionId)`
3. 拼接 URL：
   - `{ip}:{port}?SessionId={sessionId}?PlayerSessionToken={token}`
4. `PlayerController->ClientTravel(...)`

## 9) 崩溃上传流程
1. 启动扫描 `Saved/Crashes`
2. 上传未标记文件到 `/api/crashes/upload`
3. 上传最近日志到 `/api/client-logs/upload`（单文件最大 10MB）
4. 成功后写入 `.uploaded` 标记；失败保留，下次重试

## 10) 客户端禁止实现的逻辑
客户端不得实现或决定：

- 胜负结算
- 击杀/战报权威结果
- 经验/奖励最终结果
- 资产变更（金币、背包等）
- 排行榜最终分数
- 版本与维护绕过

## 11) 常见错误排查
- `401 Unauthorized`：检查 AccessToken 是否过期，确认 RefreshToken 可用。
- `BackendBaseUrl` 无法访问：确认后端服务已启动，端口可连通。
- 匹配卡住：确认 `GetTicket` 返回状态，检查是否拿到 `SessionId`。
- 无法进服：检查 `GetConnection` 是否返回 `ip/port/playerSessionToken`。
- 崩溃日志未上传：确认文件大小限制与上传标记文件状态。

## 测试账号配置（登录界面）
默认调试账号建议：

- `frontend_debug`
- `test_player_001`
- `test_player_003_banned`

登录界面“调试登录”按钮默认调用：

- `DevLogin("frontend_debug")`

