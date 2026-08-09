# 02 唯一 Owner 与迁移矩阵

本表将已有实现映射到目标唯一 Owner。`现在`只代表当前归属，并不授权移动文件；所有二进制资产移动必须在 Unreal Editor 中逐批 Fix Redirectors、Compile、Save，并由人工审核。

| 能力 | 当前对象/目录 | 目标唯一 Owner | 目标目录 | 迁移策略 |
| --- | --- | --- | --- | --- |
| 进程/世界生命周期 | `Framework/GameInstance/UDBAGameInstance` | `UDBAGameInstance` | `Core/GameInstance` | KEEP，缩减至生命周期和 Subsystem 编排 |
| 默认通用 GameMode | `Framework/GameModes/ADBAGameModeBase` | Map-specific GameMode 基类/配置 | `Core/Framework` | ADAPT；Boot/Frontend 专用职责后续从当前大厅/对局混合职责分离 |
| 前台流程状态 | `Frontend/Flow/UDBAFrontendFlowSubsystem` | `UDBAFrontendFlowSubsystem` | `Frontend/Flow` | KEEP，成为唯一页面状态机 |
| 旧 UI 步骤 | `Core/DBAEnumsCore.h::EDBAFrontendStep` | Legacy Flow Mapper | `Frontend/Flow/Legacy` | DEPRECATE；不再新增状态 |
| 会话摘要 | `GameCore/Session/UDBAFrontendSessionSubsystem` | `UDBAFrontendSessionSubsystem` | `Frontend/Session` 接口，通用类型仍在 `GameCore` | ADAPT，不承担页面路由 |
| Travel 执行 | `GameCore/Networking/Travel/UDBATravelSubsystem` | `UDBAFrontendTravelCoordinator` 调用底层 Travel | `Frontend/Flow` + `Online/Session` | ADAPT，Widget 不可直调 |
| UI 实例管理 | `UI/Controllers/UDBAGameUIManager` | `UDBAUILayerManagerSubsystem` | `Frontend/UI/Layers` | MERGE，分层迁移，不平行保留两套导航 |
| UI 流程适配 | `UI/Frontend/UDBAFrontendFlowController` | Flow-to-UI Adapter | `Frontend/UI/Controllers` | ADAPT，后续由 ViewModel 替换其状态镜像 |
| Widget 类注册 | `Data/Registries/UDBAUIFlowRegistry` | `UDBAUIFlowRegistry` | `Frontend/UI/Data` | KEEP，始终软引用 Widget 类 |
| 登录/注册 | `Frontend/Account/UDBAOnlineAccountService` | `UDBAAuthSubsystem` | `Online/Auth` | ADAPT，复用服务实现与异步回调 |
| Token | AccountService 字段 + `UDBAAccountSaveGame` | `UDBASecureTokenStorage` | `Online/Auth/SecureStorage` | DEPRECATE 明文 SaveGame Token，迁移后 DELETE-LATER |
| HTTP 传输 | `Plugins/GameBackendClient` | `GameBackendClient` | 插件保持原目录 | KEEP，插件不依赖应用 UI |
| 应用 API 门面 | `Frontend/Backend/UDBABackendFacadeSubsystem` | `UDBAApiClientSubsystem`/Facade | `Online/Http` | ADAPT，保持唯一应用边界 |
| 服务器目录 | 无独立对象 | `UDBAServerDirectorySubsystem` | `Online/ServerDirectory` | NEW-LATER，只在现有配置/API 契约确定后提取 |
| 角色列表 | AccountService CRUD + Flow CachedCharacters | `UDBACharacterRosterSubsystem` | `Online/Character/Roster` | MERGE，保留现有 DTO 和后端调用 |
| 创建草稿 | Create Widget/Controller 状态 | `UDBACharacterCreateDraftSubsystem` | `Online/Character/Create` | MERGE，固定四步，服务端仍最终校验 |
| 角色 DTO/Mapper | `GameCore/Networking/Account/DBAAccountTypes` + `DBAOnlineAccountJson` | Character Domain DTO/Mapper | `Online/Character/Contracts` | ADAPT，不复制 DTO |
| 选择/创建 Screen | `Frontend/CharacterSelection/*`、`UI/Widgets/Lobby/*` | Character Screen/Controller/ViewModel | `Frontend/UI/Screens/Character` | MERGE，二进制资产先 UNKNOWN |
| 登录 Screen | `Frontend/Auth/*` | Auth Screen/Controller/ViewModel | `Frontend/UI/Screens/Auth` | ADAPT |
| 选服 Screen | 无独立 Screen | Server Directory Screen/VM | `Frontend/UI/Screens/ServerDirectory` | NEW-LATER |
| 生肖数据 | `Data/Assets/UDBAZodiacHeroDataAsset` | 同名 DataAsset | `Character/Data/Zodiac` | KEEP，禁止竞争资产类型 |
| 生肖类注册 | `Data/Registries/UDBAZodiacCharacterRegistry` | 同名 Registry | `Character/Data/Zodiac` | KEEP，软引用与异步加载 |
| 外观 Draft | 分散于 Widget | `FDBACharacterAppearance` + Draft Subsystem | `Character/Appearance` | NEW-LATER，数据驱动可选项 |
| 前台预览 | `Frontend/CharacterSelection/ADBACharacterPresentationActor` | `UDBACharacterPreviewSubsystem` + Stage Actor | `Frontend/Preview` | ADAPT，先验证 Server 排除和资源软引用 |
| 对局角色与 GAS | `Characters/*`、`Gameplay/*` | Gameplay Owner | `Gameplay/*` | KEEP，禁止 Preview 依赖它们 |
| 后端角色 API | `Game.Api/Endpoints/Account`、Application Characters | CharacterService | 现有后端分层 | KEEP，Legacy routes 先 ADAPT |
| 世界会话 API | Session Endpoints/Services | GameSession/WorldService | 现有后端分层 | KEEP |

## 二进制资产 Owner

| 当前资产域 | 目标域 | 状态 |
| --- | --- | --- |
| `/Game/Maps/Lobby/FrontendMap` | `/Game/DBA/Maps/Frontend/L_DBA_Frontend` | UNKNOWN；先确认 Map 引用和迁移兼容策略 |
| 尚无 Boot Map | `/Game/DBA/Maps/Frontend/L_DBA_Boot` | NEW-LATER；只能通过 Unreal Editor 创建和保存 |
| `/Game/DBA/UI/Frontend/Character/*` | `/Game/DBA/UI/Frontend/Character/*` | UNKNOWN；候选权威资产域 |
| `/Game/DBA/UI/Lobby/Character/*` | `/Game/DBA/UI/Frontend/Character/*` 或 Legacy Redirector | UNKNOWN；引用审计后决定 |
| `/Game/DBA/Data/Registries/*` | `/Game/DBA/Data/Zodiac` 与 `/Game/DBA/UI/Frontend/Data` | ADAPT；按 DataAsset 类型逐批移动，当前不动 |
| `/Game/Models/Zodiac/*` 与 `/Game/DBA/Zodiacs/Chinese/*` | `/Game/DBA/Characters/Zodiac/*` | UNKNOWN；必须先确认实际引用与骨架兼容 |
| `/Game/DBA/FiveCamps/*` | `/Game/DBA/Data/Zodiac/FiveCamp` | ADAPT；五营是数据而非 TeamId 来源 |

## 不可删除对象

第 01 步列出的所有地图、Flow、UI Registry、Account/Backend 服务、生肖 Registry、双域选创角 WBP、Preview Actor 与后端 Endpoint/迁移仍属不可删除。本步骤没有将任何对象标记为可立即删除。

