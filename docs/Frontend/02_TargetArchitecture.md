# 02 目标架构：启动至角色选择/创建

## 1. 架构结论

前台只保留两张地图：`L_DBA_Boot` 负责最小启动和一次性配置读取；`L_DBA_Frontend` 是 Startup、登录、注册、选服、角色列表、角色选择与四步角色创建的唯一持久地图。上述页面只切换 Screen 与预览舞台状态，绝不各自 `OpenLevel`。只有 `Boot -> Frontend` 和 `Frontend -> Game` 能由 Travel Coordinator 发起地图切换。

现有 `UDBAGameInstance` 的“非 Frontend 世界时回到配置 FrontendMap 再启动 LoginFlow”行为可作为 Boot-to-Frontend 过渡基础；不新增第二个 GameInstance，也不改动现有地图或 Config，直到资产与状态契约完成。

## 2. 唯一前台状态机

唯一页面状态载体是保留并演进的 `UDBAFrontendFlowSubsystem`。目标状态：

`Bootstrapping -> Startup -> AutoLogin -> Login/Register -> ServerSelect -> CharacterRosterLoading -> CharacterSelect -> CharacterCreate_Zodiac -> CharacterCreate_Element -> CharacterCreate_FiveCamp -> CharacterCreate_Confirm -> EnteringWorld`

`RecoverableError` 与 `FatalError` 为异常终态或恢复入口。`EDBAFrontendStep` 只保留为旧 UI/资产 Mapper；`EDBAFrontendSessionState` 只记录 Party、Queue、ReadyCheck、Match、Travel 等跨流程会话摘要，不能决定页面跳转。现有 `EDBALoginFlowState` 是唯一迁移承载，后续必须通过显式转换表而非直接替换枚举。

## 3. 职责矩阵

| 对象 | 唯一职责 | 禁止职责 | 现有迁移来源 |
| --- | --- | --- | --- |
| `UDBAGameInstance` | 初始化、世界生命周期、取得 Subsystem、委托 Boot-to-Frontend | 保存账号、Token、角色或 UI 状态 | 现有 `UDBAGameInstance` |
| `UDBAFrontendFlowSubsystem` | 页面状态、异步请求代次、合法状态转换和错误恢复 | 直接创建 Widget、保存凭据、直接 HTTP | 现有同名 Subsystem |
| `UDBAFrontendSessionSubsystem` | 登录账号、选服、选中角色与 Travel/Party/Queue 的非敏感摘要 | 页面导航、密码/Token 持久化 | 现有同名 Subsystem |
| `UDBAUILayerManagerSubsystem` | Background/Screen/Modal/Toast/Tooltip/Debug Layer 的实例生命周期 | 账号、网络、角色权威逻辑 | `UDBAGameUIManager` 的前台部分 |
| `UDBAApiClientSubsystem` | 应用层异步 API 调用门面、超时/取消/失败归一化 | Widget 展示、业务规则、Token 展示 | `GameBackendClient` 插件 + `UDBABackendFacadeSubsystem` |
| `UDBAAuthSubsystem` | 登录、注册、自动登录意图与安全凭据协调 | 角色列表/创建、Widget 访问 | `UDBAOnlineAccountService` 的认证部分 |
| `UDBAServerDirectorySubsystem` | 服务器目录、选服结果与可用性 | 地图 Travel、角色创建 | 当前缺失；不得新建平行 ServerManager |
| `UDBACharacterRosterSubsystem` | 角色列表、刷新、选择、删除与选中角色摘要 | 名称/外观权威校验 | AccountService 的角色 CRUD 与 Flow 缓存 |
| `UDBACharacterCreateDraftSubsystem` | 四步 Draft、合法步骤推进、提交意图 | 本地决定名称唯一性、外观最终合法性 | CharacterCreate Widget/Controller 的局部状态 |
| `UDBACharacterPreviewSubsystem` | 前台预览 Actor/Stage 的异步加载、替换和释放 | GAS、Combat、Replication、服务端创建 | `ADBACharacterPresentationActor` |
| `UDBAGameSessionSubsystem` | 接收已选角色和服务端 Ticket，编排 Frontend-to-Game 请求 | 选服 UI 或角色编辑 | `UDBABackendFacadeSubsystem` 村庄分配/连接 + `UDBATravelSubsystem` |

`UDBASecureTokenStorage` 是 `UDBAAuthSubsystem` 的平台抽象依赖。其实现保存系统安全存储或受平台保护的凭据，不是 Widget、SaveGame 或 ViewModel。

## 4. UI、网络和预览边界

`WidgetBase -> WidgetController/ViewModel -> FlowSubsystem` 是唯一 UI 向业务传递意图的路径。Widget 只绑定事件和表现：不能调用 HTTP、读取 Token、访问数据库、创建另一个 Screen 或 Travel。

`FlowSubsystem -> Domain Subsystem -> ApiClient -> GameBackendClient -> HTTPS REST/JSON` 是唯一前台请求路径。外部调用必须异步，并在完成、失败、超时、取消和降级时发出中文错误事件。

预览读取 `UDBAZodiacHeroDataAsset`、`UDBAZodiacCharacterRegistry` 和外观 Draft 的软引用数据。一次只异步加载当前选中的生肖表现资源；Preview Actor 不具有 ASC/GAS、Combat、Replication 或权威角色状态。Dedicated Server 不创建 UI Layer、Preview Subsystem、Preview Actor 或 Niagara 前台展示资产。

## 5. 后端职责边界

后端以现有 `Game.Api -> Game.Application -> Game.Infrastructure -> Game.Shared` 为基础：

- AuthService 认证、刷新凭据、账号归属。
- ServerDirectory/WorldService 返回可选服务区和世界入口信息。
- CharacterService 负责角色列表、名称唯一性、生肖/元素/FiveCamp/外观合法性与持久化。
- GameSessionService 负责角色入服资格、服务器分配和 Ticket。

客户端不能从 Zodiac、Element 或 FiveCamp 推导 TeamId。FiveCamp 只作为角色创建/表现维度；旧 Faction/DivinePantheon 若在外部输入中出现，只允许 Mapper 读取后转化为 FiveCamp，不能进入运行时主流程。

## 6. 地图与 Travel 契约

| Travel | 触发者 | 允许条件 | 当前迁移来源 |
| --- | --- | --- | --- |
| `L_DBA_Boot -> L_DBA_Frontend` | `UDBAFrontendTravelCoordinator` | Bootstrap 配置完成 | `UDBAGameInstance::StartLoginFlow` 的 Frontend 回跳逻辑 |
| Frontend 内 Screen 切换 | `UDBAFrontendFlowSubsystem` 事件 | 合法状态转换 | `UDBAGameUIManager` + `UDBAFrontendFlowController` |
| `L_DBA_Frontend -> Game` | `UDBAGameSessionSubsystem` 经 Travel Coordinator | 服务端已验证 Ticket 与目标地址 | `UDBABackendFacadeSubsystem`、`UDBATravelSubsystem` |
| Game 返回 Frontend | Travel Coordinator | 会话结束或可恢复错误 | `UDBATravelSubsystem`、现有返回前台类型 |

## 7. 本步骤不实施的事项

- 不创建或移动 `L_DBA_Boot`、`L_DBA_Frontend`、Widget、DataAsset 或 Redirector。
- 不改变既有 HTTP/DTO/后端路由，不删除 Legacy Endpoint。
- 不将 `GameBackendClient`、UMG 或 Niagara 通过反向依赖引入 `GameCore`。
- 不把现有 Frontend/Lobby 同名资产任选一方删除；以 Editor Asset Registry 的实际引用为准。

## 8. 工程检查

2026-08-09 使用本机 UE 5.8 执行 `DivineBeastsArenaEditor Win64 Development` 的 Build.bat 工程检查，结果成功且 Target 已是最新状态。UBT 保留既有 `VibeUE -> StructUtils` 弃用警告；本步骤未修改插件、C++、地图或资产。
