# 步骤 08：统一 ApiClient、Token、请求取消与错误映射

## 结论

`GameBackendClient` 插件中的 `FDBA_GameBackendHttpClient` 是唯一实际 HTTP 传输实现。本步骤没有再创建 `FHttpModule` 或第二个 REST Client；新增的 `UDBAApiClientSubsystem` 只是前台 Domain Service 的唯一安全门面。

## 迁移清单

| 现有实现 | 状态 | 目标实现 | 迁移办法 |
| --- | --- | --- | --- |
| `FDBA_GameBackendHttpClient` | KEEP | 唯一传输层 | 补充请求句柄、取消、关联标识和每请求超时选项；旧 `Get/Post/Put/Delete` 保持兼容。 |
| `UDBA_GameBackendClientSubsystem` | KEEP | 认证状态与底层 Service 容器 | 继续负责内存 Token 与刷新请求的底层 Single-Flight。 |
| `UDBAApiClientSubsystem` | KEEP（新增） | 前台唯一 API 门面 | 新前台 Domain Service 通过它发送请求；禁止 Widget 直接调用 HTTP 或持有原始 JSON。 |
| `UDBAOnlineAccountService` 直接设置底层 Token | ADAPT | 通过 `UDBAApiClientSubsystem::SetAuthenticationTokens` | 登录、自动登录、注销均经过门面。 |
| `UDBABackendFacadeSubsystem::SynchronizeAuthentication` | ADAPT | 调用 `UDBAApiClientSubsystem` | 保持既有大厅/会话 API 不变。 |
| `UDBAAccountSaveGame.SessionToken/RefreshToken` | DEPRECATE | `UDBASecureTokenStorage` | 不再读取或写入旧明文存档；发现后立即清空。字段暂保留以安全兼容已有存档。 |
| `UDBADevelopmentTokenStorage` | KEEP（开发） | 进程内 RefreshToken 存储 | 不跨进程持久化、不写 Config/SaveGame/日志；发布平台必须替换为系统安全存储。 |
| `UDBAFrontendErrorMapper` | KEEP | UI 的唯一结构化错误映射 | `HTTP + backend ErrorCode -> FDBAApiError -> FText`；Widget 不判断远端英文 message。 |
| `UDBAMainLobbyWidgetController` 的原始 JSON delegate | DEPRECATE | 后续按领域 DTO 迁移 | 本步骤不改动仍在使用的大型大厅链，避免扩大范围。 |

## 请求链路

```text
Widget（意图）
  -> WidgetController / ViewModel
  -> Domain Service（DTO 编解码）
  -> UDBAApiClientSubsystem
  -> FDBA_GameBackendHttpClient
  -> HTTPS REST/JSON
```

`UDBAApiClientSubsystem` 为每项请求生成 CorrelationId，并由传输层写入 `X-Trace-Id`。日志只记录方法、路径、关联标识、状态码、业务错误码和耗时；禁止记录 AccessToken、RefreshToken、Password 或 GameTicket。

## 401、注销与生命周期

1. 前台门面让底层传输把 401 返回给它；同一批 401 仅触发一次 `RequestRefreshToken`。
2. 其余 401 请求等待刷新结果，成功后各自只重试一次；刷新失败统一映射为 `auth.refresh_failed`。
3. `CancelRequest` 取消实际 HTTP 请求；等待刷新或重试的请求也会被本地失效。
4. 回调使用 `TWeakObjectPtr<UObject>` 绑定页面/控制器所有者；所有者失效或主动取消时不会更新 UI。
5. 注销先取消前台请求，最终清空 Token、失效会话代次和开发存储，旧回调无法回写。

## DTO 与测试边界

`FDBAApiJsonSerializer` 只供 Domain Service 在 `DomainJson` 与 DTO 间转换。`FDBAApiResponse::Result` 是可安全交给 UI 的结果，`DomainJson` 不得由 Widget、ViewModel 或 WidgetController 直接持有。

提供 `SetMockTransportForTests` 与 `SetMockRefreshForTests`，可在没有后端的情况下验证 401 Single-Flight、取消与 UI 生命周期。契约测试位于 `Source/DivineBeastsArena/Private/Tests/DBAApiClientTests.cpp`。

## 后续收敛规则

- 新登录、选服、角色列表、角色创建与进服 Domain Service 必须使用 `UDBAApiClientSubsystem`。
- 现有 `GameBackend*Service` 和大厅原始 JSON 回调在对应业务迁移时逐项收敛，不得再创建新的 HTTP Helper。
- Android、Windows、Linux 发行版接入时，必须提供 `UDBASecureTokenStorage` 的平台安全实现；不得恢复 `SaveGame` 明文 Token。
