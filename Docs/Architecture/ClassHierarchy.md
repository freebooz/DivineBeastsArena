# 神兽竞技场 - 核心类族谱总览

## 一、全局继承体系图

### 1. Gameplay Framework 层次
```
AGameModeBase (UE)
    ↓
ADBAMobaGameModeBase (MobaBase)
    ↓
ADBAGameMode (DBA Project)
    ├── ADBAFrontendGameMode (Frontend)
    ├── ADBAArenaGameMode (Arena)
    └── ADBAPracticeGameMode (Practice)

AGameStateBase (UE)
    ↓
ADBAMobaGameStateBase (MobaBase)
    ↓
ADBAGameState (DBA Project)
    ├── ADBAFrontendGameState
    ├── ADBAArenaGameState
    └── ADBAPracticeGameState

APlayerState (UE)
    ↓
ADBAMobaPlayerStateBase (MobaBase)
    ↓
ADBAPlayerState (DBA Project)

APlayerController (UE)
    ↓
ADBAMobaPlayerControllerBase (MobaBase)
    ↓
ADBAPlayerController (DBA Project)

ACharacter (UE)
    ↓
ADBAMobaCharacterBase (MobaBase)
    ↓
ADBAHero (DBA Project)
    ├── ADBAHero_[Zodiac]_[Element] (具体英雄)
    └── ...

AHUD (UE)
    ↓
ADBAMobaHUDBase (MobaBase)
    ↓
ADBAHUD (DBA Project)
```

### 2. GAS 层次
```
UAbilitySystemComponent (UE)
    ↓
UDBAMobaAbilitySystemComponentBase (MobaBase)
    ↓
UDBAAbilitySystemComponent (DBA Project)

UAttributeSet (UE)
    ↓
UDBAMobaAttributeSetBase (MobaBase)
    ↓
UDBAAttributeSet (DBA Project)

UGameplayAbility (UE)
    ↓
UDBAMobaGameplayAbilityBase (MobaBase)
    ↓
UDBAGameplayAbility (DBA Project)
    ├── UDBASkillAbility (普通技能)
    ├── UDBAUltimateAbility (生肖大招)
    └── UDBAPassiveAbility (被动技能)

UGameplayEffect (UE)
    ↓
UDBAMobaGameplayEffectBase (MobaBase)
    ↓
UDBAGameplayEffect (DBA Project)
```

### 3. Component 层次
```
UActorComponent (UE)
    ↓
UDBAMobaComponentBase (MobaBase)
    ↓
UDBAComponent (DBA Project)
    ├── UDBACombatComponent (战斗组件)
    ├── UDBATargetingComponent (目标选择组件)
    ├── UDBAInteractionComponent (交互组件)
    ├── UDBAChainComponent (连锁组件)
    ├── UDBAResonanceComponent (共鸣组件)
    └── UDBAUltimateEnergyComponent (终极能量组件)
```

### 4. Subsystem 层次
```
UGameInstanceSubsystem (UE)
    ↓
UDBAMobaSubsystemBase (MobaBase)
    ↓
UDBASubsystem (DBA Project)
    ├── UDBAAccountSubsystem (账户子系统)
    ├── UDBACharacterRosterSubsystem (角色名册子系统)
    ├── UDBALobbySubsystem (大厅子系统)
    ├── UDBAPartySubsystem (组队子系统)
    ├── UDBAQueueSubsystem (匹配子系统)
    ├── UDBAReadyCheckSubsystem (准备检查子系统)
    ├── UDBAHeroSelectSubsystem (英雄选择子系统)
    ├── UDBAFlowSubsystem (流程子系统)
    ├── UDBAMonitoringClientSubsystem (监控客户端子系统，可选)
    └── UDBAGameOpsClientSubsystem (运维客户端子系统，可选)

ULocalPlayerSubsystem (UE)
    ↓
UDBALocalPlayerSubsystem (DBA Project)
    ├── UDBAUIManagerSubsystem (UI 管理子系统)
    └── UDBAInputSubsystem (输入子系统)
```

### 5. AI 层次
```
AAIController (UE)
    ↓
ADBAMobaAIControllerBase (MobaBase)
    ↓
ADBAMinionAIController (小兵 AI)
ADBAMonsterAIController (野怪 AI)
ADBAObjectiveAIController (目标点 AI)
ADBATurretAIController (防御塔 AI)
ADBACoreAIController (核心 AI)
ADBANPCAIController (NPC AI)

APawn (UE)
    ↓
ADBAMobaPawnBase (MobaBase)
    ↓
ADBAMinion (小兵)
ADBAMonster (野怪)
ADBAObjective (目标点)
ADBATurret (防御塔)
ADBACore (核心)
ADBANPC (NPC)
```

### 6. UI 层次
```
UUserWidget (UE)
    ↓
UDBAMobaUserWidgetBase (MobaBase)
    ↓
UDBAUserWidget (DBA Project)
    ├── UDBAUnitFrame (单位框架)
    ├── UDBATargetFrame (目标框架)
    ├── UDBAPartyFrame (队伍框架)
    ├── UDBARaidFrame (团队框架)
    ├── UDBAActionBar (动作条)
    ├── UDBABuffBar (Buff 条)
    ├── UDBACastBar (施法条)
    ├── UDBATooltip (提示框)
    ├── UDBANotification (通知)
    ├── UDBACombatText (战斗文字)
    ├── UDBAMinimap (小地图)
    └── UDBAScoreboard (计分板)

UObject (UE)
    ↓
UDBAWidgetController (Widget 控制器)
    ├── UDBAHUDController
    ├── UDBAFrontendController
    └── UDBAArenaController
```

### 7. Data 层次
```
UDataAsset (UE)
    ↓
UDBAMobaDataAssetBase (MobaBase)
    ↓
UDBADataAsset (DBA Project)
    ├── UDBAZodiacDataAsset (生肖数据)
    ├── UDBAElementDataAsset (元素数据)
    ├── UDBAFiveCampDataAsset (阵营数据)
    ├── UDBASkillDataAsset (技能数据)
    ├── UDBAAbilitySetDataAsset (技能组数据)
    ├── UDBAHeroDataAsset (英雄数据)
    └── UDBAMatchRuleDataAsset (匹配规则数据)

FTableRowBase (UE)
    ↓
FDBAMobaTableRowBase (MobaBase)
    ↓
FDBATableRow (DBA Project)
    ├── FDBAZodiacRow (生肖表行)
    ├── FDBAElementRow (元素表行)
    ├── FDBAFiveCampRow (阵营表行)
    ├── FDBASkillRow (技能表行)
    ├── FDBAHeroRow (英雄表行)
    └── FDBAFixedSkillGroupRow (固定技能组表行)

USaveGame (UE)
    ↓
UDBAMobaSaveGameBase (MobaBase)
    ↓
UDBASaveGame (DBA Project)
    ├── UDBAProfileSaveGame (档案存档)
    └── UDBASettingsSaveGame (设置存档)
```

### 8. Interface 层次
```
UInterface (UE)
    ↓
UDBAMobaInterfaceBase (MobaBase)
    ↓
UDBAInterface (DBA Project)
    ├── UDBADamageableInterface (可受伤接口)
    ├── UDBATargetableInterface (可选中接口)
    ├── UDBAInteractableInterface (可交互接口)
    ├── UDBATeamInterface (队伍接口)
    ├── UDBAAbilitySourceInterface (技能来源接口)
    └── UDBAReplicatedInterface (复制接口)
```

## 二、MobaBase 层与 DBA 项目层映射表

| MobaBase 层 | DBA 项目层 | 职责 |
|-------------|-----------|------|
| ADBAMobaGameModeBase | ADBAGameMode | 游戏模式基类 |
| ADBAMobaGameStateBase | ADBAGameState | 游戏状态基类 |
| ADBAMobaPlayerStateBase | ADBAPlayerState | 玩家状态基类 |
| ADBAMobaPlayerControllerBase | ADBAPlayerController | 玩家控制器基类 |
| ADBAMobaCharacterBase | ADBAHero | 角色基类 → 英雄 |
| ADBAMobaHUDBase | ADBAHUD | HUD 基类 |
| UDBAMobaAbilitySystemComponentBase | UDBAAbilitySystemComponent | ASC 基类 |
| UDBAMobaAttributeSetBase | UDBAAttributeSet | 属性集基类 |
| UDBAMobaGameplayAbilityBase | UDBAGameplayAbility | 技能基类 |
| UDBAMobaComponentBase | UDBAComponent | 组件基类 |
| UDBAMobaSubsystemBase | UDBASubsystem | 子系统基类 |
| UDBAMobaAIControllerBase | ADBA*AIController | AI 控制器基类 |
| UDBAMobaPawnBase | ADBA*Pawn | Pawn 基类 |
| UDBAMobaUserWidgetBase | UDBAUserWidget | UI 基类 |
| UDBAMobaDataAssetBase | UDBADataAsset | 数据资产基类 |
| FDBAMobaTableRowBase | FDBATableRow | 数据表行基类 |
| UDBAMobaSaveGameBase | UDBASaveGame | 存档基类 |
| UDBAMobaInterfaceBase | UDBAInterface | 接口基类 |

## 三、职责总表

### Frontend 子系统职责

| 子系统 | 职责 | 依赖 |
|--------|------|------|
| UDBAAccountSubsystem | 账户登录、注册、登出 | SaveGame |
| UDBACharacterRosterSubsystem | 角色名册管理 | SaveGame, DataAsset |
| UDBALobbySubsystem | 大厅状态管理 | Account |
| UDBAPartySubsystem | 组队、邀请、踢人 | Account, Lobby |
| UDBAQueueSubsystem | 匹配队列、取消匹配 | Party |
| UDBAReadyCheckSubsystem | 准备检查 | Queue |
| UDBAHeroSelectSubsystem | 英雄选择流程 | ReadyCheck, DataAsset |
| UDBAFlowSubsystem | 全局流程状态机 | 所有 Frontend 子系统 |

### Arena 核心职责

| 类 | 职责 | 依赖 |
|----|------|------|
| ADBAArenaGameMode | 对局规则、胜负判定 | GameState, PlayerState |
| ADBAArenaGameState | 对局状态、计分、时间 | PlayerState |
| ADBAHero | 英雄逻辑、技能释放 | ASC, Component |
| UDBACombatComponent | 战斗逻辑、伤害计算 | ASC, AttributeSet |
| UDBAChainComponent | 连锁系统 | Combat |
| UDBAResonanceComponent | 共鸣系统 | ASC |
| UDBAUltimateEnergyComponent | 终极能量 | ASC |

### AI 职责

| 类 | 职责 | 依赖 |
|----|------|------|
| ADBAMinionAIController | 小兵 AI 逻辑 | BehaviorTree, Blackboard |
| ADBAMonsterAIController | 野怪 AI 逻辑 | BehaviorTree, Blackboard |
| ADBAObjectiveAIController | 目标点 AI 逻辑 | BehaviorTree, Blackboard |
| ADBATurretAIController | 防御塔 AI 逻辑 | BehaviorTree, Blackboard |
| ADBACoreAIController | 核心 AI 逻辑 | BehaviorTree, Blackboard |
| ADBANPCAIController | NPC AI 逻辑 | BehaviorTree, Blackboard |

### UI 职责

| 类 | 职责 | 依赖 |
|----|------|------|
| UDBAUIManagerSubsystem | UI 层级管理、显示隐藏 | LocalPlayer |
| UDBAWidgetController | UI 数据绑定、事件处理 | PlayerState, GameState |
| UDBAUnitFrame | 玩家单位框架 | PlayerState |
| UDBATargetFrame | 目标框架 | TargetingComponent |
| UDBAPartyFrame | 队伍框架 | PartySubsystem |
| UDBARaidFrame | 团队框架 | GameState |
| UDBAActionBar | 动作条 | ASC |
| UDBABuffBar | Buff 条 | ASC |
| UDBACastBar | 施法条 | ASC |

### Data 职责

| 类 | 职责 | 依赖 |
|----|------|------|
| UDBAZodiacDataAsset | 十二生肖配置 | 无 |
| UDBAElementDataAsset | 自然元素之力配置 | 无 |
| UDBAFiveCampDataAsset | 五大阵营配置 | 无 |
| UDBASkillDataAsset | 技能配置 | Element, Zodiac |
| UDBAAbilitySetDataAsset | 技能组配置 | Skill |
| UDBAHeroDataAsset | 英雄配置 | Zodiac, Element, FiveCamp, AbilitySet |
| UDBAMatchRuleDataAsset | 匹配规则配置 | 无 |

### 交互职责

| 类 | 职责 | 依赖 |
|----|------|------|
| UDBAInteractableInterface | 可交互接口 | 无 |
| UDBAInteractionComponent | 交互组件 | InteractableInterface |
| ADBAPortalActor | 传送门 Actor | Interaction |
| UDBAFrontendWorldBridge | 前台世界桥接 | GameInstance |

### 外部服务职责（可选）

| 类 | 职责 | 依赖 |
|----|------|------|
| UDBAMonitoringClientSubsystem | 监控上报客户端 | HTTP, Json |
| UDBAGameOpsClientSubsystem | 运维可见性客户端 | HTTP, Json |
| FDBATelemetryTypes | 遥测数据类型 | 无 |
| FDBAGameOpsTypes | 运维数据类型 | 无 |
| UDBAExternalServiceSettings | 外部服务配置 | DeveloperSettings |

## 四、Actor / Component / UObject 归属建议

### Actor 归属
- 需要在世界中存在的实体
- 需要 Transform 的对象
- 需要碰撞的对象
- 示例：
  - ADBAHero
  - ADBAMinion
  - ADBATurret
  - ADBAPortalActor
  - ADBAProjectile

### Component 归属
- 可附加到 Actor 的功能模块
- 可复用的逻辑单元
- 示例：
  - UDBAAbilitySystemComponent
  - UDBACombatComponent
  - UDBAInteractionComponent
  - UDBATargetingComponent

### UObject 归属
- 不需要在世界中存在的逻辑对象
- 数据容器
- 工具类
- 示例：
  - UDBAWidgetController
  - UDBADataAsset
  - UDBAAbilitySetDataAsset
  - UDBAGameplayAbility

### Subsystem 归属
- 全局单例服务
- 生命周期由引擎管理
- 示例：
  - UDBAAccountSubsystem
  - UDBAPartySubsystem
  - UDBAUIManagerSubsystem

### SaveGame 归属
- 需要持久化的数据
- 示例：
  - UDBAProfileSaveGame
  - UDBASettingsSaveGame

### DataAsset 归属
- 策划配置数据
- 静态数据
- 示例：
  - UDBAZodiacDataAsset
  - UDBASkillDataAsset

### DataTable Row Struct 归属
- 表格数据行
- CSV 导入数据
- 示例：
  - FDBAZodiacRow
  - FDBASkillRow

### Interface 归属
- 解耦不同系统
- 定义契约
- 示例：
  - UDBADamageableInterface
  - UDBAInteractableInterface

## 五、文件名 / 类名 / 目录名命名登记总表

### 命名规则
```
文件名：[Prefix][Descriptor][Suffix].[Extension]
类名：[Prefix][Descriptor][Suffix]
目录名：[Descriptor]

示例：
- DBAGameModeBase.h / ADBAGameModeBase
- DBAAbilitySystemComponent.h / UDBAAbilitySystemComponent
- DBAZodiacDataAsset.h / UDBAZodiacDataAsset
- Core/ (目录)
```

### 前缀规则
| 前缀 | 类型 |
|------|------|
| A | Actor 派生类 |
| U | UObject 派生类（非 Actor） |
| F | 结构体 / 普通 C++ 类 |
| E | 枚举 |
| I | 接口（已弃用，使用 U + Interface 后缀） |
| T | 模板类 |

### 后缀规则
| 后缀 | 含义 |
|------|------|
| Base | 基类 |
| Component | 组件 |
| Subsystem | 子系统 |
| Interface | 接口 |
| DataAsset | 数据资产 |
| Row | 数据表行 |
| SaveGame | 存档 |
| Settings | 设置 |
| Controller | 控制器 |
| Widget | UI 组件 |

### 命名登记表

| 文件名 | 类名 | 目录 | 层级 |
|--------|------|------|------|
| DBAGameModeBase.h | ADBAMobaGameModeBase | MobaBase/Framework/ | Base |
| DBAGameMode.h | ADBAGameMode | DBA/Framework/ | Project |
| DBAGameStateBase.h | ADBAMobaGameStateBase | MobaBase/Framework/ | Base |
| DBAGameState.h | ADBAGameState | DBA/Framework/ | Project |
| DBAPlayerStateBase.h | ADBAMobaPlayerStateBase | MobaBase/Framework/ | Base |
| DBAPlayerState.h | ADBAPlayerState | DBA/Framework/ | Project |
| DBAPlayerControllerBase.h | ADBAMobaPlayerControllerBase | MobaBase/Framework/ | Base |
| DBAPlayerController.h | ADBAPlayerController | DBA/Framework/ | Project |
| DBACharacterBase.h | ADBAMobaCharacterBase | MobaBase/Framework/ | Base |
| DBAHero.h | ADBAHero | DBA/Heroes/ | Project |
| DBAHUDBase.h | ADBAMobaHUDBase | MobaBase/Framework/ | Base |
| DBAHUD.h | ADBAHUD | DBA/Framework/ | Project |
| DBAAbilitySystemComponentBase.h | UDBAMobaAbilitySystemComponentBase | MobaBase/GAS/ | Base |
| DBAAbilitySystemComponent.h | UDBAAbilitySystemComponent | DBA/GAS/ | Project |
| DBAAttributeSetBase.h | UDBAMobaAttributeSetBase | MobaBase/GAS/ | Base |
| DBAAttributeSet.h | UDBAAttributeSet | DBA/GAS/ | Project |
| DBAGameplayAbilityBase.h | UDBAMobaGameplayAbilityBase | MobaBase/GAS/ | Base |
| DBAGameplayAbility.h | UDBAGameplayAbility | DBA/GAS/ | Project |
| DBACombatComponent.h | UDBACombatComponent | DBA/Combat/ | Project |
| DBAChainComponent.h | UDBAChainComponent | DBA/Combat/ | Project |
| DBAResonanceComponent.h | UDBAResonanceComponent | DBA/Combat/ | Project |
| DBAUltimateEnergyComponent.h | UDBAUltimateEnergyComponent | DBA/Combat/ | Project |
| DBAAccountSubsystem.h | UDBAAccountSubsystem | Frontend/Account/ | Project |
| DBAPartySubsystem.h | UDBAPartySubsystem | Frontend/Party/ | Project |
| DBAQueueSubsystem.h | UDBAQueueSubsystem | Frontend/Queue/ | Project |
| DBAFlowSubsystem.h | UDBAFlowSubsystem | Frontend/Flow/ | Project |
| DBAMinionAIController.h | ADBAMinionAIController | AI/Minion/ | Project |
| DBAMonsterAIController.h | ADBAMonsterAIController | AI/Monster/ | Project |
| DBAUIManagerSubsystem.h | UDBAUIManagerSubsystem | UI/Framework/ | Project |
| DBAWidgetController.h | UDBAWidgetController | UI/Framework/ | Project |
| DBAUnitFrame.h | UDBAUnitFrame | UI/HUD/ | Project |
| DBATargetFrame.h | UDBATargetFrame | UI/HUD/ | Project |
| DBAZodiacDataAsset.h | UDBAZodiacDataAsset | DBA/Data/ | Project |
| DBAElementDataAsset.h | UDBAElementDataAsset | DBA/Data/ | Project |
| DBASkillDataAsset.h | UDBASkillDataAsset | DBA/Data/ | Project |
| DBAInteractableInterface.h | UDBAInteractableInterface | Shared/Interfaces/ | Project |
| DBAPortalActor.h | ADBAPortalActor | Frontend/Portal/ | Project |
| DBAMonitoringClientSubsystem.h | UDBAMonitoringClientSubsystem | External/Monitoring/ | Project |
| DBAGameOpsClientSubsystem.h | UDBAGameOpsClientSubsystem | External/GameOps/ | Project |

## 六、单向依赖图

```
Core (基础类型、枚举、常量)
    ↓
MobaBase (通用 MOBA 逻辑)
    ↓
DBA Project (项目专属逻辑)
    ↓
Frontend / Arena / Practice (功能模块)
    ↓
UI / AI / Audio (表现层)

外部服务（可选，低耦合）
    ← 可选依赖 ← DBA Project
```

### 依赖规则
1. 下层不能依赖上层
2. 同层之间通过接口解耦
3. 外部服务不能被核心逻辑依赖
4. Dedicated Server 不依赖客户端表现层

## 七、Stub 类优先级

### 必须先有 Stub 的类
1. **Framework 基类**
   - ADBAMobaGameModeBase
   - ADBAMobaGameStateBase
   - ADBAMobaPlayerStateBase
   - ADBAMobaPlayerControllerBase
   - ADBAMobaCharacterBase

2. **GAS 基类**
   - UDBAMobaAbilitySystemComponentBase
   - UDBAMobaAttributeSetBase
   - UDBAMobaGameplayAbilityBase

3. **Component 基类**
   - UDBAMobaComponentBase

4. **Subsystem 基类**
   - UDBAMobaSubsystemBase

5. **Interface**
   - UDBADamageableInterface
   - UDBATargetableInterface
   - UDBAInteractableInterface

### 可后续实现的类
- 具体技能实现
- 具体 UI 页面
- 具体 AI 行为
- 具体数据资产

## 八、蓝图派生建议

### 适合蓝图派生的类
- ADBAHero（英雄实现）
- ADBAMinion（小兵实现）
- ADBAMonster（野怪实现）
- UDBAUserWidget（UI 组件）
- UDBAGameplayAbility（技能实现）
- ADBAPortalActor（传送门）

### 不适合蓝图承担权威逻辑的类
- ADBAGameMode（服务器权威）
- ADBAGameState（服务器权威）
- ADBAPlayerState（服务器权威）
- UDBAAbilitySystemComponent（服务器权威）
- UDBACombatComponent（服务器权威）
- 所有 Subsystem（服务器权威）

### 蓝图使用原则
- 蓝图用于资源绑定和表现配置
- 核心逻辑必须在 C++ 中实现
- 服务器权威逻辑不能在蓝图中实现
- 蓝图可调用 C++ 暴露的接口

## 九、Dedicated Server 类行为

### 仅保留逻辑骨架的类
```cpp
#if UE_SERVER
    // 仅保留逻辑，不创建表现资源
    // 不加载 Niagara、Sound、Material
#endif
```

- ADBAHUD（Server 不创建）
- UDBAUserWidget（Server 不创建）
- UDBAUIManagerSubsystem（Server 不创建）
- Niagara 相关类（Server 不加载）
- Audio 相关类（Server 不加载）

### 完整实现的类
- ADBAGameMode
- ADBAGameState
- ADBAPlayerState
- ADBAPlayerController
- ADBAHero
- UDBAAbilitySystemComponent
- UDBACombatComponent
- 所有 AI 类
- 所有 Subsystem

## 十、VS Code 配置在工具链中的位置

### 工具链层次
```
开发者
    ↓ 选择工具
Visual Studio / Rider / VS Code / 命令行
    ↓ 调用
Scripts/ (构建脚本)
    ↓ 调用
UE5 UBT / UAT
    ↓ 读取
Target.cs / Build.cs
    ↓ 生成
编译命令
    ↓ 执行
编译器
```

### VS Code 配置位置
```
.vscode/
├── settings.json       # 编辑器设置（不调用构建）
├── tasks.json          # 构建任务（调用 Scripts/）
├── launch.json         # 调试配置（调用可执行文件）
└── extensions.json     # 推荐扩展（不参与构建）

Scripts/
├── Build/              # 实际构建逻辑
│   ├── BuildEditor.bat
│   ├── BuildClient.bat
│   └── BuildServer.bat
└── Tools/              # 辅助工具

Source/
├── *.Target.cs         # Target 配置
└── */Build.cs          # 模块配置
```

### 关系说明
- VS Code 配置不包含构建逻辑
- VS Code tasks.json 调用 Scripts/
- Scripts/ 是构建逻辑的唯一真实来源
- 其他 IDE 也可调用 Scripts/
- 保证工具链一致性