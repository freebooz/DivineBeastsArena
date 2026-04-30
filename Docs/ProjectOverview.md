# DivineBeastsArena 神兽竞技场 - 项目概览

> 版本: 0.1.0 | 引擎: Unreal Engine 5.7.1 | 架构: 三层模块化

---

## 目录

1. [项目简介](#项目简介)
2. [核心概念](#核心概念)
3. [三层架构](#三层架构)
4. [核心系统](#核心系统)
5. [模块说明](#模块说明)
6. [枚举系统](#枚举系统)
7. [常量配置](#常量配置)

---

## 项目简介

**DivineBeastsArena（神兽竞技场）** 是一款基于 Unreal Engine 5.7.1 开发的多人在线竞技游戏（MOBA）。

### 技术栈

| 组件 | 技术 |
|------|------|
| 引擎 | Unreal Engine 5.7.1 (源码版) |
| 编程语言 | C++ / Blueprint |
| 核心框架 | GameplayAbilitySystem (GAS) |
| 网络模型 | 客户端预测 + 服务端权威 |
| UI 框架 | UMG (Unreal Motion Graphics) |

---

## 核心概念

### 五大自然元素

游戏中的自然元素系统，包含五行相克关系：

```
火 → 金 → 木 → 土 → 水 → 火
     ↓
    相克
```

| 元素 | 特征 | 克制 |
|------|------|------|
| 金 (Gold) | 锋利坚固，擅长物理攻击和防御 | 克木 |
| 木 (Wood) | 生发成长，擅长持续治疗和增益 | 克土 |
| 水 (Water) | 柔韧变化，擅长控制和解控 | 克火 |
| 火 (Fire) | 炽热爆发，擅长范围伤害 | 克金 |
| 土 (Earth) | 沉稳厚重，擅长防护和生存 | 克水 |

### 十二生肖

游戏中的生肖系统，每个生肖拥有独特的被动技能：

鼠、牛、虎、兔、龙、蛇、马、羊、猴、鸡、狗、猪

### 五大阵营

| 阵营 | 描述 | 主题 |
|------|------|------|
| 东方 | 神秘东方力量 | 青色、东方哲学 |
| 西方 | 西方魔法世界 | 金色、光明魔法 |
| 南方 | 火焰南方帝国 | 红色、烈焰 |
| 北方 | 冰霜北境王国 | 蓝色、冰雪 |
| 中央 | 平衡中立区域 | 紫色、混沌 |

### 共鸣系统

当英雄装备的技能中，同元素技能达到一定数量时触发：

| 共鸣等级 | 同元素技能数 | 效果 |
|----------|-------------|------|
| Lv.1 | 2 | +5% 伤害，+2% 防御 |
| Lv.2 | 3 | +10% 伤害，+4% 防御 |
| Lv.3 | 4 | +15% 伤害，+6% 防御 |
| Lv.4 | 5 | +20% 伤害，+8% 防御 |

### 连锁系统

连续命中敌人触发连锁，伤害递增：

| 连锁等级 | 伤害加成 | 触发条件 |
|----------|----------|----------|
| 1-5 | 120% | 6+ 连击 |
| 6-9 | 135% | 10 连击 |
| 10 (终结) | 20% 生命值 | 连锁终结技 |

---

## 三层架构

项目采用 **Common → MobaBase → DivineBeastsArena** 三层架构：

```
┌─────────────────────────────────────────────────────────────┐
│                    DivineBeastsArena (游戏层)                 │
│  神兽竞技场专属内容：十二生肖、五行元素、五大阵营、共鸣系统      │
├─────────────────────────────────────────────────────────────┤
│                      MobaBase (MOBA 层)                      │
│  MOBA 游戏通用逻辑：技能系统、战斗属性、输入路由、UI 框架      │
├─────────────────────────────────────────────────────────────┤
│                       Common (通用层)                         │
│  游戏无关通用组件：子系统框架、日志系统、对象池、账户系统      │
└─────────────────────────────────────────────────────────────┘
```

### Common 层 (通用层)

位置: `Source/DivineBeastsArena/Public/Common/`

| 模块 | 功能 |
|------|------|
| Subsystems | 子系统框架 (DBASubsystemBase, DBAGameInstanceSubsystemBase) |
| UI | 通用 UI 基类 (DBAUserWidgetBase, DBAWidgetController) |
| Account | 账户系统 (DBAAccountServiceBase, DBAMockAccountService) |
| Party | 队伍系统 (DBAPartyServiceBase) |
| Queue | 队列系统 (DBAQueueServiceBase) |
| Session | 会话管理 (DBAFrontendSessionSubsystem) |
| ObjectPool | 对象池 (DBAObjectPoolSubsystem) |
| Types | 通用类型定义 (DBACommonEnums, DBACommonTypes) |
| Log | 日志通道 (DBALogChannels) |

### MobaBase 层 (MOBA 层)

位置: `Source/DivineBeastsArena/Public/MobaBase/`

| 模块 | 功能 |
|------|------|
| Framework | MOBA 游戏框架 (DBAMobaGameModeBase, DBAGameStateBase) |
| GAS | MOBA 能力系统 (DBAMobaAbilitySystemComponentBase, DBAMobaGameplayAbilityBase) |
| Data | MOBA 数据资产 (DBAMobaAbilitySetData, DBAInputConfigDataAsset) |
| Input | 输入系统 (DBAInputPlatformPolicy, DBAInputRouterComponent) |
| UI | MOBA UI 基类 (UDBAMobaUserWidgetBase, UDBAMobaWidgetControllerBase) |

### DivineBeastsArena 层 (游戏层)

位置: `Source/DivineBeastsArena/Public/`

| 模块 | 功能 |
|------|------|
| GAS/Abilities | 元素/生肖能力 (DBAElementAbilityBase, DBAZodiacAbilityBase) |
| GAS/Attributes | 战斗属性 (DBABattleAttributeSet, DBAHeroGrowthAttributeSet) |
| GAS/Effects | 游戏效果 (DBEEnergyCostEffect, DBEResonanceBuffEffect) |
| Frontend | 前端系统 (英雄选择、元素选择、五阵营选择) |
| UI/Arena | 战斗界面 (技能栏、状态栏、背包) |

---

## 核心系统

### GameplayAbilitySystem (GAS)

项目使用 Unreal Engine 的 GameplayAbilitySystem 作为核心技能框架：

```
Ability (技能)
    ├── DBAElementAbilityBase (元素主动技能)
    │     ├── Skill01-04 (主动技能1-4)
    │     └── EnergyCost (能量消耗)
    ├── DBAZodiacAbilityBase (生肖技能)
    └── DBAResonanceAbilityBase (共鸣技能)

AttributeSet (属性集)
    └── DBABattleAttributeSet
          ├── AttackPower (攻击力)
          ├── Defense (防御力)
          ├── MaxHealth (最大生命)
          ├── CurrentHealth (当前生命)
          ├── MaxShield (最大护盾)
          ├── CurrentShield (当前护盾)
          ├── CurrentEnergy (当前能量)
          └── MaxEnergy (最大能量)

GameplayEffect (游戏效果)
    ├── DBEEnergyCostEffect (能量消耗)
    └── DBEResonanceBuffEffect (共鸣增益)
```

### 技能结构

每个英雄拥有以下技能槽位：

| 槽位 | 类型 | 输入绑定 |
|------|------|----------|
| Passive | 被动技能 | 无 |
| Skill01 | 主动技能1 | InputID 4 |
| Skill02 | 主动技能2 | InputID 5 |
| Skill03 | 主动技能3 | InputID 6 |
| Skill04 | 主动技能4 | InputID 7 |
| Ultimate | 生肖终极技 | InputID Ultimate |

### 网络复制

| 属性 | 复制模式 |
|------|----------|
| UltimateEnergy | DOREPLIFETIME |
| ChainLevel | DOREPLIFETIME |
| ResonanceLevel | DOREPLIFETIME |

---

## 模块说明

### 子系统 (Subsystems)

项目使用 UE 的子系统框架管理不同功能域：

| 子系统 | 父类 | 功能 |
|--------|------|------|
| DBAStartupVideoSubsystem | GameInstanceSubsystem | 启动视频播放 |
| DBAElementSelectSubsystem | GameInstanceSubsystem | 元素选择管理 |
| DBAFiveCampSelectSubsystem | GameInstanceSubsystem | 五阵营选择管理 |
| DBAHeroSelectSubsystem | GameInstanceSubsystem | 英雄选择管理 |
| DBAQueueSubsystem | GameInstanceSubsystem | 匹配队列管理 |
| DBALobbySubsystem | GameInstanceSubsystem | 大厅管理 |
| DBAPartySubsystem | GameInstanceSubsystem | 队伍管理 |

### 前端流程

```
启动视频 → 主大厅 → 英雄选择 → 元素选择 → 五阵营选择 → 匹配 → 对局
```

### 日志系统

项目定义了专用日志通道：

| 通道 | 用途 |
|------|------|
| LogDBACore | 核心系统日志 |
| LogDBACombat | 战斗系统日志 |
| LogDBAUI | UI 系统日志 |
| LogDBAData | 数据系统日志 |

---

## 枚举系统

### 游戏状态枚举

```cpp
EDBAGameModeState
├── None
├── Waiting      // 等待中
├── Playing      // 游戏中
├── Paused       // 已暂停
└── GameOver     // 游戏结束
```

### 技能槽位枚举

```cpp
EDBASkillSlot
├── None
├── Passive      // 被动技能
├── Active1      // 主动技能1
├── Active2      // 主动技能2
├── Active3      // 主动技能3
├── Active4      // 主动技能4
└── Ultimate     // 终极技能
```

---

## 常量配置

所有游戏常量集中在 `DBAConstants` 命名空间：

### 技能系统

| 常量 | 值 | 说明 |
|------|-----|------|
| CoreCombatInputCount | 6 | 核心战斗输入数量 |
| ActiveSkillCount | 4 | 主动技能数量 |
| MaxSkillLevel | 5 | 最大技能等级 |
| MaxHeroLevel | 18 | 最大英雄等级 |

### 共鸣系统

| 常量 | 值 | 说明 |
|------|-----|------|
| ResonanceLevel1_SkillCount | 2 | 共鸣1级需求 |
| ResonanceLevel2_SkillCount | 3 | 共鸣2级需求 |
| ResonanceLevel3_SkillCount | 4 | 共鸣3级需求 |
| ResonanceLevel4_SkillCount | 5 | 共鸣4级需求 |

### 连锁系统

| 常量 | 值 | 说明 |
|------|-----|------|
| MaxChainLevel | 10 | 最大连锁等级 |
| ChainTimeout | 6.0s | 连锁超时时间 |
| ChainTier1Threshold | 6 | 连锁1阶门槛 |
| ChainTier2Threshold | 10 | 连锁2阶门槛 |

### 终极能量

| 常量 | 值 | 说明 |
|------|-----|------|
| MaxUltimateEnergy | 100 | 最大终极能量 |
| UltimateEnergy_SkillHit | 3 | 技能命中获得 |
| UltimateEnergy_HeroKill | 20 | 击杀英雄获得 |
| UltimateEnergy_PassiveRegen | 1/s | 被动回复速度 |

---

## 文件结构

```
Source/DivineBeastsArena/
├── Public/
│   ├── Common/                    # 通用层
│   │   ├── Account/
│   │   ├── ObjectPool/
│   │   ├── Party/
│   │   ├── Queue/
│   │   ├── Session/
│   │   ├── Subsystems/
│   │   ├── Types/
│   │   └── UI/
│   ├── MobaBase/                  # MOBA 层
│   │   ├── Data/
│   │   ├── Framework/
│   │   ├── GAS/
│   │   ├── Input/
│   │   └── UI/
│   ├── GAS/                       # 游戏层 - GAS
│   │   ├── Abilities/
│   │   ├── Attributes/
│   │   └── Effects/
│   ├── Lobby/                  # 游戏层 - 前端
│   │   ├── BuildValidation/
│   │   ├── ElementSelect/
│   │   ├── FiveCampSelect/
│   │   ├── FixedSkillGroup/
│   │   ├── HeroSelect/
│   │   ├── Invite/
│   │   ├── Lobby/
│   │   ├── LoadingPreparation/
│   │   ├── MatchEntry/
│   │   ├── Newbie/
│   │   ├── Party/
│   │   ├── Portal/
│   │   ├── Practice/
│   │   ├── Queue/
│   │   ├── ReadyCheck/
│   │   └── Startup/
│   └── UI/                        # 游戏层 - UI
│       ├── Arena/
│       └── Lobby/
└── Private/                       # 实现文件
```

---

## 开发指南

### 添加新能力

1. 在 `Public/GAS/Abilities/` 创建能力类，继承 `DBAElementAbilityBase`
2. 在构造函数中设置 `ElementType` 和 `EnergyCost`
3. 重写 `ActivateAbility()` 实现技能逻辑
4. 在蓝图中配置 `CostGameplayEffect`

### 添加新子系统

1. 在 `Public/Common/Subsystems/` 创建头文件
2. 继承 `UGameInstanceSubsystem` 或 `UWorldSubsystem`
3. 混入 `DBASubsystemImpl` 获取日志功能
4. 在 `Initialize()` 中进行初始化

### 添加新 UI 组件

1. 在 `Public/UI/` 创建 Widget 类
2. 继承 `DBAUserWidgetBase` 或特定子系统 Widget 基类
3. 创建对应的 WidgetController
4. 在蓝图中实现 `BP_` 前缀的交互事件

---

*文档生成时间: 2026-04-28*
