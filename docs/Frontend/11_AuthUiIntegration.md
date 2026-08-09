# 步骤 11：UE 登录、注册与自动登录收敛

## 范围与审计结论

本步骤在既有前台链路上收敛账号入口，不移动或直接编辑 `.uasset` / `.umap`。审计确认 `UDBAOnlineAccountService` 已是唯一的跨地图账号子系统；`GameBackendClient` 是其唯一 HTTP 传输实现；`UDBAFrontendFlowSubsystem` 是唯一前台状态机。

| 现有实现 | 结论 | 目标与迁移方式 |
| --- | --- | --- |
| `UDBAOnlineAccountService` | KEEP | 保持唯一账号权威入口，补充语义化入口 `TryAutoLogin`、`LoginWithCredentials`、`RegisterAccount`、`RefreshSession`。 |
| `UDBAApiClientSubsystem` + `UDBASecureTokenStorage` | KEEP | AccessToken 仅驻留内存；RefreshToken 仅经安全存储抽象持久化。记住登录关闭时立即清除持久化刷新令牌。 |
| `UDBA_GameBackendAuthService` | ADAPT | 账号登录、注册、刷新、登出、`me` 收敛到 `/api/v1/auth/*`；保留游客旧路由，因为后端尚未提供 v1 游客端点。 |
| `UDBAFrontendFlowSubsystem` | KEEP / ADAPT | 登录、注册、自动登录成功统一进入 `ServerSelect`；只有选服成功后才进入 `CharacterRosterLoading`。 |
| `UDBALoginWidgetController` + `UDBALoginViewModel` | MERGE | 一个 Controller/一个 ViewModel 服务登录和注册显示状态，Widget 不触碰 HTTP、Token 或原始 JSON。 |
| `WBP_DBA_Login` | KEEP | 已有 C++ 父类的注册按钮从占位提示改为发送 `BeginRegistration` 意图。 |
| `WBP_DBA_Register` | ADAPT | 保留资产，不直接改二进制；后续人工在 Editor 将提交/返回绑定至 `UDBALoginWidgetController` 的 `RegisterWithCredentials` / `CancelRegistration`，并以 `UDBALoginViewModel` 绑定 loading 与结构化错误。 |

## 最终业务链

```text
进程启动
  -> Flow.StartLoginFlow
  -> AutoLogin（有安全存储的 RefreshToken 时）或 Login
  -> AuthSubsystem / OnlineAccountService
  -> 成功：ServerSelect
  -> SelectServer(ServerId)
  -> CharacterRosterLoading
  -> CharacterSelect 或 CharacterCreate_Zodiac
```

自动登录失败是可恢复分支：不显示循环错误弹窗，直接返回 `Login`，用户可继续手动登录。登录、游客登录和注册均有 Flow 层的 in-flight guard，重复按钮事件不会创建第二次认证业务请求。

## 职责边界

- Widget：账号与密码输入、按钮、加载表现、焦点和 Back；不持有 Token，不请求 HTTP。
- `UDBALoginViewModel`：仅保存可展示的异步状态、结构化 `FDBAApiError`、游客开关和记住登录选择。
- `UDBALoginWidgetController` / `UDBAFrontendFlowController`：把 UI 意图转发给 Flow，订阅状态与错误事件。
- `UDBAFrontendFlowSubsystem`：前台状态转换、取消/回退和认证完成后的选服路由。
- `UDBAOnlineAccountService`：账号认证、刷新、登出与会话缓存；不创建 Widget。
- `UDBAApiClientSubsystem`：唯一 Token/请求生命周期入口；`UDBASecureTokenStorage` 是刷新令牌的唯一持久化边界。

## 配置与安全

`[/Script/DivineBeastsArena.DBAFrontendSettings]` 增加：

- `bEnableGuestLogin`：保留项目已有游客入口的显式开关。
- `bAllowDevelopmentLogin=False`：生产配置默认关闭开发登录；当前没有可用的开发账号 UI 或流程入口。
- `bRememberSessionByDefault`：控制刷新令牌是否可经安全存储持久化，AccessToken 始终不落盘。

账号、密码、AccessToken、RefreshToken 与 GameTicket 均不会写入新增日志、ViewModel 或 `FDBAFrontendSessionContext`。

## 验证契约

已扩展 `DBA.Frontend.StateMachine.TransitionGuards` 的编译期自动化契约，覆盖：登录/注册/自动登录必须先到选服、选服后才加载角色列表。运行时 UI、登录与选服需按仓库人工审核策略在可见 Editor/客户端中完成；本步骤不自动操作游戏 UI 或创建角色。

## 人工资产绑定前置项

在本步骤代码完成后，使用 Unreal Editor 人工检查并保存以下资产绑定：

1. `WBP_DBA_Login` 的注册按钮应触发已有 C++ 父类 `HandleRegisterAccountClicked`。
2. `WBP_DBA_Register` 的账号、密码、确认密码、提交、返回控件应绑定同一 `UDBALoginWidgetController`，不得新增 HTTP 蓝图节点。
3. Flow 的 `ServerSelect` 状态应由既有 UI Layer Manager 激活唯一选服 Screen，不在登录 Widget 内直接创建该页面。

