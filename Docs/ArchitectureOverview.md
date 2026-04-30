# DivineBeastsArena 神兽竞技场 - 架构文档

> 本文档基于代码分析生成，详细说明项目架构、核心系统设计与模块交互。

---

## 1. 项目概览

**项目名称**: DivineBeastsArena (神兽竞技场)
**引擎版本**: Unreal Engine 5.7.1 (源代码编译)
**核心玩法**: MOBA 5v5 实时竞技
**架构模式**: GAS (Gameplay Ability System) + UMG UI + Subsystem

### 1.1 目录结构

```
Source/DivineBeastsArena/
├── Private/                    # 实现文件
│   ├── Core/                   # 核心系统
│   │   ├── Account/            # 账号服务
│   │   ├── ObjectPool/         # 对象池
│   │   ├── Party/              # 队伍系统
│   │   ├── Queue/              # 匹配队列
│   │   ├── Session/            # 会话管理
│   │   └── Subsystems/         # 核心子系统
│   ├── Data/                   # 数据资产
│   ├── Lobby/               # 前台系统 (大厅/选择流程)
│   ├── GAS/                    # Gameplay Ability System
│   │   ├── Abilities/          # 技能基类
│   │   ├── Attributes/         # 属性集
│   │   └── Effects/            # 游戏效果
│   ├── Input/                  # 输入系统
│   ├── MobaBase/               # MOBA 框架基类
│   └── UI/                     # UI 系统
│       ├── Arena/              # 竞技场 HUD
│       ├── Lobby/           # 前台 UI
│       └── Startup/            # 启动动画
├── Public/                     # 头文件
│   └── (同上结构)
└── DivineBeastsArena.h         # 模块入口
```

---

## 2. 核心架构

### 2.1 Gameplay Ability System (GAS)

GAS 是游戏核心战斗系统的基石，管理技能、属性和状态效果。

#### 2.1.1 继承层级

```
UAbilitySystemComponent
└── UDBAMobaAbilitySystemComponentBase      # MOBA ASC 扩展
    └── UDBAAbilitySystemComponent          # 项目专用 ASC

UGameplayAbility
├── UDBAMobaGameplayAbilityBase             # MOBA Ability 基类
│   ├── UDBAElementAbilityBase             # 元素技能 (Skill01~04)
│   ├── UDBAZodiacAbilityBase              # 生肖技能基类
│   │   └── UDBAZodiacUltimateAbilityBase   # 生肖大招
│   └── UDBAResonanceAbilityBase           # 共鸣被动

UAttributeSet
├── UDBABattleAttributeSet                 # 战斗属性 (生命/能量/防御等)
└── UDBHeroGrowthAttributeSet              # 成长属性
```

#### 2.1.2 属性系统 (DBABattleAttributeSet)

**复制属性** (使用 `DOREPLIFETIME_CONDITION_NOTIFY`):

| 属性 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| CurrentHealth / MaxHealth | float | 1000 / 1000 | 生命值 |
| CurrentEnergy / MaxEnergy | float | 100 / 100 | 能量值 |
| AttackPower | float | 100 | 攻击力 |
| Defense | float | 50 | 防御力 |
| MoveSpeed | float | 600 | 移动速度 |
| CriticalRate | float | 0.1 | 暴击率 (0~1) |
| CriticalMultiplier | float | 2.0 | 暴击倍率 |
| MaxShield / CurrentShield | float | 0 / 0 | 护盾值 |

**防御计算公式**:
```cpp
DamageReduction = Defense / (Defense + 100);
FinalDamage = BaseDamage * (1 - DamageReduction);
```

#### 2.1.3 技能激活策略 (EDBAMobaAbilityActivationPolicy)

```cpp
enum class EDBAMobaAbilityActivationPolicy : uint8
{
    OnInputTriggered,   // 收到输入时激活
    OnSpawn,            // Avatar 生成时自动激活
    Passive             // 被动技能
};
```

#### 2.1.4 能量消耗机制

能量消耗通过 GameplayEffect 实现，确保预测和复制系统正常工作：

```cpp
// DBAElementAbilityBase::CommitAbilityCost
FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
EffectContext.AddSourceObject(ActorInfo->AvatarActor.Get());
TSubclassOf<UGameplayEffect> EnergyCostEffectClass = UDBEEnergyCostEffect::StaticClass();
FGameplayEffectSpec EffectSpec(EnergyCostEffectClass, EffectContext, EnergyCost);
ASC->ApplyGameplayEffectSpecToSelf(EffectSpec);
```

---

## 3. 输入系统

### 3.1 架构概览

```
PlayerController
└── UDBAInputRouterComponent           # 输入路由组件
    ├── 加载平台对应的 InputMappingContext
    ├── 绑定输入动作到回调函数
    └── 发送 RPC 到服务端

UDBAAndroidTouchInputBridge           # Android 触屏专用
    ├── 技能轮盘 (长按展开/拖拽选择/松手释放)
    └── Ultimate 大按钮状态管理
```

### 3.2 输入路由组件 (DBAInputRouterComponent)

**职责**:
- 根据平台加载对应的输入映射上下文
- 绑定输入动作到 GAS / Targeting 系统
- 处理客户端本地反馈与服务端裁定边界
- 提供输入调试日志开关

**RPC 频率限制**:

| RPC | 最小间隔 |
|-----|----------|
| ServerPing | 0.5s |
| ServerLockTarget | 0.3s |
| ServerInteract | 0.2s |

**UI 事件广播**:

```cpp
FDBAOnPingTriggered          // Ping 位置
FDBAOnScoreboardTriggered    // 计分板切换
FDBAOnMenuTriggered          // 菜单
FDBAOnChatTriggered          // 聊天
FDBAOnMapTriggered            // 地图
FDBAOnLockTargetChanged       // 锁定目标变更
```

### 3.3 Android 触屏输入桥接

**技能轮盘事件**:

```cpp
FDBAOnSkillWheelShow         // 轮盘显示 (SkillIndex)
FDBAOnSkillWheelHide         // 轮盘隐藏
FDBAOnSkillDirectionUpdate   // 方向更新 (FVector2D)
FDBAOnSkillReleased          // 技能释放 (SkillIndex)
```

---

## 4. UI 系统架构

### 4.1 Widget 层级

```
UUserWidget
├── UDBAUserWidgetBase              # 通用基类
│   └── UDBAWidgetBase              # 包含 WidgetController 支持
│       └── DBAAbilityBarWidgetBase # 技能栏
└── UDBAWidgetController            # 数据绑定控制器
```

### 4.2 WidgetController 模式

WidgetController 负责从 GameInstanceSubsystem 获取数据并绑定到 Widget：

```
DBAGameInstanceSubsystem
    └── DBAWidgetController
            └── DBAArenaHUDWidgetController
                    ├── DBAPlayerUnitFrameWidgetController  # 玩家信息框
                    ├── DBABuffBarWidgetController          # Buff 栏
                    └── ...
```

### 4.3 核心 UI 组件

**竞技场 HUD**:
- `DBAAbilityBarWidgetBase` - 技能栏
- `DBAPlayerUnitFrameWidgetBase` - 玩家信息框 (头像/生命/能量)
- `UDBABuffBarWidgetBase` - Buff/Debuff 栏
- `UDBAUltimateReadyPromptWidgetBase` - 大招就绪提示
- `UDBAChainUltimatePanelWidgetBase` - 连环大招面板
- `UDBAMomentumPanelWidgetBase` - 动能面板

---

## 5. 前台流程子系统

### 5.1 选择流程

```
DBAMatchEntryCoordinator           # 入口协调器
├── DBAHeroSelectSubsystem        # 生肖选择
├── DBAElementSelectSubsystem     # 元素之力选择
├── DBAFiveCampSelectSubsystem    # 五大阵营选择
└── DBAFixedSkillGroupSubsystem   # 固定技能组

DBABuildValidationLibrary          # 构建验证
DBABuildValidationHintWidgetBase   # 验证提示 UI
```

### 5.2 匹配队列

```
DBAQueueSubsystem                  # 队列管理
DBAQueueServiceBase               # 队列服务 (可扩展)
DBAReadyCheckSubsystem            # 准备确认
```

### 5.3 多人系统

```
DBAPartySubsystem                 # 队伍子系统
DBAPartyServiceBase               # 队伍服务基类
DBAInviteServiceStub              # 邀请服务 (存根)
```

---

## 6. 数据资产

### 6.1 核心数据资产

| 数据资产 | 用途 |
|----------|------|
| `DBAZodiacHeroDataAsset` | 生肖英雄配置 (生肖/阵营/技能组) |
| `DBAElementDefinition` | 自然元素定义 (克制关系/图标/颜色) |
| `DBAAbilitySetDataAsset` | 技能组数据 (Passive/Skill01~04/Ultimate/Resonance) |
| `DBAModeDefinition` | 游戏模式定义 |
| `DBAMapDefinition` | 地图定义 |

### 6.2 GameplayTag 分类

| 分类 | 示例 | 用途 |
|------|------|------|
| State.* | State.Dead, State.Stunned | 角色状态 |
| Status.* | Status.Buff, Status.Debuff | 效果状态 |
| Ability.* | Ability.Active, Ability.Ultimate | 技能类型 |
| Cooldown.* | Cooldown.Skill01 | 冷却管理 |
| Event.* | Event.Damage, Event.Heal | 事件触发 |
| Element.* | Element.Metal, Element.Wood | 元素类型 |
| Zodiac.* | Zodiac.Dragon, Zodiac.Tiger | 生肖类型 |

---

## 7. 网络安全

### 7.1 RPC 验证

所有客户端→服务端 RPC 使用 `WithValidation` 验证：

```cpp
UFUNCTION(Server, Reliable, WithValidation)
void ServerPing(FVector_NetQuantize10 Location);

// 验证: 距离检查 (MaxPingDistance = 50000)
bool ServerPing_Validate(FVector_NetQuantize10 Location);
```

### 7.2 目标验证

`IsValidTarget` 通过 `IDBATeamAgentInterface` 检查队伍敌我关系：

```cpp
// 中立目标 (TeamId = -1) 不能被作为敌人
// 不同团队 = 敌人
// 同团队 = 非敌人
```

### 7.3 Authority 检查

```cpp
if (GetOwnerRole() != ROLE_Authority)
{
    return; // 仅服务端执行
}
```

---

## 8. 日志系统

### 8.1 LogChannel 配置

| Channel | 用途 |
|---------|------|
| LogDBACore | 核心系统日志 |
| LogDBACombat | 战斗系统日志 |
| LogDBAUI | UI 系统日志 |
| LogDBAData | 数据系统日志 |

### 8.2 使用示例

```cpp
UE_LOG(LogDBACombat, Log, TEXT("[Ability] 技能激活 - %s"), *AbilityTag.ToString());
UE_LOG(LogDBACombat, Warning, TEXT("[Ability] 能量不足"));
```

---

## 9. 关键设计模式

### 9.1 Subsystem 模式

项目大量使用 Subsystem 管理不同生命周期：

```
UGameInstanceSubsystem
├── DBAStartupVideoSubsystem      # 启动视频
├── DBAFrontendSessionSubsystem   # 前台会话
├── DBAQueueSubsystem             # 匹配队列
└── DBACharacterRosterSubsystem   # 角色列表

ULocalPlayerSubsystem
└── DBAInputRouterComponent       # 输入路由 (组件不是 Subsystem)

UWorldSubsystem
└── DBAObjectPoolSubsystem       # 对象池
```

### 9.2 DECLARE_EVENT 模式

UI 事件使用 `DECLARE_EVENT` 避免依赖：

```cpp
DECLARE_EVENT_OneParam(UDBAInputRouterComponent, FDBAOnPingTriggered, FVector)
FDBAOnPingTriggered OnPingTriggeredEvent;
```

### 9.3 数据驱动设计

技能和属性通过 DataAsset 配置，而非硬编码：

```cpp
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
EDBAAbilityInputID InputID;

UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
float EnergyCost;
```

---

## 10. 待完善功能

| 功能 | 位置 | 说明 |
|------|------|------|
| 视线检测 | DBAInputRouterComponent::ServerLockTarget_Validate | 高级锁定 |
| 交互系统 | DBAInputRouterComponent::OnInteractTriggered | 完整交互检测 |
| 护盾机制 | DBABattleAttributeSet | 作为独立系统实现 |

---

*文档版本: 1.0*
*生成时间: 2026-04-27*
*引擎版本: UE 5.7.1*
