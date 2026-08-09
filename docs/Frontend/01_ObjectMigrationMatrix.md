# 01 对象迁移矩阵

本矩阵定义本步骤后的唯一迁移方向。标记含义：`KEEP` 继续作为权威实现；`MERGE` 并入唯一入口；`ADAPT` 以兼容适配层接入；`DEPRECATE` 停止新增依赖；`DELETE-LATER` 仅在资产/配置引用清零后移除；`UNKNOWN` 需要 Editor Asset Registry 或运行人工审核确认。

| 现有对象或能力 | 现状 | 标记 | 唯一目标 | 迁移办法 |
| --- | --- | --- | --- | --- |
| `UDBAFrontendFlowSubsystem` + `EDBALoginFlowState` | 登录至村庄/对局都在一个状态机 | MERGE | `UDBAFrontendFlowSubsystem` 的唯一前台状态机 | 保留类型和委托，先加入目标状态转换契约；前台外的 Party/Queue/Arena 状态逐步移至专属服务 |
| `EDBAFrontendStep` | UI 前台步骤枚举 | DEPRECATE | `EDBALoginFlowState` | 后续只作旧 UI/资产输入 Mapper；不再加入新状态 |
| `EDBAFrontendSessionState` | Party/Queue/Match 会话状态 | ADAPT | `UDBAFrontendSessionSubsystem` 会话摘要 | 保留非页面会话语义；禁止作为 Screen 路由来源 |
| `UDBAFrontendFlowController` | Flow 的 UObject 包装器 | ADAPT | Flow 到 UI Layer 的事件适配 | 保留现有 Widget 兼容入口；新 Screen 只消费统一 Flow 事件 |
| `UDBAGameUIManager` | 解析注册表、创建/显示多类 Widget | MERGE | `UDBAUILayerManagerSubsystem` | 先委托/包裹现有实例管理，再逐 Screen 迁移；完成前不可删除 |
| `UDBAUIFlowRegistry` | Widget 软类路径唯一配置 | KEEP | 同名 DataAsset 注册表 | 扩展目标 Screen 字段，禁止重新写 C++ 硬路径 |
| `UDBAAccountServiceBase` | 登录、注册、角色 CRUD、SaveGame、Token | ADAPT | Auth + Roster + Secure Token 分域服务 | 保留基类作为兼容表面；将新业务从角色管理和凭据持久化职责中分出 |
| `UDBAOnlineAccountService` | 后端账号和角色接口的具体实现 | ADAPT | `UDBAAuthSubsystem` 的过渡实现 | 不新增平行 AuthManager；先通过 Facade/Adapter 维护异步契约 |
| `UDBAAccountSaveGame` | 账户、角色缓存和 Token 持久化 | DEPRECATE | `UDBASecureTokenStorage` + 非敏感本地偏好 | 先做安全迁移与兼容读取；Token 数据清零并完成人工审核后才能 DELETE-LATER |
| `UDBAFrontendSessionSubsystem` | Party、Queue、ReadyCheck、Travel | KEEP | 前台会话摘要 | 新增账号、选服、选中角色时保持摘要化，不保存密码或 Token |
| `UDBATravelSubsystem` | ClientTravel 基础能力 | KEEP | `UDBAFrontendTravelCoordinator` 的底层执行器 | 新协调器只能调用其合法 Travel；前台屏幕不可直调 |
| `UDBABackendFacadeSubsystem` | 应用层到插件的边界 | KEEP | 同名唯一应用 Facade | 添加领域调用前先定义 DTO/Mapper，禁止 Widget 访问 `GameBackendClient` |
| `UDBACharacterSelectWidgetController` | 角色选择 UI 意图 | ADAPT | Character Select Controller/ViewModel | 保留并改为调用 Roster 状态，不直持列表权威状态 |
| `UDBACharacterCreateWidgetController` + `UDBACharacterCreateFlowWidgetBase` | 单 Widget 收集创建字段和本地校验 | MERGE | Draft Subsystem + Create ViewModel + 四步 Screen | 以现有请求 DTO 为提交出口；逐步移出 Widget 内状态与权威校验 |
| `ADBACharacterPresentationActor` | 世界放置预览舞台 | ADAPT | Preview Stage/Actor 兼容实现 | 先做 Server 排除、软引用与无 GAS/Replication 审计，确认后再拆 Subsystem |
| `UDBAZodiacHeroDataAsset` | 生肖展示数据 | KEEP | 同名唯一生肖数据资产 | 扩展允许的 Appearance 选项，禁止新建竞争 Archetype 资产类型 |
| `UDBAZodiacCharacterRegistry` | 生肖到角色类映射 | KEEP | 同名唯一 Registry | 保持软引用和异步加载；不以 Zodiac/FiveCamp 推导 TeamId |
| `/Game/DBA/UI/Frontend/Character/*` | Frontend 选创角资产域 | UNKNOWN | 候选权威资产域 | 通过 Editor 引用审计确认 Registry 指向前不得移动 |
| `/Game/DBA/UI/Lobby/Character/*` | Lobby 选创角资产域 | UNKNOWN | Legacy 或候选权威资产域 | 同上；如无引用，先转 Redirector 再 DELETE-LATER |
| `UDBA_GameBackendAuthService`、`UDBA_GameBackendSessionService` | HTTP 服务封装 | KEEP | `GameBackendClient` 插件域 | 不让 `GameCore` 或 Widget 反向依赖应用实现 |
| 后端 `AuthService`、`AccountEndpoints` | 登录/账户 API | KEEP | AuthService 领域 | 角色创建服务与账户认证保持 DTO 边界 |
| 后端 `PlayerCharacterUseCases`、`CharacterBuildPolicy` | 角色创建与构建规则 | KEEP | CharacterService 领域 | 服务端继续验证名称、外观、生肖/元素/FiveCamp 合法性 |
| 后端 `SessionService`、`VillageAllocationService` | 进入世界与会话分配 | KEEP | GameSession/WorldService 领域 | 仅接受服务端签发 Ticket，客户端不自行推导 |

## 重复能力表

| 能力 | 当前实现 | 结论 |
| --- | --- | --- |
| 账号/认证 | `UDBAAccountServiceBase` + `UDBAOnlineAccountService` + 插件 AuthService | 基类/具体实现/传输层是合理分层，不新增 `AuthManager`；但应把 Token 存储从 AccountSaveGame 分出 |
| 角色列表与创建 | AccountService 中 CRUD + Flow 缓存 + 两个 WidgetController | 暂无 `CharacterRosterSubsystem`；以现有 AccountService/Flow 为兼容输入，后续提炼唯一 Roster 和 Draft |
| 前台状态 | `EDBALoginFlowState`、`EDBAFrontendStep`、`EDBAFrontendSessionState` | 重叠；目标分别为页面状态、旧 UI Mapper、会话摘要 |
| UI 导航 | `UDBAGameUIManager` + `UDBAFrontendFlowController` + Widget 自身回调 | 目标是 Flow 驱动 UI Layer；禁止 Widget A 创建 Widget B |
| 角色预览 | `ADBACharacterPresentationActor` + Frontend/Lobby 两域 WBP + 多生肖资源域 | 资产引用未知，严禁删除；目标为单舞台和单资产域 |
| 生肖数据 | `UDBAZodiacHeroDataAsset`、`UDBAZodiacCharacterRegistry`、CharacterSelection DataAsset | 职责不同，不合并类型；统一由 Registry/Selection 数据驱动 UI 和预览 |

## 退出门槛

下一阶段不得新建同义 Manager、Subsystem、DTO 或资产类型。只有先建立状态转换表、确认 UI 注册表的二进制资产引用、并设计 SecureTokenStorage 迁移，才能修改前台运行时逻辑。

