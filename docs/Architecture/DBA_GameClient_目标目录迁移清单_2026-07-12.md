# DBA_GameClient 目标目录迁移清单

## 1. 目的与边界

本清单将 `DBA_GameClient` 逐步收敛到 MOBA+RPG 的目标目录结构。迁移遵循以下边界：

- 不直接编辑、移动或重命名 `.uasset`、`.umap`；正式资产迁移必须通过 Unreal Editor/MCP 事务，并执行 Compile、Save 与人工审核。
- 保留 `/Game/Maps`；地图仅按 `Frontend`、`Lobby`、`Arena`、`Training` 的目标分类逐批迁移，不因目录整理批量移动现有地图。
- 保留 `/Game/DBA/Zodiacs/Chinese` 作为十二生肖正式资源域，不回退到旧 `Models` 或 `Animation` 域。
- 代码逻辑保持 C++ 实现，配置和资源引用保持 DataAsset/DeveloperSettings 驱动；不得为迁移新增 Blueprint 业务逻辑或硬编码资源路径。
- 每批只处理可独立编译、可人工审核的目录；未完成的协议、资产引用和后端契约不得强行迁移。

## 2. 已完成：第一批源码目录迁移

| 原路径 | 新路径 | 状态 | 说明 |
| --- | --- | --- | --- |
| `GameDBA/Character` | `GameDBA/Characters` | 已完成 | 统一为目标结构的复数领域名；类名与模块名不变。 |
| `GameDBA/Animation` | `GameDBA/Presentation/Animation` | 已完成 | 表现层动画实现归位。 |
| `GameDBA/VFX` | `GameDBA/Presentation/VFX` | 已完成 | 表现层特效实现归位。 |
| `GameDBA/Framework/DBAGameModeBase.*` | `GameDBA/Framework/GameModes/` | 已完成 | GameMode 子域独立。 |
| `GameDBA/Framework/DBAReplicationGraph.*` | `GameDBA/Framework/Replication/` | 已完成 | 复制子域独立。 |
| `GameDBA/Framework/DBAUrlOptions.*` | `GameDBA/Framework/Travel/` | 已完成 | Travel URL 传输契约归位。 |
| `GameMoba/RPC` | `GameMoba/Networking/RPC` | 已完成 | 通用网络 RPC 归位。 |
| `GameMoba/Framework/DBAMobaGameModeBase.*` | `GameMoba/Combat/` | 已完成 | 通用对局基类归入战斗层。 |
| 中性目标锁定与死亡状态枚举 | `GameMoba/Targeting/DBACombatTypes.h` | 已完成 | 不依赖生肖、资源或 Arena 数据资产，可供多个玩法域复用。 |
| `GameCore/Data/DBAAsyncDataTableLoader.*` | `GameCore/Async/` | 已完成 | 异步数据表加载器归入异步基础设施。 |
| `GameCore/ObjectPool` | `GameCore/Data/Pooling` | 已完成 | 通用对象池归入数据/缓存能力。 |
| `GameCore/Character/DBACharacterBuildTypes.*` | `GameCore/Types/` | 已完成 | 已中性化的构筑身份传输契约归入类型层。 |
| `GameCore/Party`、`GameCore/Queue` | `GameCore/Session/{Party,Queue}/` | 已完成 | 组队和排队服务按前台会话职责归位，协议和服务行为未变。 |
| 在线账户服务、HTTP/JSON DTO | `GameCore/Networking/Account/` | 已完成 | 登录与后端交互源码归入网络层，接口与协议字段未变。 |
| SaveGame、资料与版本类型 | `GameCore/Data/Profile/` | 已完成 | 本地持久化模型归入数据层，存档字段未变。 |
| `GameDBA/Data/*DataAsset.*` | `GameDBA/Data/Assets/` | 已完成 | Arena DataAsset 基类、英雄与技能组资产代码归位。 |
| `GameDBA/Characters/Zodiac/DBAZodiacCharacterRegistry.*` | `GameDBA/Data/Registries/` | 已完成 | 生肖角色注册表归入数据注册表层。 |
| `GameDBA/Data/*Row.h` 等表结构 | `GameDBA/Data/Tables/` | 已完成 | 纯 DataTable 行结构、地图与模式定义归位。 |
| `GameDBA/UI/Lobby/Login` 的前台流程类 | `GameDBA/Frontend/{Auth,CharacterSelection}/` | 已完成 | 登录、选角、创建角色和预览流程归位。 |
| `GameDBA/UI/Lobby` 的大厅入口类 | `GameDBA/Frontend/Lobby/` | 已完成 | 大厅入口、HUD 与 Controller 归位。 |
| `GameDBA/GAS/Abilities` | `GameDBA/Gameplay/Abilities/` | 已完成 | Arena 具体 Ability 实现归入 Gameplay 领域；ASC、Attribute、Effect、Cue 继续保留在 GAS 基础设施。 |
| `GameDBA/Combat/DBAPlayableSkill*` | `GameDBA/Gameplay/Loadout/` | 已完成 | 玩家技能目录、技能组件、配置和数据资产归入构筑领域。 |
| Arena GAS Attributes 与 Balance | `GameDBA/Gameplay/Progression/{Attributes,Balance}/` | 已完成 | 角色成长、属性默认值和英雄平衡归入成长领域；ASC、Effect、Cue 保留在 GAS。 |
| Combat Feedback、Niagara 技能参数与投射物表现配置 | `GameDBA/Presentation/VFX/` | 已完成 | 纯表现反馈与表现配置从战斗逻辑层分离。 |
| Arena Spell 与 Projectile 实现 | `GameDBA/Gameplay/Abilities/{Spells,Projectiles}/` | 已完成 | 项目特有治疗、护盾、链电及投射物能力归入 Gameplay Ability 子域。 |
| Arena 元素伤害输入输出与计算器 | `GameDBA/Gameplay/Abilities/Damage/` | 已完成 | 依赖 Arena 元素与共鸣规则，保留在项目 Gameplay，不下沉到 `GameMoba`。 |
| Arena 客户端技能预测组件 | `GameDBA/Gameplay/Abilities/Prediction/` | 已完成 | 直接依赖生肖角色、Arena ASC 与项目 RPC，属于项目能力预测实现。 |
| `ADBAPlayerState` | `GameDBA/Framework/Replication/` | 已完成 | 承载复制的对局统计、GAS 状态与权威服结算 DTO 映射。 |
| `ADBALobbyPlayerController` | `GameDBA/Frontend/Lobby/` | 已完成 | 大厅、选角展示相机与前台输入控制器归入前台大厅域。 |
| Arena 具体 RPC Handler | `GameDBA/Framework/Replication/RPC/` | 已完成 | 保持通用 RPC 契约在 `GameMoba/Networking/RPC`，Arena 角色、技能和输入的实现留在项目层。 |
| `FDBA_NavigationAgent` | `GameDBA/Gameplay/MapRules/Navigation/` | 已完成 | NavMesh 代理配置属于项目地图规则数据。 |
| 固定技能组生成子系统与配置 | `GameDBA/Gameplay/Loadout/SkillGroups/` | 已完成 | 生肖与元素固定技能组由 DataTable 异步加载，归入构筑领域。 |
| 断线重连子系统与本地会话存档 | `GameDBA/Frontend/Lobby/Reconnect/` | 已完成 | 服务端会话恢复、前台状态事件与本地重连数据属于大厅前台域。 |
| 项目通用输入组件、配置与输入语义契约 | `GameDBA/Gameplay/Input/{Components,Configuration,Contracts}/` | 已完成 | 大厅与竞技场共用的 Enhanced Input 配置归入玩法输入域，保持 DataAsset/DeveloperSettings 驱动。 |
| 未使用的默认键位映射契约 | 已移除 | 已完成 | 删除未被运行时代码调用的 `DBAInputMapping`，避免保留 Q/W/E/R、Tab、Space 等硬编码键位；唯一输入权威仍为 DataAsset 与 IMC。 |
| Android 触屏输入桥接 | `GameDBA/Gameplay/Input/Platform/` | 已完成 | 平台特有触控输入与 UI/GAS 事件桥接归入玩法输入平台子域。 |
| 观战输入配置数据资产 | `GameDBA/Spectator/Input/` | 已完成 | 仅服务观战组件的配置独立于玩家玩法输入。 |
| Arena 专有 ASC、AbilitySet、Effect 与 Cue | `GameDBA/Gameplay/GAS/` | 已完成 | 直接依赖生肖、元素共鸣、Arena 属性和数据表；不满足下沉 `GameMoba/GAS` 的复用条件。 |
| 技能名称与生肖技能定义的 DataTable 配置 | `GameDBA/Data/Tables/Settings/` | 已完成 | DeveloperSettings 仅保存 DataTable 软引用，不承载玩法规则。 |
| 技能名称 DataTable 异步查询子系统 | `GameDBA/Data/Tables/Runtime/` | 已完成 | 运行时异步加载和查询表数据，保持非阻塞数据访问职责。 |
| 十二生肖外观 DeveloperSettings | `GameDBA/Presentation/Visual/` | 已完成 | 角色预览、大厅与训练怪物的视觉资源配置归入表现层。 |
| `UDBAGameInstance` | `GameDBA/Framework/GameInstance/` | 已完成 | 项目生命周期、跨关卡前台流程入口归入框架层。 |
| 通用软引用异步加载器 | `GameCore/Async/DBAAsyncAssetLoader.h` | 已完成 | 无 Arena 依赖且被角色、玩法、UI、表现共享，归入核心异步基础设施。 |
| 通用 Subsystem 生命周期基类 | `GameCore/Core/Subsystems/` | 已完成 | GameInstance、World、LocalPlayer 子系统基类及实现混入类归入核心运行时基础设施。 |
| `GameDBA/UI/Arena`、`Common`、`Splash`、`Startup`、`Lobby/Loading` 的 View 类 | `GameDBA/UI/Widgets/` | 已完成 | Arena HUD、通用、启动、加载等 UMG C++ View 按表现职责归位。 |
| Arena/Loading Widget Controller 与 `DBAGameUIManager` | `GameDBA/UI/Controllers/` | 已完成 | 状态协调器与 Widget Controller 从 View 类中分离。 |
| 剩余 `GameDBA/UI/Lobby` View 类 | `GameDBA/UI/Widgets/Lobby/` | 已完成 | 大厅面板、队列、邀请、背包、任务与交互提示等 View 归位。 |
| `UDBAQueueWidgetController` | `GameDBA/UI/Controllers/Lobby/` | 已完成 | 队列状态协调从大厅 View 类中分离。 |

上述迁移已同步更新项目内 C++ include 路径。未保留同名旧路径转发头，避免出现双轨源码目录；UCLASS/USTRUCT 类名、模块名、资产类路径均未改变。

### 2.1 团队枚举迁移约束

`EDBATeamId` 当前仍位于 `GameDBA/Core/DBAEnumsCore.h`。该枚举带有 Unreal 反射标记，直接移动会改变脚本包路径，并可能影响既有 Blueprint、DataAsset 与地图资产的序列化引用；在完成 Editor/MCP 资产引用审查和重定向验证前，本轮不将其迁移到 `GameMoba/Teams`。后续仅在存在至少两个玩法域的真实复用需求时执行该迁移。

## 3. 目标结构与分批安排

### 3.1 Source

```text
Source/
├─ GameCore/
│  ├─ Public/GameCore/{Async,Data,Networking,Session,UI,Types}/
│  └─ Private/GameCore/...
├─ GameMoba/
│  ├─ Public/GameMoba/{Combat,GAS,Targeting,Teams,UI,Networking}/
│  └─ Private/GameMoba/...
└─ DivineBeastsArena/
   ├─ Public/GameDBA/
   │  ├─ Characters/{Zodiac,Monster,NPC}/
   │  ├─ Gameplay/{Abilities,GAS,Input,Items,Loadout,Progression,MapRules}/
   │  ├─ Frontend/{Auth,CharacterSelection,Lobby}/
   │  ├─ UI/{Controllers,ViewModels,Widgets}/
   │  ├─ Data/{Assets,Tables,Registries}/
   │  ├─ Presentation/{Animation,Audio,VFX,Visual}/
   │  ├─ Framework/{GameInstance,GameModes,Replication,Travel}/
   │  ├─ Spectator/{Input,Components,UI}/
   │  └─ Core/                         项目共享枚举、标签、错误码、类型与接口
   └─ Private/GameDBA/...（同构）
```

### 3.1.1 受控保留项

- `GameDBA/Core`：保留 Arena 共享的反射枚举、GameplayTag、错误码、类型和接口。该目录不依赖具体玩法域；其中部分反射类型可能被资产引用，未完成 Editor/MCP 引用审查前不得迁移或拆分。
- `GameDBA/Spectator`：观战输入、状态管理和 UI 共同构成完整功能域，保留独立边界，避免将观战状态机错误混入战斗或表现层。
- `GameMoba/Teams`：尚无至少两个玩法域的真实复用实现；本阶段不创建空目录或预造团队抽象。`EDBATeamId` 继续遵循第 2.1 节的资产引用审查约束。

| 批次 | 范围 | 前置条件 | 验收方式 |
| --- | --- | --- | --- |
| 第二批 | `GameCore/Account`、`Party`、`Queue`、`Subsystems` 收敛到目标中性能力；`Async`、`Data/Pooling`、`Types` 已完成 | 后端 DTO、登录流和匹配会话完成稳定标识符兼容 | C++ 编译与人工登录、选角、进入大厅审核。 |
| 第三批 | `GameMoba` 增加真实复用的 `Targeting`、`Teams`、伤害输入输出和 HUD 协议 | 至少有两个玩法域实际使用，禁止空目录和预造抽象 | C++ 编译与人工战斗/HUD 审核。 |
| 第四批 | Arena `Combat`、`GAS`、`Input`、`Navigation`、`Player`、`Services` 分入 `Gameplay`、`Frontend`、`UI` | 已完成；旧 include 路径为零，且完整 Editor Target 编译成功 | 人工验证受影响流程。 |
| 第五批 | `Data` 拆为 `Assets`、`Tables`、`Registries` | Asset Manager 与 PrimaryAsset 类型已登记 | Editor/MCP 事务、保存、人工引用审查。 |

### 3.2 Content

```text
Content/
├─ DBA/
│  ├─ Data/{Registries,Heroes,Abilities,Items,MapRules,UIFlows}/
│  ├─ Zodiacs/Chinese/{Visuals,Animations,Materials,Blueprints}/
│  ├─ Gameplay/{Abilities,Items,Progression}/
│  ├─ UI/{Frontend,Lobby,HUD,Common,Fonts}/
│  ├─ Audio/{UI,SFX,Music}/
│  ├─ VFX/{Common,Abilities,Environment}/
│  └─ Experimental/
└─ Maps/{Frontend,Lobby,Arena,Training}/
```

当前 `UI/{Frontend,Lobby,Arena,Common,Fonts}`、`Audio/{UI,SFX}`、`VFX/{Common,Abilities}` 与 `Zodiacs/Chinese` 已具备正式域基础。`Data/Defaults`、`Data/SkillCatalog`、`Data/SkillGroups`、`VFX/LegacyProjectile`、`Maps/FrostMage` 等保留为已存在来源，待引用审查通过后再通过 MCP/Editor 分批归档。当前已完成可安全执行的 Arena 源码目录收束并多次通过完整 Editor Target 编译；两项生肖权威数据资产已通过 Editor 迁移至 `Data/Registries`，其余二进制资产尚未移动。

2026-07-12 执行记录：尝试开始 `Data/Defaults -> Data/Registries` 的正式资产引用审查时，Monolith 报告 Unreal Editor 未运行；因此该 MCP 资产批次已跳过，没有产生资产或配置写入。待 Editor 可用后，必须先读取两个生肖数据资产的引用，再通过 Editor Transaction 小批次移动。

后续执行记录：Unreal Editor 已重新启动且 Monolith 本地端口处于监听状态，但当前 Codex 会话仍无法获得 Monolith MCP 工具连接。不得绕过 MCP 直接移动二进制资产；资产批次继续保持跳过。UI 源码迁移的全部编译单元已成功编译，最终 DLL 链接因打开的 Editor 占用主模块 DLL 被阻断。

2026-07-13 资产引用审查：已通过 Monolith 只读确认 `DA_DBA_ZodiacCharacterRegistry` 与 `DA_DBA_ZodiacCharacterSelection` 均仍位于 `/Game/DBA/Data/Defaults`，其依赖分别指向正式十二生肖资源域及 DBA 数据表；资产索引未返回反向资产引用。全仓文本审查额外发现 `DefaultGame.ini` 包含注册表两处、选角数据一处软引用，正式迁移必须同步更新为 `/Game/DBA/Data/Registries`。尝试通过 Monolith `editor.run_python` 在 Editor Transaction 中执行单资产迁移时，Editor 会出现无正文的“消息”模态窗口并导致 MCP 会话中断；已使用取消操作恢复 Editor，并核验两个源资产仍在 `Defaults`、目标资产不存在。故本次未产生资产或配置写入，待该写入通道恢复后再按单资产事务重试。

2026-07-13 资产迁移执行：已在 Editor Transaction 中创建 `/Game/DBA/Data/Registries`，并经 AssetTools 分两次迁移、保存 `DA_DBA_ZodiacCharacterRegistry` 与 `DA_DBA_ZodiacCharacterSelection`。`DefaultGame.ini` 的注册表两处与选角数据一处软引用均已改为 `/Game/DBA/Data/Registries`。Monolith 保存状态确认两个目标资产均在磁盘上，注册表继续引用十二生肖正式网格、AnimBP 与材质，选角资产继续引用三个正式数据表。旧 `Defaults` 路径仅剩 UE 自动生成的 `ObjectRedirector`；其反向资产引用为空。遵循当前不得删除资产的约束，本批不删除重定向器，待获得明确删除授权后以 Editor 的引用修复与重定向器清理事务处理。

2026-07-13 Defaults 余项执行：`DA_DBA_BattleAttributeDefaults` 与 `DA_DBA_HeroGrowthDefaults` 是战斗属性和英雄成长属性的项目级默认数据，不属于角色注册表。已通过独立的 Editor/MCP 事务归档至 `/Game/DBA/Gameplay/Progression`，并同步修改两项 DeveloperSettings 软引用。两个目标资产均已保存、无未保存包；旧 `Defaults` 路径仅保留 UE 自动生成的重定向器。战斗属性 DeveloperSettings 的 C++ 构造函数已移除硬编码资源路径，改由 `DefaultGame.ini` 配置，运行时仍通过异步加载与事件结果应用。

`Experimental` 仅接纳审批中的 MCP 生成资产。现有正式资源、历史来源和第三方导入资源不得通过文件系统直接拖入该目录。

### 3.3 美术源文件与导出物

| 根目录 | 目标子域 | 当前处理 |
| --- | --- | --- |
| `SourceArt` | `Characters`、`UI`、`VFX`、`Audio` | 当前已有 `UI`；其余目录只在有对应源文件时创建和迁移。 |
| `SourceAssets` | `Import`、`References` | 当前 `UI` 内容待按导入源或参考图逐批分类。 |
| `Exports` | `Approved` | 现有 `Art`、`Audio`、`DataTables`、`Fonts` 在审核通过前保持原位置；不得批量改名。 |

## 4. 禁止事项

1. 禁止用资源管理器或脚本移动 Unreal 二进制资产。
2. 禁止为了凑齐目录树创建空玩法模块、空 DataAsset 或空 Blueprint。
3. 禁止把十二生肖、元素、五营、固定技能组等 Arena 业务再次下沉到 `GameCore` 或 `GameMoba`。
4. 禁止在未完成后端协议兼容前移动账户/登录 DTO，避免破坏登录到大厅和 Dedicated Server 加入流程。
5. 禁止以自动化测试、自动旅行或自动保存替代本项目要求的人工审核。

## 5. 每批固定检查

1. 检索旧 include 路径，结果必须为零。
2. 编译受影响 C++ 模块；若 Editor 占用 DLL，仅记录链接锁定并在关闭 Editor 后完成最终链接。
3. 对资产迁移执行 MCP/Editor Transaction、Compile、Save 与引用检查。
4. 人工审核登录、选角、创建角色、进入大厅及受影响的表现流程。
