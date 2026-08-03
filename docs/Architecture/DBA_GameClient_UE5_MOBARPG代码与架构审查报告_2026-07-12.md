# DBA_GameClient UE5 MOBA+RPG 代码与架构审查报告

版本：v1.0
日期：2026-07-12
审查对象：`DBA_GameClient` Unreal Engine 5.8 游戏工程。
初始审查方法：读取工程目录、模块描述、Target、插件描述、`DefaultGame.ini`、`DefaultEngine.ini`、Git LFS/忽略规则、C++ 文件分布与资源文件名。后续整改记录见下节；未运行自动化测试、PIE、Cook 或打包。

## 整改执行记录（2026-07-12）

本报告提出的 H-01 至 H-04 已启动最小可验证整改，当前状态如下：

| 编号 | 本轮结果 | 证据 |
| --- | --- | --- |
| H-01 | 已移除业务 C++ 对旧 Content 根路径的直接引用；4 个 UI Widget 与 15 个投射物 Niagara 系统已通过 MCP AssetTools 迁入 `/Game/DBA` 正式域；投射物 VFX 引用已迁入 DeveloperSettings 配置。 | 静态检索未发现业务 C++ 的 `/Game/UI`、`/Game/Blueprints`、`/Game/ProjectileHitVFX` 等旧根路径。 |
| H-02 | 已删除 `DefaultEngine.ini` 中重复的旧域/Engine 教程强制 Cook 条目；启动 Cook 清单收敛到正式地图、UI、音频、VFX、Rosales 与 Splash 域。 | `DefaultGame.ini` 保留 7 项启动 Cook 域；Asset Manager 的 HeroData 扫描目录修正为 `/Game/DBA/Data` 并新增 AbilitySet/DBAData 类型。 |
| H-03 | 已从版本化默认配置清空 Android File Server 安全令牌，并记录本机/CI 注入要求。 | `DefaultEngine.ini` 保留空值 `SecurityToken=`，不再保存令牌内容。历史提交中的令牌旋转仍需在外部凭据系统完成。 |
| H-04 | 已将竞技场 HUD 转发和登录交互分别拆到独立 C++ 实现文件；登录 Widget 已移除 `NativeTick` 轮询，保留有上限的异步布局重试；新增 `UDBAFrontendFlowController` 统一订阅登录状态并转发异步用例。 | `DBAGameUIManager.cpp` 由约 2420 行降至 2179 行；`UDBALoginFlowWidgetBase.cpp` 由约 2259 行降至 2058 行；登录、选角与创建角色 Widget 不再直接调用登录子系统的提交命令。 |

投射物 Niagara 资产已确认存在于 `/Game/DBA/VFX/LegacyProjectile`，并已完成配置切换。旧目录当前保留 Redirector 与未迁移依赖，待引用审查为零后再单独退役。

## 1. 总体评价

**总体评价：良好，处于“基础架构已建立、项目收束尚未完成”的阶段。**

工程已经具备中大型 MOBA+RPG 客户端的关键底座：三个 Runtime C++ 模块、Game/Editor/Dedicated Server Target、GAS、ReplicationGraph、Enhanced Input、CommonUI、异步后端插件、DataAsset/DataTable、Asset Manager 及 Git LFS。`DivineBeastsArena -> GameMoba -> GameCore` 的模块方向清晰，且 Dedicated Server Target 使未来的权威战斗与多人同步具备可演进基础。

当前主要短板是“项目内容与业务职责尚未完全收束”：`Content` 同时保留正式域和旧域；主项目模块过重，UI/登录流程出现超大实现类；装备、天赋、赛季等 RPG 核心域尚未形成可由目录和类型直接确认的独立边界；Asset Manager 只登记了 Map 与 HeroData 两种 Primary Asset；Cook 配置仍同时包含新旧资源路径。它们不会阻止原型继续推进，但会在英雄、技能、商城、赛季和多地图扩张时显著提高协作与发布成本。

## 2. 优点

1. **UE 工程基础完整**：工程根目录包含标准的 `Source/`、`Content/`、`Config/`、`Plugins/`、`Build/`、`SourceArt/`、`SourceAssets/`、`Exports/`、`Docs/`，并有 Game、Editor、Server 三个 Target。
2. **模块依赖方向合理**：`GameCore` 不依赖项目业务模块，`GameMoba` 依赖 `GameCore`，`DivineBeastsArena` 依赖前两者；这是适合 MOBA+RPG 演进的依赖基础。
3. **网络与战斗基础已接入**：项目启用 GameplayAbilities、GameplayTags、GameplayTasks、ReplicationGraph、OnlineSubsystem 和 GameBackendClient；源码可确认存在 ASC、AttributeSet、ReplicationGraph、异步资产/数据表加载器。
4. **数据驱动意识明确**：`DataAssetBase`、英雄数据资产、技能组、平衡表、角色选择数据资产和 DeveloperSettings 已存在；关键配置通过软路径进入运行时，符合“C++ 逻辑 + DataAsset 配置”的项目策略。
5. **资源制作链已分离**：`SourceArt/`、`SourceAssets/`、`Exports/` 不与 `Content/` 混放，方向正确；Git LFS 已跟踪 uasset、umap、FBX、贴图、音频和视频等大文件。
6. **命名总体可读**：C++ 以 `DBA` 业务前缀为主，资源大量使用 `WBP_`、`DA_`、`DT_`、`BP_`、`MI_`、`NS_`、`SFX_`、`ABP_`、`SKM_`、`SM_` 等 UE 常用前缀，英文命名为主，未发现中文资源文件名或拼音主导的命名体系。

## 3. 目录结构审查

### 3.1 当前工程树与判断

```text
DBA_GameClient/
├─ Source/
│  ├─ GameCore/                 通用会话、账号、数据、对象池、队列、基础 UI
│  ├─ GameMoba/                 可复用 MOBA/GAS/HUD/RPC 基类
│  └─ DivineBeastsArena/        十二生肖、战斗、角色、前台、UI、VFX、项目流程
├─ Plugins/
│  └─ GameBackendClient/        异步后端访问插件
├─ Content/
│  ├─ DBA/                      当前正式项目资源主域
│  ├─ Maps/                     地图
│  ├─ Animation/ Models/ UI/    历史或迁移来源域
│  ├─ Audio/ VFX/ Blueprints/
│  ├─ MCP_Generated/            MCP 实验生成资产
│  ├─ _AutoPlaceholders/        自动占位资产
│  └─ Characters/ FrostMage/ Templates/ ProjectileHitVFX/ 等样例或第三方域
├─ Config/
├─ SourceArt/ SourceAssets/ Exports/
├─ Docs/
└─ Build/ Binaries/ Intermediate/ DerivedDataCache/ Saved/
```

### 3.2 C++ 模块与职责

| 模块 | 当前规模 | 合理部分 | 需要收敛的部分 |
| --- | ---: | --- | --- |
| `GameCore` | Public 35、Private 24 文件 | 账号、会话、异步数据、对象池和基础 UI 可作为通用能力。 | 已出现 Character、Party、Queue 等偏业务名称；十二生肖、元素、五营和固定构筑规则不得继续留在该层。 |
| `GameMoba` | Public 9、Private 7 文件 | GAS、Character、Framework、RPC、UI 基类适合成为可复用 MOBA 抽象。 | 当前明显偏薄，不能宣称已承载完整通用战斗层；应只下沉跨多个玩法域复用且不认识生肖业务的能力。 |
| `DivineBeastsArena` | Public 211、Private 168 文件 | `Character`、`Combat`、`Data`、`GAS`、`Input`、`UI`、`VFX`、`Framework` 等业务子域覆盖全面。 | 主模块承担了绝大多数项目逻辑，需继续按 Feature/Domain 分解接口，避免把所有流程、表现和协调逻辑堆到 Arena 层。 |

`DivineBeastsArena/Public/GameDBA` 与 `Private/GameDBA` 的镜像结构是正确的。Public 下只应保留稳定的跨模块 API、UCLASS/USTRUCT/UENUM 和可复用接口；实现细节、具体 Widget 流程、资源加载策略和内部辅助对象应继续留在 Private。

### 3.3 Content 资源域

`Content/DBA` 已有 AbilitySets、Audio、Blueprints、Characters、Data、Elements、FiveCamps、Heroes、Input、Materials、UI、VFX、Zodiacs 等正式子域，方向正确。正式十二生肖资源已开始固定在 `/Game/DBA/Zodiacs/Chinese`，玩家选角、创建角色和大厅角色应只经 `DA_DBA_ZodiacCharacterRegistry` 与 `DA_DBA_ZodiacCharacterSelection` 读取。

但 `Content` 仍存在 20 个一级目录，其中 `ProjectileHitVFX` 有 365 个资源、`Animation` 有 144 个、`Blueprints` 有 89 个、旧 `UI` 有 87 个、`Audio`/`VFX` 各 48 个。与 `Content/DBA` 同时存在的旧域意味着资源所有权、软引用、Cook 范围和迁移状态仍有双轨风险。

**结论**：地图维持 `/Game/Maps` 是 UE 常见且当前 Asset Manager 已扫描的合理做法；项目运行资源应逐步向 `/Game/DBA` 收束。旧 `/Game/Models/Zodiac` 与 `/Game/Animation/Zodiac` 只能作为迁移来源，不能再次成为代码或 DataAsset 回退路径。

## 4. 文件命名审查

### 4.1 C++ 命名

当前项目的 `UDBA*`、`ADBA*`、`FDBA*`、`EDBA*` 命名形式基本符合 UE 类型前缀规则，`DBA` 业务前缀也利于全局搜索。建议保持：

| 类型 | 推荐格式 | 示例 |
| --- | --- | --- |
| UObject | `UDBA<领域><名词>` | `UDBAZodiacHeroDataAsset` |
| Actor / Pawn / Controller | `ADBA<领域><名词>` | `ADBALobbyPlayerController` |
| 组件 | `UDBA<领域><名词>Component` | `UDBAPlayableSkillComponent` |
| 接口 | `IDBA<领域><能力>` | `IDBACharacterRef` |
| 结构体 / 枚举 | `FDBA...` / `EDBA...` | `FDBA...`、`EDBA...` |
| 子系统 | `UDBA<领域>Subsystem` | `UDBAHeroBalanceSubsystem` |

不建议新增 `ZodiacArena`、`ArenaGame`、无前缀 `Game*` 等第四套项目命名；现有 `GameCore`、`GameMoba` 模块名应保持不变，避免无收益重命名造成包含路径和 UHT 重定向风险。

### 4.2 资源命名

主流前缀覆盖较好，但存在以下可验证的不一致：

| 级别 | 现状示例 | 问题 | 建议 |
| --- | --- | --- | --- |
| 中 | `Content/Data/Elements/DBAElementResonanceRowe`、`Content/Data/Skills/SkillDataTable` | 缺少 `DA_`/`DT_` 前缀，且 `Rowe` 疑似拼写错误。 | 经引用审查后改为 `DT_DBA_ElementResonance`、`DT_DBA_Skill` 等按真实类型命名，并使用 Redirector 迁移。 |
| 中 | `ThirdPersonIdle`、`ThirdPersonRun`、`ThirdPersonWalk` | 动画资源缺少 `AN_` 或 `ABP_` 前缀，来源和类型不直观。 | 迁移为 `AN_Mannequin_Idle` 等，或在第三方域明确保留原厂命名并禁止正式业务直接引用。 |
| 中 | `DefaultAndCanBeDelete` | 名称表达临时状态而非资源职责，且位于 Content 根。 | 先排查引用；无引用时按资产清理流程退役，有引用则迁到明确域并按类型重命名。 |
| 低 | `SFX_*_22496` 重复尾缀 | 下载/导入批次号泄露到运行时资源名，搜索会产生同义重复。 | 用来源登记记录批次号；正式资产采用语义名，保留原始文件名仅在 SourceAssets。 |
| 低 | `M_`、`MI_`、`MF_`、`MM_`、`AM_`、`AN_`、`ABP_` 并存 | 前缀本身大多符合 UE 习惯，但团队需明确定义 `MM_`、`AN_`、`AM_` 的唯一含义。 | 在命名登记表写出前缀、资产类型、后缀、目录和示例。 |

禁止用“临时、最终、新、可删除、测试、复制、数字递增”描述正式运行时资产。Blueprint 只承担参数、资源引用和表现绑定，名称应体现其表现职责，如 `WBP_DBA_Login`、`BP_DBA_LobbyPresentation`，而不是流程实现细节。

## 5. MOBA+RPG 适配性评估

### 已具备的能力

- **英雄与选择流程**：存在 Zodiac Hero Data、角色选择/创建 Widget、角色展示 Actor、Registry/Selection DataAsset，且正向正式生肖资源域迁移。
- **技能与战斗**：存在 GAS、Ability Set、Attribute Set、伤害计算、技能投射物、战斗反馈、技能 VFX、平衡子系统与 GameplayTag 基础。
- **多人网络**：存在 Dedicated Server Target、ReplicationGraph、RPC、PlayerState、GameMode 和 OnlineSubsystem 依赖；适合采用“服务器权威、客户端预测、ASC 复制”的 MOBA 基线。
- **UI 与前台**：CommonUI、异步加载、登录/选角/大厅流程与 UI 控制器均已存在。
- **数据与资源加载**：已有 PrimaryDataAsset 基类、DataTable 异步加载器、软引用和 Asset Manager 基础扫描。

### 需要补齐或确认的能力

1. **装备/物品与天赋/符文**：目录和类型检索未发现独立 `Equipment` 或 `Talent` 领域类型。可能尚未实现，也可能使用其他命名；在新增商城、掉落、构筑和赛季内容前，应建立独立的 `Item`、`Inventory`、`Loadout`、`Progression` 领域边界。
2. **地图玩法域**：当前地图位于 `/Game/Maps`，但从目录无法确认 Lane、Jungle、Objective、Spawn、Minion、Tower、Fog of War 的可配置子域。建议以 Map Rule DataAsset 组织，而不是让地图蓝图承担规则。
3. **赛季、社交、LiveOps**：后端已经有多个服务端契约，但客户端应通过 `GameCore` 的中性异步接口和事件 ViewModel 暴露，不应使 UI 直接绑定后端 DTO 或 HTTP 细节。
4. **资源热更新与分包**：当前 Asset Manager 仅扫描 `Map` 与 `HeroData`，且 `bGenerateChunks=False`；这适合开发期，但不足以支持生产级英雄、皮肤、地图、音频和赛季内容的独立分包/按需下载。

## 6. 问题清单

### 高优先级

| 编号 | 问题 | 证据与影响 | 建议 |
| --- | --- | --- | --- |
| H-01 | Content 正式域与历史域双轨 | `/Game/DBA` 与 `Animation`、`Models`、`UI`、`VFX`、`Blueprints` 等并列；旧域仍有大量资源。 | 用资产登记和引用审查分批迁移，禁止文件系统移动 uasset/umap，禁止运行时代码回退旧路径。 |
| H-02 | Cook 配置含新旧路径并重复分散 | `DefaultGame.ini` 与 `DefaultEngine.ini` 同时列出大量 `DirectoriesToAlwaysCook`；后者还含 `/Game/Core`、`/Game/MobaBase`、`/Game/UI`、`/Game/Data` 和 Engine Tutorial 资源。 | 先做 Cook 引用审查；以 PrimaryAsset/Asset Bundle 作为权威，收缩 `AlwaysCook` 到启动地图、启动 UI 和不可避免的兜底资源。 |
| H-03 | Android File Server 安全令牌处于版本化默认配置 | `Config/DefaultEngine.ini` 的 Android File Server 节包含 `SecurityToken`。 | 立即单独走安全整改：旋转令牌，将本机/CI 令牌移至未跟踪配置或安全凭据注入；不要在报告、日志和提交信息中复制令牌值。 |
| H-04 | 超大 UI/流程实现类存在 God Class 风险 | `DBAGameUIManager.cpp` 约 2420 行；登录流程 Widget 约 2259 行；选角/创建/大厅 UI 也超过 800 至 1250 行。 | 按“状态机/异步用例/展示 Presenter/Widget 绑定”拆分；流程状态留在 C++ Subsystem 或 Controller，Widget 只处理视图事件。 |

### 中优先级

| 编号 | 问题 | 证据与影响 | 建议 |
| --- | --- | --- | --- |
| M-01 | GameCore 语义有业务泄漏，GameMoba 偏薄 | `GameCore` 仍包含 Character、Party、Queue；`DBACommonEnums.h` 维护十二生肖、元素、五营，且旧 `DBACharacterBuildTypes.cpp` 曾在 Core 内拼接固定技能组并推导五营。`GameMoba` 当前仅约 16 个源码文件。 | 保持依赖方向：先迁移身份标识与构筑规则等 Arena 业务；只将跨多个玩法域复用、且不认识十二生肖业务的 GAS、目标选择、队伍、伤害输入输出和 HUD 协议下沉 `GameMoba`。禁止为了增加文件数量而搬运项目业务。 |
| M-02 | Asset Manager 类型登记不足 | 当前仅扫描 `Map` 与 `HeroData`。 | 增加 `ZodiacRegistry`、`AbilitySet`、`ItemDefinition`、`MapRule`、`UIFlow`、`AudioBank` 等 PrimaryAssetType，并以 Asset Bundle 标注 Lobby、Match、Cosmetic、Server。 |
| M-03 | MOBA+RPG 核心域未形成显式目录 | 未见独立 Equipment/Talent 领域类型；Inventory 主要出现在大厅 UI。 | 增加 C++ 域接口与数据资产：`Gameplay/Items`、`Gameplay/Progression`、`Gameplay/Loadout`，不要先创建空目录或用 Widget 替代领域模型。 |
| M-04 | MCP/编辑器工具插件启用状态需要发布隔离 | uproject 启用 EditorToolset、VibeUE、UmgMcp、Monolith、ModelContextProtocol。 | 检查每个插件 Descriptor 的模块类型；确保仅 Editor/Developer 模块参与编辑器，Shipping 与 Dedicated Server 不加载编辑器自动化依赖。 |
| M-05 | 资源命名含临时和导入批次痕迹 | `DefaultAndCanBeDelete`、无前缀动画、`_22496` 批次尾缀等。 | 先建立命名白名单和迁移表，按引用批次在 Editor 中 Rename 并修复 Redirector。 |

#### M-01 已执行的第一阶段（2026-07-12）

1. `GameCore/Character/DBACharacterBuildTypes` 已改为仅传递 `ZodiacId`、`PrimaryElementId`、`FiveCampId` 和 `FixedSkillGroupId` 的中性 `FName` 契约；已移除 Core 中十二生肖、元素、五营到固定技能组的拼接规则，以及按元素推导五营的规则。
2. `GameCore/Session/DBATravelTypes` 的 Travel DTO 已改用中性构筑标识；它只检查字段完整性，不再在 Core 重复玩法规则。`DBAUrlOptions` 也只解析传输标识，不再维护十二生肖/元素/五营字符串映射表。
3. Dedicated Server 的运行时玩家加入上报前，已调用 `UDBASkillGroupGeneratorSubsystem::IsBuildIdentityConfigured`。该校验以异步预加载后的 Arena 固定技能组数据表 `RowId` 为权威来源，拒绝未配置、停用或开发中的构筑；不再依赖 C++ 字符串拼接。
4. `FDBAPlayerMatchInfo` 的选择字段也已改为中性标识符。账户 DTO 仍是后续批次的遗留业务语义，必须与后端 JSON 契约、选角界面和存档版本一起改造，不能为收缩目录而破坏登录与联机协议。
5. 源码目录第一批已完成：`GameDBA/Character` 已收敛为 `GameDBA/Characters`，动画与特效已归入 `GameDBA/Presentation`，GameMode、Replication、Travel 已归入对应 `Framework` 子域；`GameMoba/RPC` 已归入 `Networking/RPC`，通用 GameMode 基类已归入 `Combat`。完整批次边界见《DBA_GameClient 目标目录迁移清单》。

#### M-01 后续边界

- `GameCore` 最终只保留账号 ID、会话 ID、异步请求结果、网络传输 DTO、通用队列和中性事件；`Account`、`Session`、`MatchSession` 中的生肖/元素/五营字段应按协议版本分批替换为稳定标识符。
- `GameMoba` 只承接可跨 Arena、练习、PVE 或未来玩法复用的 GAS 基类、目标选择接口、队伍关系、伤害请求/结果、HUD 视图协议和网络战斗契约。十二生肖、五营、角色构筑、固定技能组、策划数值和资源引用继续留在 `DivineBeastsArena` 的 Data/Gameplay/Character 域。
- 每个后续批次必须先完成后端协议兼容、DataAsset 数据完整性检查和人工登录至大厅/联机加入审核；当前项目禁止以自动化测试或自动旅行替代人工验收。

### 低优先级

| 编号 | 问题 | 建议 |
| --- | --- | --- |
| L-01 | `DBA_GameClient` 是历史名，但承载 Server 与 Editor。 | 近期保持物理目录，文档统一称“UE 游戏工程”；仅在大版本且重定向、构建链完整时评估改名。 |
| L-02 | `SourceArt`、`SourceAssets`、`Exports` 需要来源与交付说明。 | 每个一级目录增加简短 README/登记表，标注来源、导入目标、是否可修改和责任人。 |
| L-03 | Target 构建输出仍包含英文 Console 文本。 | 触及 Target 时改为中文构建说明，保持全局中文日志策略。 |
| L-04 | `.mcp.json` 位于 UE 工程根目录。 | 保留本地 MCP 接入时确保不含令牌/密钥；团队共享配置采用示例文件与环境变量。 |

## 7. 推荐目标目录结构

以下是兼容现有约束的目标形态，**不是要求一次性移动**。保留 `/Game/Maps` 与已确定的 `/Game/DBA/Zodiacs/Chinese`，避免破坏现有地图和生肖资源策略。

```text
DBA_GameClient/
├─ Source/
│  ├─ GameCore/
│  │  ├─ Public/GameCore/{Async,Data,Networking,Session,UI,Types}/
│  │  └─ Private/GameCore/...
│  ├─ GameMoba/
│  │  ├─ Public/GameMoba/{Combat,GAS,Targeting,Teams,UI,Networking}/
│  │  └─ Private/GameMoba/...
│  └─ DivineBeastsArena/
│     ├─ Public/GameDBA/
│     │  ├─ Characters/{Zodiac,Monster,NPC}/
│     │  ├─ Gameplay/{Abilities,Items,Loadout,Progression,MapRules}/
│     │  ├─ Frontend/{Auth,CharacterSelection,Lobby}/
│     │  ├─ UI/{Controllers,ViewModels,Widgets}/
│     │  ├─ Data/{Assets,Tables,Registries}/
│     │  ├─ Presentation/{Animation,Audio,VFX}/
│     │  └─ Framework/{GameModes,Replication,Travel}/
│     └─ Private/GameDBA/...（同构）
├─ Plugins/
│  └─ GameBackendClient/
├─ Content/
│  ├─ DBA/
│  │  ├─ Data/{Registries,Heroes,Abilities,Items,MapRules,UIFlows}/
│  │  ├─ Zodiacs/Chinese/{Visuals,Animations,Materials,Blueprints}/
│  │  ├─ Gameplay/{Abilities,Items,Progression}/
│  │  ├─ UI/{Frontend,Lobby,HUD,Common,Fonts}/
│  │  ├─ Audio/{UI,SFX,Music}/
│  │  ├─ VFX/{Common,Abilities,Environment}/
│  │  └─ Experimental/                 仅审批中的 MCP 资产
│  └─ Maps/{Frontend,Lobby,Arena,Training}/
├─ SourceArt/{Characters,UI,VFX,Audio}/
├─ SourceAssets/{Import,References}/
├─ Exports/{Approved}/
├─ Config/
└─ Docs/
```

资源前缀的最低统一集：`BP_`、`WBP_`、`DA_`、`DT_`、`CURVE_`、`M_`、`MI_`、`MF_`、`T_`、`SM_`、`SKM_`、`SKEL_`、`ABP_`、`AN_`、`AM_`、`NS_`、`SFX_`、`BGM_`、`IA_`、`IMC_`、`MAP_`。所有前缀的确切资产类型和目录归属必须在登记表中唯一说明。

## 8. 修改实施计划

| 阶段 | 优先级 | 工作项 | 预计工作量 | 收益 |
| --- | --- | --- | --- | --- |
| 短期 1 | P0 | 旋转 Android File Server 令牌，移出版本化默认配置；审查 MCP 配置是否泄露凭据。 | 0.5 至 1 人日 | 消除直接凭据泄露风险。 |
| 短期 2 | P0 | 建立 Content 域、命名、来源和迁移台账；冻结旧生肖目录为只读迁移来源。 | 2 至 3 人日 | 防止新引用继续扩散，降低资产迁移风险。 |
| 短期 3 | P0 | 梳理并收缩 `DirectoriesToAlwaysCook`，确定启动资源与正式 PrimaryAsset 清单。 | 2 至 4 人日 | 降低包体膨胀、Cook 不确定性和旧资源误打包。 |
| 短期 4 | P1 | 拆分 `DBAGameUIManager`、登录流和选创角流：提取 C++ Flow Controller、异步 Use Case、ViewModel/Presenter。 | 5 至 10 人日 | 消除流程补丁堆积，提升人工验证和多人协作效率。 |
| 中期 1 | P1 | 以 Registry/Selection 为唯一入口完成十二生肖资源引用迁移与人工验证，再退役旧来源。 | 5 至 15 人日，按资产批次 | 正式资源域唯一、角色展示稳定。 |
| 中期 2 | P1 | 扩展 Asset Manager 与 Asset Bundle，建立英雄、技能、物品、地图规则、UI 流程的 PrimaryAsset 策略。 | 5 至 10 人日 | 支持按模式/英雄加载、分包和后续热更新方案。 |
| 中期 3 | P1 | 建立 Item/Loadout/Progression 领域模型与 DataAsset，接入 GAS 和服务器权威校验。 | 10 至 20 人日 | 为装备、养成、天赋、商城和赛季建立可扩展骨架。 |
| 长期 | P2 | 逐步清除 GameCore 的项目业务语义，提炼真正通用的 GameMoba 服务；评估地图分包、跨平台 Device Profile、赛季内容包。 | 20 人日以上 | 降低模块耦合，支撑长期英雄、地图与运营扩张。 |

所有资产移动、重命名与清理必须通过 Unreal Editor/MCP 事务完成，保留 Redirector、Compile、Save 与人工审核记录；不得用文件系统直接移动 `.uasset` 或 `.umap`。当前项目禁止自动化测试和自动旅行，本计划不将自动化测试作为前置验收，验证以人工 Editor/客户端/服务器流程审核为准。

## 9. 额外建议与需补充的信息

1. **命名规范**：在 `docs/Architecture/命名与目录登记表.md` 增加“C++ 类型、模块归属、资源前缀、后缀、禁止词、迁移例外”的单一表格，并要求新资产入库时填写来源。
2. **人工架构检查清单**：受当前“禁止自动化脚本/测试”策略约束，不建议新增自动化扫描脚本。可维护人工审核清单，检查模块方向、Public API、软引用、Cook 清单、Editor 插件隔离和资源命名。
3. **性能与网络深入审查所需信息**：要对性能、复制频率、GAS 预测、RepGraph 节点、GC、内存与实际包体给出结论，还需人工提供或允许读取 ReplicationGraph 实现、角色/技能复制策略、AssetManager 子类、Cook 报告、Unreal Insights 采样和目标平台 Device Profile。
4. **插件发布审查所需信息**：需要读取 VibeUE、UmgMcp、Monolith、ModelContextProtocol 的 `.uplugin` Descriptor 与模块类型，才能确认它们是否被 Shipping 或 Server Target 排除。

## 10. 本轮边界

本轮唯一写入为本报告。没有修改工程结构、命名、配置、安全令牌、插件、源码或资产；没有删除任何文件；没有执行自动化测试、PIE、打包、Cook 或网络联调。
