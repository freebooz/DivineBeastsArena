# DivineBeastsArena 神兽竞技场 - 项目概览

> 版本: 0.2.0 | 引擎: Unreal Engine 5.7.1 | 架构: GameCore → GameMoba → GameDBA 三层模块化

---

## 目录

1. [项目简介](#项目简介)
2. [核心概念](#核心概念)
3. [三层架构](#三层架构)
4. [核心系统](#核心系统)
5. [模块说明](#模块说明)

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

| 阵营 | 描述 |
|------|------|
| 东方 | 神秘东方力量 |
| 西方 | 西方魔法世界 |
| 南方 | 火焰南方帝国 |
| 北方 | 冰霜北境王国 |
| 中央 | 平衡中立区域 |

---

## 三层架构

项目采用 **GameCore → GameMoba → GameDBA** 三层架构：

```
┌─────────────────────────────────────────────────────────────┐
│                    GameDBA (游戏专用层)                       │
│  神兽竞技场专属内容：十二生肖、五行元素、五大阵营、共鸣系统      │
├─────────────────────────────────────────────────────────────┤
│                      GameMoba (MOBA框架层)                    │
│  MOBA 游戏通用逻辑：技能系统、战斗属性、输入路由、UI 框架      │
├─────────────────────────────────────────────────────────────┤
│                      GameCore (通用核心层)                    │
│  游戏通用基础组件：子系统框架、日志系统、对象池、账户系统      │
└─────────────────────────────────────────────────────────────┘
```

### GameCore 层 (通用核心层)

位置: `Source/DivineBeastsArena/Public/GameCore/`

| 模块 | 功能 |
|------|------|
| Account | 账户系统 (DBAAccountServiceBase, DBAMockAccountService) |
| Data | 通用数据资产 (DBADataAssetBase) |
| ObjectPool | 对象池 (DBAObjectPoolSubsystem) |
| Party | 队伍系统 (DBAPartyServiceBase) |
| Queue | 队列系统 (DBAQueueServiceBase) |
| Session | 会话管理 (DBAFrontendSessionSubsystem) |
| Subsystems | 子系统框架 (DBASubsystemBase, DBAWorldSubsystemBase) |
| Types | 通用类型定义 (DBACommonEnums, DBACommonTypes) |
| UI | 通用 UI 基类 (DBAUserWidgetBase, DBAWidgetController) |

### GameMoba 层 (MOBA框架层)

位置: `Source/DivineBeastsArena/Public/GameMoba/`

| 模块 | 功能 |
|------|------|
| Data | MOBA 数据资产 (DBAMobaAbilitySetData, DBAInputConfigDataAsset) |
| Framework | MOBA 游戏框架 (DBAMobaGameModeBase, DBAGameStateBase) |
| GAS | MOBA 能力系统 (DBAMobaAbilitySystemComponentBase, DBAMobaGameplayAbilityBase) |
| Input | 输入系统 (DBAInputPlatformPolicy, DBAInputRouterComponent) |
| RPC | 网络 RPC (DBARpcInterface, DBARpcServer, DBARpcClient) |
| UI | MOBA UI 基类 (UDBAMobaUserWidgetBase, UDBAMobaWidgetControllerBase) |

### GameDBA 层 (游戏专用层)

位置: `Source/DivineBeastsArena/Public/GameDBA/`

| 模块 | 功能 |
|------|------|
| Animation | 生肖动画配置 (DBAZodiacAnimInstance, DBAZodiacAnimConfig_*) |
| Audio | 音频资源管理 |
| Character | 角色定义 (DBAZodiacCharacterBase) |
| Combat | 战斗系统 (DBASkillProjectileBase, DBAProjectile_*) |
| Core | 游戏核心类型 (DBAConstants, DBAEnumsCore, DBAGameplayTags) |
| Data | 游戏数据表 (DBAZodiacHeroData, DBASkillDataRow) |
| GAS | 游戏技能实现 (DBAElementAbilityBase, DBAZodiacAbilityBase) |
| Input | 游戏输入处理 (DBAAndroidTouchInputBridge) |
| UI | 游戏界面 (UDBAArenaHUDRootWidgetBase, 大厅/选择界面) |
| VFX | 视觉特效 (DBASkillVFXManager, DBAZodiacSkillVFXComponent_*) |

---

## 核心系统

### GameplayAbilitySystem (GAS)

项目使用 Unreal Engine 的 GameplayAbilitySystem 作为核心技能框架：

```
GameDBA 层
├── DBAZodiacAbilityBase (生肖技能基类)
│     └── 12生肖各自的技能实现
├── DBAElementAbilityBase (元素主动技能)
│     └── E/Q/W/R 四个主动技能
└── DBAResonanceAbilityBase (共鸣技能)

GameMoba 层
├── DBAMobaGameplayAbilityBase (MOBA技能基类)
└── DBAMobaAbilitySystemComponentBase (能力系统组件)
```

### 技能结构

每个生肖英雄拥有以下技能槽位：

| 槽位 | 类型 | 说明 |
|------|------|------|
| Passive | 被动技能 | 生肖专属被动 |
| E | 主动技能1 | 元素技能 |
| Q | 主动技能2 | 元素技能 |
| W | 主动技能3 | 元素技能 |
| R | 主动技能4 | 元素技能 |

---

## 枚举系统

### 游戏核心枚举 (GameDBA/Core)

```cpp
EDBAElementType - 五行元素
EDBAGameModeState - 游戏状态
EDBASkillSlot - 技能槽位
```

### MOBA框架枚举 (GameMoba)

```cpp
EDBAMobaInputAction - MOBA输入动作
EDBAPlatformType - 平台类型
```

---

## 文件结构

```
Source/DivineBeastsArena/
├── Public/
│   ├── GameCore/                    # L1 通用核心层
│   │   ├── Account/
│   │   ├── Data/
│   │   ├── ObjectPool/
│   │   ├── Party/
│   │   ├── Queue/
│   │   ├── Session/
│   │   ├── Subsystems/
│   │   ├── Types/
│   │   └── UI/
│   ├── GameMoba/                    # L2 MOBA框架层
│   │   ├── Data/
│   │   ├── Framework/
│   │   ├── GAS/
│   │   ├── Input/
│   │   ├── RPC/
│   │   └── UI/
│   └── GameDBA/                     # L3 游戏专用层
│       ├── Animation/
│       ├── Audio/
│       ├── Character/
│       ├── Combat/
│       ├── Core/
│       ├── Data/
│       ├── GAS/Abilities/
│       ├── Input/
│       ├── UI/
│       └── VFX/
├── Private/
│   ├── GameCore/
│   ├── GameMoba/
│   └── GameDBA/
└── Docs/
```

---

## 开发指南

### 添加新技能

1. 在 `GameDBA/GAS/Abilities/` 创建能力类，继承 `DBAElementAbilityBase`
2. 重写 `ActivateAbility()` 实现技能逻辑
3. 使用 Python 脚本生成十二生肖的技能变体

### 添加新子系统

1. 在 `GameCore/Subsystems/` 创建头文件
2. 继承 `UGameInstanceSubsystem` 或 `UWorldSubsystem`
3. 混入 `DBASubsystemImpl` 获取日志功能

### 层级依赖规则

- **GameDBA** 可依赖 GameCore 和 GameMoba
- **GameMoba** 仅可依赖 GameCore
- **GameCore** 不可依赖 GameMoba 或 GameDBA

---

*文档生成时间: 2026-05-05*