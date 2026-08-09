# 04：前台核心契约

## 本步骤范围

本步骤为“启动 → 登录 → 选服 → 角色列表 → 选择/创建 → 进入游戏请求”建立统一基础契约，不改写既有 UI 或后端业务。固定显示名恢复为 **神兽竞技场**；工程名、模块名和协议命名仍为 `DivineBeastsArena` / `DBA`。

## 唯一新业务入口

| 能力 | 权威对象 | 旧实现处理 | 新业务规则 |
|---|---|---|---|
| 生肖、元素、五营 | `EDBAZodiac`、`EDBAElement`、`EDBAFiveCamp`（GameCore） | `EDBA*Type` 通过 `DBAIdentityTypeAdapter` 映射 | 创建草稿和服务 DTO 使用规范枚举；`TeamId` 不在本契约中，绝不能由三者推导 |
| 前台流程状态 | `EDBAFrontendState` | `EDBALoginFlowState` 仍服务已存在蓝图；`GetFrontendState()` 是新 Screen 入口 | 新 UI 不新增流程状态枚举 |
| 异步结果 | `FDBAOperationResult` + `EDBAAsyncOperationState` | 原有 `EDBAErrorCode`、`ErrorMessage` 保持兼容 | 新业务读取 `OperationState` 与 `ApiError` |
| API 错误 | `FDBAApiError` + `UDBAFrontendErrorMapper` | 旧字符串 `OnFlowError` 保留 | Widget 订阅 `OnFlowApiError`/`OnLoginApiError`，基于 `ErrorCode`、`Category`、`UserMessage` 展示 |
| 前台会话数据 | `FDBAFrontendSessionContext` | 既有 `UDBAFrontendSessionSubsystem` 继续维护组队/匹配上下文 | 仅存业务 ID 与创建草稿；绝不存凭据 |
| 前台地图/槽位 | `UDBAFrontendSettings` | `DBAFrontendConfig` 已标注废弃，暂留未知资产兼容 | 通过软对象路径读取 Boot/Frontend Map 与最大角色槽位 |
| 外部服务 | `UDBAExternalServiceSettings`（GameBackendClient 插件） | 原 `UDBA_GameBackendClientSettings` 已由 Core Redirect 迁移 | Gateway URL、API Version、HTTP Timeout 从此设置读取 |

## 状态与错误规则

`EDBAFrontendState` 覆盖启动、认证、角色加载、角色选择/创建、大厅连接、对局连接及可恢复/不可恢复错误。旧登录流状态值与其保持同序，仅作为资产迁移期适配，任何新增 C++/Blueprint 逻辑不可引用旧枚举。

`FDBAApiError` 是唯一允许跨 Controller → ViewModel/Widget 的网络错误模型：

- `ErrorCode`：稳定机器码，如 `character.name_taken`；
- `Category`：网络或业务错误分类；
- `UserMessage`：错误映射层提供的中文可显示文案；
- `HttpStatusCode` 和 `bCanRetry`：行为决策信息。

结构体刻意不包含服务端响应原文、Password、Token、RefreshToken 或 GameTicket。`FromLegacyMessage` 仅为现有字符串回调生成安全通用文案；后续 API 客户端必须调用 `FromHttpStatus` 并传入后端业务码。

## 配置归属

```ini
[/Script/DivineBeastsArena.DBAFrontendSettings]
BootMap=/Game/Maps/Lobby/FrontendMap.FrontendMap
FrontendMap=/Game/Maps/Lobby/FrontendMap.FrontendMap
MaxCharacterSlots=3

[/Script/GameBackendClient.DBAExternalServiceSettings]
GatewayBaseUrl="http://localhost:8080"
ApiVersion=v1
RequestTimeoutSeconds=15
```

地图字段使用软引用。当前尚未创建 `L_DBA_Boot`，所以 BootMap 临时指向已存在前台图；实际地图到位时只修改配置，不在 C++ 中硬编码新地图名。`DBAFrontendConfig` 的地图项不再由 `UDBAGameInstance` 读取。

## 日志与敏感数据

统一日志域为 `LogDBAFrontend`、`LogDBAOnline`、`LogDBACharacter`、`LogDBAPreview`。所有新增日志保持中文，并禁止输出 Password、Token、RefreshToken、GameTicket；网络响应原文也不得作为 UI 或日志默认内容。

## 迁移顺序

1. API Client 把 HTTP 状态和后端 ErrorCode 映射为 `FDBAApiError`。
2. WidgetController/ViewModel 改订阅结构化事件；旧 `OnFlowError` 最后移除。
3. `EDBALoginFlowState` 和 `EDBAFrontendSessionState` 分别按 Flow/对局会话边界合并到规范状态，处理并保存蓝图重定向。
4. 待 `L_DBA_Boot` 与持久 `L_DBA_Frontend` 资产创建并人工审核后，删除 `DBAFrontendConfig` 中的旧地图键。
