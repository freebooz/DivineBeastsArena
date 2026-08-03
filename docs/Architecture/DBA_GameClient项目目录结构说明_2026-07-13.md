# DBA_GameClient 项目目录结构说明

## 1. 文档目的与快照范围

本文记录 `DBA_GameClient` 在 2026-07-13 的实际目录结构、各目录职责和使用边界，供客户端、服务端联调、美术导入与关卡编辑共同参照。

- 项目根目录：`DBA_GameClient/`
- UE 项目文件：`DivineBeastsArena.uproject`
- 当前 C++ 模块：`GameCore`、`GameMoba`、`DivineBeastsArena`
- 正式 Content 主域：`/Game/DBA`
- 已保留地图根域：`/Game/Maps`
- 本文是当前磁盘结构说明，不等同于一次性迁移命令；二进制资产只能通过 Unreal Editor/MCP 事务移动。

## 2. 项目根目录

```text
DBA_GameClient/
├─ DivineBeastsArena.uproject             # UE 项目描述文件；定义模块、插件与启动入口。
├─ .mcp.json                              # 本机 MCP 连接配置；不得写入密钥或生产凭据。
├─ .gitattributes / .gitignore            # 版本控制与二进制资源规则。
├─ README.md                              # 客户端本地开发说明。
├─ Build/                                 # 平台打包与部署辅助文件。
│  ├─ Windows/                            # Windows 客户端打包资源与规则。
│  └─ WindowsServer/                      # Dedicated Server 打包资源与规则。
├─ Config/                                # 受版本控制的 UE 配置权威来源。
│  ├─ DefaultGame.ini                     # 游戏、DeveloperSettings、DataAsset 软引用。
│  ├─ DefaultEngine.ini                   # 引擎、地图、网络与 Cook 配置。
│  └─ DefaultInput.ini                    # Enhanced Input 默认映射配置。
├─ Content/                               # UE 二进制资产根目录；禁止文件系统直接移动 .uasset/.umap。
├─ Docs/                                  # 仅客户端局部文档与设计记录。
├─ Exports/                               # 已导出的审核素材、数据表、字体等交付物。
├─ Plugins/                               # 项目级插件。
│  └─ GameBackendClient/                  # 后端 HTTP/会话/业务客户端插件模块。
├─ Scripts/                               # 客户端维护脚本；不承载运行时 Gameplay 逻辑。
├─ Source/                                # 三层 C++ 模块源码与测试。
├─ SourceArt/                             # 原始美术源文件，按 UI/角色/VFX/音频分类。
├─ SourceAssets/                          # 导入源、参考图及素材溯源文件。
├─ Binaries/                              # UE 构建产物；不作为人工编辑目录。
├─ Intermediate/                          # UHT、编译中间产物；可再生，不作为源文件。
├─ DerivedDataCache/                      # 本机派生数据缓存；可再生。
└─ Saved/                                 # 日志、自动保存、截图与本机运行数据；不可作为正式资产来源。
```

## 3. C++ 源码结构

```text
Source/
├─ GameCore/                              # 跨玩法、跨前台流程的中性基础能力。
│  ├─ GameCore.Build.cs                   # 模块依赖声明。
│  ├─ Public/GameCore/                    # 可被其他模块依赖的稳定接口。
│  │  ├─ Async/                           # 异步资源与数据表加载器。
│  │  ├─ Core/Subsystems/                 # Subsystem 生命周期基础类。
│  │  ├─ Data/{Pooling,Profile}/          # 对象池、账号资料和本地持久化模型。
│  │  ├─ Networking/Account/              # 登录/账户网络协议与服务接口。
│  │  ├─ Session/{Party,Queue}/           # 组队、排队等前台会话通用能力。
│  │  ├─ Types/                           # 中性传输类型与构筑身份定义。
│  │  └─ UI/                              # 通用 UI 基类与事件接口。
│  ├─ Private/GameCore/                   # 上述接口的私有实现；目录与 Public 同构。
│  └─ Private/Tests/                      # GameCore 单元测试源码。
├─ GameMoba/                              # 多个竞技玩法可复用的 MOBA 基础能力。
│  ├─ GameMoba.Build.cs                   # 模块依赖声明。
│  ├─ Public/GameMoba/
│  │  ├─ Combat/                          # 通用对局基类与中性战斗契约。
│  │  ├─ GAS/                             # 可复用的 GAS 基础扩展。
│  │  ├─ Networking/RPC/                  # 通用 RPC 契约和网络边界。
│  │  ├─ Targeting/                       # 中性目标选择类型。
│  │  └─ UI/                              # 多玩法共享 HUD/UI 协议。
│  └─ Private/GameMoba/                   # GameMoba 私有实现。
└─ DivineBeastsArena/                     # 神兽竞技场项目层；承载十二生肖和具体玩法规则。
   ├─ DivineBeastsArena.Build.cs          # 项目模块依赖声明。
   ├─ Public/GameDBA/                     # 可供插件与其他模块调用的项目接口。
   ├─ Private/GameDBA/                    # 项目实现；应与 Public 维持职责同构。
   └─ Private/Tests/                      # 项目层 C++ 测试源码；不得自动执行，须人工审核触发。
```

### 3.1 `GameDBA` 项目层子域

```text
GameDBA/
├─ Characters/                            # 角色 C++：Zodiac、Monster、Guardian 等。
│  ├─ Zodiac/                             # 十二生肖角色基类与派生配置入口。
│  ├─ Monster/                            # 大厅/对局怪物、AI 组件与属性加载入口。
│  └─ Guardian/                           # 守护单位与场景守卫表现。
├─ Core/                                  # Arena 共享枚举、Tag、错误码、接口。
│  └─ Interfaces/                         # 项目内部抽象接口；反射类型迁移须先审查资产引用。
├─ Data/                                  # C++ 数据模型与加载入口，不存放玩法规则硬编码。
│  ├─ Assets/                             # DataAsset 基类、技能/英雄等数据资产类型。
│  ├─ Registries/                         # 注册表资产的 C++ 类型与查询接口。
│  └─ Tables/                             # DataTable 行结构、运行时读取与设置。
├─ Framework/                             # UE 生命周期、复制与跨关卡框架。
│  ├─ GameInstance/                       # 游戏实例、前台环境与跨关卡状态。
│  ├─ GameModes/                          # GameMode 与规则入口。
│  ├─ Replication/                        # PlayerState、ReplicationGraph、项目 RPC 实现。
│  └─ Travel/                             # URL 参数、Travel 契约和跳转边界。
├─ Frontend/                              # 登录至大厅的 C++ 流程控制层。
│  ├─ Auth/                               # 登录、账户验证和异步结果处理。
│  ├─ CharacterSelection/                 # 选角、创建角色和预览流程。
│  └─ Lobby/                              # 大厅入口、重连、前台控制器。
├─ Gameplay/                              # 权威玩法逻辑；必须以 C++ 为主。
│  ├─ Abilities/                          # Arena Ability、伤害、投射物、预测。
│  ├─ GAS/                                # ASC、Effect、Cue 与 Arena GAS 扩展。
│  ├─ Input/                              # Enhanced Input 组件、配置和平台桥接。
│  ├─ Loadout/                            # 技能目录、技能组、角色构筑。
│  ├─ MapRules/                           # 地图规则、导航代理等项目玩法定义。
│  └─ Progression/                        # 属性、成长、平衡与成长数据加载。
├─ Presentation/                          # 纯表现层 C++，不得承担权威规则。
│  ├─ Animation/                          # AnimInstance、动画工具与动画配置。
│  ├─ VFX/                                # Niagara 参数、飘字、战斗反馈与特效管理。
│  └─ Visual/                             # 角色外观、材质与展示资源设置。
├─ Spectator/                             # 观战的独立功能域。
│  ├─ Components/                         # 观战状态、相机及交互组件。
│  ├─ Input/                              # 观战输入配置。
│  └─ UI/                                 # 观战 HUD 与小地图接口。
└─ UI/                                    # C++ UI 展示协议、Controller 与 View 基类。
   ├─ Controllers/                        # UI 状态协调和事件订阅；不应形成 God Class。
   ├─ Frontend/                           # 前台 UI 控制器与流程桥接。
   └─ Widgets/                            # UMG C++ 基类；Blueprint 仅配置绑定和表现。
```

## 4. Content 资源结构

```text
Content/
├─ DBA/                                   # 当前正式资源主域；新增正式资产优先进入此处。
│  ├─ Data/                               # 数据资产、表与注册表。
│  │  ├─ Defaults/                        # 仅保留迁移重定向器；禁止新增运行时权威数据。
│  │  ├─ PoolConfigs/                     # 对象池配置资产。
│  │  ├─ Registries/                      # 十二生肖角色注册表、选角数据等权威入口。
│  │  ├─ SkillCatalog/                    # 可玩技能目录资产；待后续按玩法域复核。
│  │  ├─ SkillGroups/                     # 固定技能组配置资产；待后续按玩法域复核。
│  │  └─ Tables/                          # DataTable 与源 CSV/JSON 等导入来源。
│  ├─ Gameplay/Progression/               # 英雄成长、战斗属性等运行时数据资产。
│  ├─ Zodiacs/                            # 星座/生肖资源域。
│  │  ├─ Chinese/                         # 十二生肖唯一正式视觉资源域。
│  │  │  ├─ Animations/                   # 正式 AnimBP 与动画资源。
│  │  │  └─ Visuals/{Meshes,Materials,Skeletons}/ # 正式网格、材质和骨架。
│  │  ├─ Blueprints/                      # 生肖表现配置 Blueprint；逻辑必须在 C++。
│  │  └─ Western/DataAssets/              # 西方星座概念/数据资产域。
│  ├─ UI/                                 # 正式 UMG、贴图、字体与前台资源。
│  │  ├─ Frontend/{Login,Character,Splash}/ # 登录、选创角、启动表现。
│  │  ├─ Lobby/                           # 大厅、队伍、背包、设置、加载等界面。
│  │  ├─ Arena/HUD/                       # 对局 HUD 资源。
│  │  ├─ Common/                          # 通用交互与共享控件资源。
│  │  └─ Fonts/                           # UI 字体资源。
│  ├─ Audio/                              # UI 与技能音频。
│  │  ├─ UI/{BGM,SFX}/                    # 前台音乐与交互音效。
│  │  └─ SFX/{Abilities,Common,Downloaded}/ # 技能、通用和待审核导入音效。
│  ├─ VFX/                                # 正式特效。
│  │  ├─ Abilities/                       # 五营/能力特效。
│  │  ├─ Common/                          # 冲击、状态等共享特效。
│  │  └─ LegacyProjectile/                # 历史投射物来源；禁止新增正式引用。
│  ├─ Characters/{Mannequins,Rosales}/    # 模板或第三方角色来源；不得作为生肖运行时回退。
│  ├─ AbilitySets/、Blueprints/、Elements/、FiveCamps/、Heroes/、Input/、Materials/
│  │                                        # 已存在正式业务域；后续按引用审查逐步收束。
│  └─ ...                                 # 其他已有资产，移动前均需 Asset Registry 审查。
├─ Maps/                                  # 保留的地图根域，不与普通资产混放。
│  ├─ Arena/                              # 对局地图。
│  ├─ Lobby/                              # 大厅地图。
│  └─ FrostMage/                          # 历史/样例地图与关联资源；待人工审查归类。
├─ MCP_Generated/AI_Showcase/             # MCP 生成样例；仅供实验与审批，不能成为正式运行时依赖。
├─ Animation/、Models/、UI/、VFX/          # 根级历史资源来源；禁止新增正式运行时引用。
├─ Blueprints/、Characters/、Data/、Audio/ # 其他旧资源域；按资产引用审查分批迁移。
├─ _AutoPlaceholders/、Templates/          # 自动占位或模板内容；不得直接作为正式发版资源。
├─ Developers/                            # 开发者本地资源，不应成为共享运行时依赖。
└─ 其他第三方/示例目录                     # 如 FrostMage、ProjectileHitVFX、Splash、Game 等，先审查再归档。
```

### 4.1 当前四项权威数据资产入口

| 资产 | 正式路径 | 职责 |
| --- | --- | --- |
| 十二生肖角色注册表 | `/Game/DBA/Data/Registries/DA_DBA_ZodiacCharacterRegistry` | 角色类、网格、AnimBP、材质的唯一注册入口。 |
| 选角与创建角色数据 | `/Game/DBA/Data/Registries/DA_DBA_ZodiacCharacterSelection` | 描述、属性倾向、技能说明等选创角数据。 |
| 战斗属性默认值 | `/Game/DBA/Gameplay/Progression/DA_DBA_BattleAttributeDefaults` | 角色和怪物的属性初始值。 |
| 英雄成长默认值 | `/Game/DBA/Gameplay/Progression/DA_DBA_HeroGrowthDefaults` | 等级、经验、复活与奖励等成长默认值。 |

以上路径由 `DefaultGame.ini` 的 DeveloperSettings 软引用配置。运行时必须异步加载，UI 通过 C++ 事件更新；不得在 C++ 或 Blueprint 中复制路径、数值或流程规则。

## 5. 美术源文件、导入源与导出物

```text
SourceArt/
└─ UI/{Common,Lobby,Interaction}/          # 可编辑 UI 源图、分层设计源文件。

SourceAssets/
└─ UI/{LobbyHUD,Login,LoginCover,LoginReference}/
                                            # 导入源、参考图与素材溯源，不直接作为运行时资源。

Exports/
├─ Art/                                    # 审核后的美术导出物。
├─ Audio/                                  # 音频导出物。
├─ DataTables/                             # 数据表导出物。
└─ Fonts/                                  # 字体导出物。
```

源文件和导出物不应被 UE 运行时代码直接引用。需进入游戏的内容应经导入、命名、审核和 Editor 保存后进入 `Content/DBA`。

## 6. 目录治理规则

1. Gameplay、网络、登录、GAS、输入、UI 状态和跨系统流程只在 C++ 实现；Blueprint/UMG 只处理配置、资源引用和表现绑定。
2. 可变数值、资源引用、文案、技能和成长数据通过 DataAsset、DataTable、DeveloperSettings 或后端配置提供，不写业务硬编码。
3. `Content/DBA/Zodiacs/Chinese` 是十二生肖正式资源唯一域。选角、创建角色与大厅展示只读取注册表和选角数据资产，不得回退到 `Models/Zodiac`、`Animation/Zodiac`、模板角色或 C++ 路径。
4. `Content/DBA/Data/Defaults` 只保留当前迁移生成的重定向器；删除或修复重定向器属于破坏性资产操作，必须在单独的 Editor/MCP 事务中执行。
5. 新增地图仍放在 `Content/Maps`，按 `Frontend`、`Lobby`、`Arena`、`Training` 分类；不得通过文件资源管理器移动 `.umap`。
6. `MCP_Generated`、`_AutoPlaceholders`、`Templates`、`Developers` 和根级历史资源域不得新增为正式运行时依赖。
7. `Binaries`、`Intermediate`、`DerivedDataCache` 与 `Saved` 为生成或本机目录，不作为代码、资产或文档的权威来源。

## 7. 后续维护方式

- 每次完成源码或资产迁移后，更新本文及 `DBA_GameClient_目标目录迁移清单_2026-07-12.md`。
- C++ 变更至少执行一次对应 Editor Target 编译；资产变更必须使用 Editor/MCP 事务并保存。
- 不自动运行 PIE 或自动化测试；所有功能验证由人工审核发起。
- 目录调整优先做小批次、可回滚、可审查的迁移，不进行资源管理器批量移动或跨域大规模重命名。
