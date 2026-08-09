# 步骤29：最终对象清单

## 客户端权威对象

| 链路阶段/能力 | 唯一 Owner | 关键消费方 | 最终状态 |
| --- | --- | --- | --- |
| 引擎启动协调 | `UDBAGameInstance` | Engine | KEEP；不承载账号/角色权威业务 |
| 启动检查与 Frontend 协调 | `UDBAStartupCoordinatorSubsystem` | GameInstance/Flow | KEEP |
| 前台状态机 | `UDBAFrontendFlowSubsystem` + `EDBAFrontendState` | 所有 Screen Controller | KEEP，唯一状态提交者 |
| 前台会话摘要 | Flow 内 `FDBAFrontendSessionContext` | Flow/Session/Roster | KEEP；不保存密码、Token、Ticket |
| UI 根与分层 | `UDBAUILayerManagerSubsystem` | Flow/UI Controller | KEEP，唯一 Viewport Root Owner |
| HTTP/JSON 门面 | `UDBAApiClientSubsystem` | Auth/Server/Roster/GameSession | KEEP，唯一前台 API 入口 |
| HTTP 传输 | `FDBA_GameBackendHttpClient` | ApiClient | KEEP，唯一底层传输 |
| Token 抽象 | `UDBASecureTokenStorage` | ApiClient/Auth | KEEP；开发实现不代表发布平台安全存储 |
| 账号认证领域适配 | `UDBAOnlineAccountService` | Login Controller/Flow | ADAPT；新代码不得调用其旧角色 CRUD |
| 登录 UI 编排 | `UDBALoginWidgetController` + `UDBALoginViewModel` | Login/Register Widget | KEEP |
| 区服目录 | `UDBAServerDirectorySubsystem` | ServerSelect Controller | KEEP |
| 区服 UI 编排 | `UDBAServerSelectWidgetController` + `UDBAServerSelectViewModel` | ServerSelect Widget | KEEP |
| 角色列表与缓存 | `UDBACharacterRosterSubsystem` | CharacterSelect/Confirm/Flow | KEEP，缓存键为 AccountId+ServerId |
| 角色选择 UI 编排 | `UDBACharacterSelectWidgetController` + `UDBACharacterSelectViewModel` | CharacterSelect Widget | KEEP |
| 创建草稿 | `UDBACharacterCreateDraftSubsystem` | 四步创建 Controller/Flow | KEEP |
| 创建 UI 编排 | `UDBACharacterCreateWidgetController` + Step ViewModels | 四步创建 Widget | KEEP |
| 生肖静态配置 | `UDBAZodiacHeroDataAsset` | Registry/Preview/UI | KEEP，唯一生肖 Primary Asset 类型 |
| 生肖注册表 | `UDBAZodiacRegistrySubsystem` | Create/Select/Preview | KEEP，按需异步加载 |
| 外观 | `FDBACharacterAppearance` + `UDBACharacterAppearanceComponent` | Preview/Gameplay Character | KEEP，数据库只存稳定 ID |
| 3D 预览 | `UDBACharacterPreviewSubsystem` | Select/Create Controller | KEEP，generation 防乱序 |
| 预览舞台 | `ADBACharacterPreviewStage` | PreviewSubsystem | KEEP |
| 预览角色 | `ADBACharacterPreviewActor` | PreviewStage | KEEP；不含 GAS/AI/Replication |
| 预览相机 | `ADBACharacterPreviewCameraRig` | PreviewStage | KEEP |
| EnterWorld | `UDBAGameSessionSubsystem` | Flow | KEEP |
| ClientTravel | `UDBAFrontendTravelCoordinator` | GameSession/Flow | KEEP，Widget 不拼地址/Travel |

## 兼容对象

| 对象 | 当前引用证据 | 替代对象 | 状态 |
| --- | --- | --- | --- |
| `EDBALoginFlowState` | 旧 Widget/UIManager/大厅源码仍使用 | `EDBAFrontendState` | DEPRECATED，只读投影 |
| `EDBAFrontendStep` | 源码和资产字符串未发现消费方 | `EDBAFrontendState` | DEPRECATED；人工 E2E 前不删除 UENUM |
| `UDBAFrontendFlowController` | 旧 FlowWidget/UIManager 源码仍使用 | 页面专属 WidgetController | DEPRECATED ADAPTER |
| `UDBAGameUIManager` 的登录 Flow 入口 | GameInstance、Startup、Splash 与旧 UI 使用 | `UDBAUILayerManagerSubsystem` | 部分 API DEPRECATED；Manager 本体 KEEP |
| `ADBACharacterPresentationActor` | GameMode、旧 FlowWidget 与 Preview fallback 使用 | PreviewStage/Actor | ADAPT；不可删除 |
| `UDBAZodiacDataAsset` | 未发现内容资产字符串命中 | `UDBAZodiacHeroDataAsset` | 已标 DeprecatedNode；人工资产审计前保留 |
| `EDBAZodiacType/EDBAElementType/EDBAFiveCampType` | Adapter 与旧数据定义使用 | canonical 三枚举 | DEPRECATED 迁移类型 |
| 旧 Village allocation/connection 私有链 | Flow 内部兼容字段/回调仍存在 | GameSession + TravelCoordinator | DEPRECATE-LATER |

## 后端权威对象

| 能力 | 唯一 Owner | 传输契约 | 状态 |
| --- | --- | --- | --- |
| 注册/登录/刷新/登出 | `AuthService` | `/api/v1/auth/*` | KEEP |
| 首次玩家名 | `PlayerService` | `/api/v1/auth/player-name/generate`；UE 注册编排显式调用 | KEEP |
| 区服目录 | `ServerDirectoryService` | `/api/v1/servers` | KEEP |
| 角色 CRUD/选择 | CharacterService 应用层与 `CharacterEndpoints` | `/api/v1/characters` | KEEP，唯一新客户端入口 |
| EnterWorld | `GameEnterService` | `/api/v1/game/enter` | KEEP |
| 一次性 Ticket 索引 | `GameTicketRedisRegistry` + PostgreSQL JoinTicket | DS consume | KEEP |
| 数据库上下文 | `GameDbContext` | EF Core migrations | KEEP，唯一 DbContext |
| 旧账号角色接口 | `AccountEndpoints` | `/api/account/*`、`/api/players/me/characters` | DEPRECATED ADAPTER |

## 内容资产不可删除清单

- `/Game/DBA/Data/Registries/DA_DBA_UIFlowRegistry`
- `/Game/DBA/UI/Lobby/Character/WBP_DBA_CharacterSelect`
- `/Game/DBA/UI/Lobby/Character/WBP_DBA_CharacterCreate`
- `/Game/DBA/UI/Frontend/Character/WBP_DBA_CharacterSelect`
- `/Game/DBA/UI/Frontend/Character/WBP_DBA_CharacterCreate`
- `/Game/Maps/Lobby/FrontendMap`
- `/Game/Maps/Lobby/LobbyMap`
- Zodiac Registry、十二生肖 DataAsset、Appearance Catalog 与 Preview 表现资产

前两项 Lobby Character Widget 被 `DA_DBA_UIFlowRegistry` 明确指向；Frontend 同名资产尚未取得 Editor Reference Viewer 的无引用证明。以上资产不得通过文件系统直接删除或移动。
