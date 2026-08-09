# 01 当前状态审计：启动至角色选择/创建

## 审计范围与结论

本审计覆盖 `DivineBeastsArena.uproject`、已启用插件、三个 UE 模块及其 Build/Target、Config、相关 C++ 类型、可按文件名识别的二进制资产、后端解决方案、Docker 与数据库迁移。审计日期为 2026-08-09。

当前客户端有可复用的登录、角色列表、创建、选择、前台 UI 注册表和后端 HTTP 边界；但尚未收敛成目标中的“Boot -> 持久 Frontend -> Screen 状态机”。本步骤不修改业务、Config、地图或二进制资产，只记录唯一迁移基线。

## 当前启动及角色链

1. `DefaultEngine.ini` 将客户端默认地图设为 `/Game/Maps/Lobby/FrontendMap`，服务器默认地图为 `/Game/Maps/Lobby/LobbyMap`；全局 GameMode 是 `ADBAGameModeBase`，GameInstance 是 `UDBAGameInstance`。
2. `UDBAGameInstance::StartLoginFlow` 启动 `UDBAFrontendFlowSubsystem`；启动视频 Widget 也是现有入口之一。
3. `UDBAFrontendFlowSubsystem` 通过 `UDBAAccountServiceBase::Resolve` 取得 `UDBAOnlineAccountService`，处理登录、游客登录、角色列表、选角、创角和村庄连接。
4. `UDBAOnlineAccountService` 使用 `GameBackendClient` 插件异步调用后端；`UDBABackendFacadeSubsystem` 是应用层用于同步认证上下文和村庄会话的边界。
5. `UDBAGameUIManager` 读取 `UDBAUIFlowRegistry`（DeveloperSettings 软引用），异步创建 Login、CharacterSelect、CharacterCreate 等 Widget，并持有 `UDBAFrontendFlowController` 订阅前台流程状态。
6. 角色创建当前由 `UDBACharacterCreateFlowWidgetBase` 和 `UDBACharacterCreateWidgetController` 收集名称、生肖、元素与 FiveCamp 后提交 `FDBACharacterCreateRequest`；尚未表达为固定四个独立 Screen 状态。
7. 角色选择/创建表现使用已放置在世界中的 `ADBACharacterPresentationActor`。它是前台舞台 Actor，不是轻量 Preview Subsystem，且其 StageSpec 仍含多项硬编码表现数值。

## 审计对象

| 域 | 已发现对象 | 当前状态 | 迁移标记 |
| --- | --- | --- | --- |
| 生命周期 | `UDBAGameInstance`、`ADBAGameModeBase`、`ADBALobbyPlayerController`、`UDBAGameUIManager` | 默认直接进 `FrontendMap`；GameMode 同时有大厅、对局后端心跳与 Pawn 解析职责 | KEEP / ADAPT |
| 状态与路由 | `UDBAFrontendFlowSubsystem`、`EDBALoginFlowState`、`UDBAFrontendFlowController`、`EDBAFrontendStep`、`UDBAFrontendSessionSubsystem`、`EDBAFrontendSessionState` | 三套状态概念并存 | MERGE / ADAPT |
| 账号和 Token | `UDBAAccountServiceBase`、`UDBAOnlineAccountService`、`UDBAAccountSaveGame`、`UDBAProfileSaveGame` | 服务基类同时含登录、角色 CRUD、本地 SaveGame 和 Token 内存字段；SaveGame 明文成员含 Session/Refresh Token | ADAPT，安全风险需优先消除 |
| 后端通信 | `UDBA_GameBackendClientSubsystem`、`UDBA_GameBackendAuthService`、`UDBA_GameBackendSessionService`、`UDBABackendFacadeSubsystem` | 插件实现 HTTP/JSON；应用层有 Facade，Widget 未直接依赖插件 | KEEP / ADAPT |
| 角色 | `FDBACharacterSummary`、`FDBACharacterCreateRequest`、`UDBACharacterSelectWidgetController`、`UDBACharacterCreateWidgetController` | Roster 与 Create Draft 尚未独立为 Subsystem | ADAPT |
| UI | `UDBAUIFlowRegistry`、`UDBAFrontendFlowController`、Login/Character Select/Create WidgetBase 和 Controller | Registry 使用软类引用；UI Manager 仍为集中创建/显隐入口；未发现 C++ MVVM ViewModel | KEEP / MERGE |
| 生肖与构建 | `UDBAZodiacHeroDataAsset`、`UDBAZodiacCharacterRegistry`、`DBAZodiacCharacterRegistry` DataAsset、`EDBAZodiac`、`EDBAElement`、`EDBAFiveCamp` | 生肖注册表和 UI 选择数据均存在；FiveCamp 已为代码主词汇 | KEEP |
| 角色预览 | `ADBACharacterPresentationActor`、`DBAZodiacVisualDeveloperSettings` | 世界放置舞台、Camera、Mesh、灯光和后处理都在一个 Actor；未发现 SceneCapture/RenderTarget Preview 实现 | ADAPT |
| 资产 | `/Game/DBA/UI/Frontend/Character/*` 与 `/Game/DBA/UI/Lobby/Character/*` | 两个路径存在同名 `WBP_DBA_CharacterSelect` / `WBP_DBA_CharacterCreate` | UNKNOWN，均不可删除 |
| 后端 | Auth、Account、Character、Session、GameServer Endpoints；Application/Infrastructure/Shared 分层；EF migrations 与 Docker Compose | Postgres 表含 `account`、`player_character`、`refresh_token`、`game_session`、`game_server_instance`；Redis 配置应在后续部署审计确认 | KEEP |

## 已检索的同义与 Legacy 词

- 源码运行主链未检出 `Faction`、`FactionSelect`、`DivinePantheon` 或 `Pantheon`；新增业务不得引入这些词。
- 检出旧/并行资源域：`Models/Zodiac`、`DBA/Zodiacs/Chinese`、`DBA/Zodiacs/Western`、`DBA/UI/Frontend`、`DBA/UI/Lobby`。文件名检索不能证明资产引用关系，均列入不可删除项。
- 未检出 C++ `ViewModel` 实现；CommonUI 已启用，MVVM 模块未列入现有 Build.cs 依赖。
- 未检出 C++ `SceneCapture` 或 `RenderTarget` 预览对象。
- 未检出 UE 5.7 版本声明；引擎关联指向本机已安装的 UE 5.8 源码目录。

## 后端契约概览

UE 仅应经 `GameBackendClient` 插件与 `UDBABackendFacadeSubsystem` / `UDBAOnlineAccountService` 访问 REST/JSON。后端由 `Game.Api -> Game.Application -> Game.Infrastructure -> Game.Shared` 组成，API Endpoints 分为 Auth、Account、Session、GameServer 与 Admin。数据库权威对象已覆盖账户、角色、刷新凭据、会话和服务器实例。

本阶段不能将 Token、角色名唯一性、外观合法性或入服 Ticket 判为客户端权威；客户端只保留异步意图和 UI 可消费的结果事件。

## 不可删除清单

以下对象存在 Config、源码、注册表或二进制资产引用路径，未完成 Editor Asset Registry 审核前不得删除或移动：

- `/Game/Maps/Lobby/FrontendMap`、`/Game/Maps/Lobby/LobbyMap`。
- `UDBAGameInstance`、`ADBAGameModeBase`、`ADBALobbyPlayerController`、`UDBAGameUIManager`。
- `UDBAFrontendFlowSubsystem`、`UDBAFrontendFlowController`、`UDBAOnlineAccountService`、`UDBABackendFacadeSubsystem`。
- `UDBAUIFlowRegistry`、`DA_DBA_UIFlowRegistry`、`UDBAZodiacCharacterRegistry`、`DA_DBA_ZodiacCharacterRegistry`、`DA_DBA_ZodiacCharacterSelection`。
- `UDBACharacterSelectFlowWidgetBase`、`UDBACharacterCreateFlowWidgetBase` 及其两个 Frontend/Lobby 资产路径的同名 WBP。
- `ADBACharacterPresentationActor`、`DBAZodiacVisualDeveloperSettings`、十二生肖和 FiveCamp 数据资产。
- 后端 Account/Auth/Character/Session/GameServer Endpoint、契约、迁移和 Docker Compose 文件。

## 本步骤未修改的风险

- `UDBAAccountSaveGame` 有明文 Token 字段，不符合目标的 `UDBASecureTokenStorage` 抽象；该变更需要平台存储设计，不能在审计阶段直接删除字段。
- `UDBAAccountServiceBase` 包含 `GenerateGuestAccountId`、`GenerateCharacterId` 和本地角色 CRUD 兜底，和“服务器权威”要求冲突，需要后续受控迁移。
- `ADBACharacterPresentationActor` 的预览资源路径和舞台数值部分硬编码，且未见 Server 限制的明确审计结论。
- `ADBALobbyPlayerController::PlayerTick` 存在；其具体用途需在输入/前台分离步骤检查，不能仅凭存在判为 UI 轮询。
- 根文档说明与当前仓库仍包含 load-test JavaScript；该脚本不属于本步骤启动到创角主链，未删除。

## 工程检查结果

2026-08-09 使用本机 `D:/UnrealEngine-5.8.0-release` 执行：

`Engine/Build/BatchFiles/Build.bat DivineBeastsArenaEditor Win64 Development DBA_GameClient/DivineBeastsArena.uproject -WaitMutex -NoHotReload`

结果为成功，Target 已是最新状态，未发现由本步骤引入的 C++ 编译错误。UBT 给出既有警告：项目通过 `VibeUE` 依赖已在 UE 5.5 标为弃用的 `StructUtils` 插件；本步骤没有修改该插件依赖。
