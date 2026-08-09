# 05：前台唯一状态机

## 现有实现到目标实现

| 现有实现 | 目标实现 | 迁移办法 |
|---|---|---|
| `UDBAFrontendFlowSubsystem` 的 `EDBALoginFlowState` 同时表示业务和页面 | `EDBAFrontendState` 是唯一前台业务状态；FlowSubsystem 为唯一转移提交者 | 保留 `EDBALoginFlowState` 作为已存在 UMG/蓝图的只读投影 |
| `UDBAFrontendSessionSubsystem` 管理 Party/Queue/Travel | 继续作为 GameCore 对局会话子系统 | 前台摘要留在 Flow 的 `FDBAFrontendSessionContext`，避免 GameCore 反向依赖应用模块 |
| UI Manager 按旧状态显示已有 Widget | UI Manager 通过 `UDBAFrontendFlowController` 消费 Flow 状态 | Controller 订阅 `OnFrontendStateChanged`，再为既有 Widget 输出兼容视图状态 |
| 登录、选角、创建逻辑各自回退 | Flow 统一处理回退与 Guard | 所有意图调用 Flow 方法，Widget 不创建页面、不 OpenLevel |

## 状态表与 Guard

| 当前状态 | 允许目标 | 主要入口 |
|---|---|---|
| Bootstrapping | Startup、FatalError | 引擎启动协调 |
| Startup | AutoLogin、Login、FatalError | 配置决定是否尝试自动登录 |
| AutoLogin | CharacterRosterLoading、Login、RecoverableError | 自动登录失败无提示回 Login |
| Login | Register、ServerSelect、CharacterRosterLoading、RecoverableError | 登录/游客登录成功后加载角色 |
| Register | Login、RecoverableError、FatalError | 取消注册回 Login |
| ServerSelect | Login、CharacterSelect、RecoverableError | 选服只写 `ServerId` 摘要 |
| CharacterRosterLoading | CharacterSelect、CharacterCreate_Zodiac、Login、RecoverableError | 空列表进入创建首步 |
| CharacterSelect | CharacterCreate_Zodiac、ServerSelect、EnteringWorld、Login、RecoverableError | 选择已有角色后进服 |
| CharacterCreate_Zodiac | CharacterCreate_Element、CharacterSelect、RecoverableError | 生肖/外观完成 |
| CharacterCreate_Element | CharacterCreate_Zodiac、CharacterCreate_FiveCamp、CharacterSelect、RecoverableError | 元素完成 |
| CharacterCreate_FiveCamp | CharacterCreate_Element、CharacterCreate_Confirm、CharacterSelect、RecoverableError | 五营完成 |
| CharacterCreate_Confirm | CharacterCreate_FiveCamp、EnteringWorld、CharacterSelect、RecoverableError | 名称确认及服务端创建 |
| EnteringWorld | CharacterSelect、RecoverableError、FatalError | 后端分配/连接失败可返回 |
| RecoverableError | Login、ServerSelect、CharacterSelect、CharacterCreate_Zodiac、FatalError | 统一错误回退 |
| FatalError | Bootstrapping | 仅新生命周期重启 |

`DBAFrontendStateMachine::CanTransition` 是唯一转换表。`TryTransitionTo` 在任何非法跳转时拒绝并写中文日志；Flow 才可修改 `ClientSessionState`。

## SessionContext 与安全边界

`FDBAFrontendSessionContext` 仅保存 `AccountId`、`ServerId`、`SelectedCharacterId`、`ClientSessionState` 及创建草稿的 Zodiac/Element/FiveCamp。它不保存 Password、SessionToken、RefreshToken、GameTicket 或服务器连接票据。

## 回退策略

- AutoLogin 失败：`AutoLogin → Login`，不向 Widget 暴露远端错误原文。
- Logout：取消挂起请求、清空摘要与角色列表、回 `Login`。
- TokenExpired：`当前状态 → RecoverableError → Login`。
- ServerUnavailable：`当前状态 → RecoverableError → CharacterSelect`；无角色时到 `CharacterCreate_Zodiac`。
- CharacterCreateCancel：任一创建步骤清除草稿后直达 `CharacterSelect`，无循环。

## Screen 路由边界

既有 UI Manager 是唯一 Widget 容器和可见性管理者；它依据 Flow 状态显示已有 Login、CharacterSelect、CharacterCreate 或 Loading Screen。WidgetController 只能把用户意图交给 Flow。任何 Widget 禁止互相创建/销毁页面，也禁止调用 `OpenLevel`。

## 验证

`DBA.Frontend.StateMachine.TransitionGuards` 覆盖合法链路、自动登录回退、创建取消，以及三种非法跳转拒绝。该测试只验证纯状态表，不会启动登录、创建角色、网络请求或地图切换。
