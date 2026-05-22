# 技能系统需求规格说明书

> 项目: DivineBeastsArena 神兽竞技场
> 版本: 0.2.0
> 日期: 2026-05-05

---

## 1. 概述

### 1.1 目的

本文档定义神兽竞技场游戏技能系统的功能需求和非功能需求，作为技能系统设计和实现的依据。

### 1.2 范围

本文档涵盖以下技能系统范围：
- 技能分类与结构
- 技能激活与取消
- 技能效果与伤害计算
- 冷却管理

---

## 2. 系统架构

### 2.1 技能继承体系

```
UDBAMobaGameplayAbilityBase (GameMoba层 - MOBA技能基类)
    │
    ├── UDBAZodiacAbilityBase (GameDBA层 - 生肖技能基类)
    │       │
    │       ├── UDBAElementAbilityBase (元素主动技能)
    │       │       └── 12生肖 × 4主动技能 (E/Q/W/R)
    │       │
    │       └── UDBAZodiacUltimateAbilityBase (终极技能)
    │               └── 12生肖 × 1终极技能
    │
    └── (自动生成技能)
            └── 12生肖 × 5技能 (Passive + E/Q/W/R)

总计: 12生肖 × 6技能 = 72个技能
```

### 2.2 技能分类

| 技能类型 | 数量 | 触发方式 | 能量消耗 |
|----------|------|----------|----------|
| Passive (被动) | 12 | 自动触发 | 无 |
| E (技能1) | 12 | 手动触发 | 有 |
| Q (技能2) | 12 | 手动触发 | 有 |
| W (技能3) | 12 | 手动触发 | 有 |
| R (技能4) | 12 | 手动触发 | 有 |
| Ultimate (终极) | 12 | 手动触发+能量要求 | 高能量 |

---

## 3. 功能需求

### 3.1 技能激活

#### 3.1.1 激活条件检查

**前置检查 (CanActivateAbility)**
1. 能量检查: `CurrentEnergy >= EnergyCost`
2. 冷却检查: `CooldownRemaining <= 0`
3. 目标有效性: `Target != nullptr && IsValid(Target)`
4. 施法范围: `Distance <= CastRange`
5. 游戏状态: `EDBAGameModeState == Playing`

#### 3.1.2 激活流程

```
客户端                    服务端
  │                         │
  ├─ ServerTryActivate ───>│
  │                         ├─ 验证激活条件
  │                         ├─ 扣减能量
  │                         ├─ 启动冷却
  │                         ├─ 执行技能效果
  │                         ├─ 广播GameplayCue
  │<─ ClientAbilityActivated─┤
  │                         │
```

#### 3.1.3 能量消耗

**成本提交 (CommitAbilityCost)**
- 检查当前能量 >= 能量消耗
- 扣减能量: `CurrentEnergy -= EnergyCost`
- 使用 GameplayEffect 实现能量消耗

### 3.2 技能效果

#### 3.2.1 伤害计算

**伤害公式**:
```
FinalDamage = BaseDamage × ElementMultiplier × ResonanceBonus × ChainBonus

其中:
- BaseDamage: 技能基础伤害 (来自DataTable)
- ElementMultiplier: 元素克制修正 (1.0 / 0.8 / 0.5)
- ResonanceBonus: 共鸣加成 (1.0 ~ 1.2)
- ChainBonus: 连锁加成 (1.0 ~ 1.35)
```

#### 3.2.2 元素克制

| 攻击元素 | 被克元素 | 伤害倍率 |
|----------|----------|----------|
| 金 (Gold) | 木 (Wood) | 1.2 |
| 木 (Wood) | 土 (Earth) | 1.2 |
| 土 (Earth) | 水 (Water) | 1.2 |
| 水 (Water) | 火 (Fire) | 1.2 |
| 火 (Fire) | 金 (Gold) | 1.2 |
| 其他 | - | 1.0 |

#### 3.2.3 共鸣加成

| 共鸣等级 | 同元素技能数 | 伤害加成 |
|----------|-------------|----------|
| Lv.1 | 2 | +5% |
| Lv.2 | 3 | +10% |
| Lv.3 | 4 | +15% |
| Lv.4 | 5 | +20% |

#### 3.2.4 连锁加成

| 连锁等级 | 伤害加成 | 触发条件 |
|----------|----------|----------|
| 1-5 | 120% | 6连击 |
| 6-9 | 135% | 10连击 |
| 10 | 20%最大生命 | 终结 |

### 3.3 技能冷却

#### 3.3.1 独立冷却

- 每个技能独立管理冷却时间
- 不存在共享冷却组
- 冷却时间由技能数据表定义

#### 3.3.2 冷却结构

```cpp
struct FDBAAbilityCooldown
{
    float RemainingTime;    // 剩余冷却时间
    float TotalTime;        // 总冷却时间
};
```

### 3.4 技能取消

#### 3.4.1 取消条件

- 玩家主动取消 (移动、转身等)
- 目标死亡或消失
- 超出施法范围

#### 3.4.2 取消流程

```
客户端                    服务端
  │                         │
  ├─ ServerCancelAbility ─>│
  │                         ├─ 中止当前技能
  │                         ├─ 返还部分冷却 (可选)
  │<─ ClientAbilityFailed ──┤
```

---

## 4. 数据结构

### 4.1 技能数据表 (DataTable)

**DBASkillDataRow**
```cpp
struct DIVINEBEASTSARENA_API FDBASkillDataRow : public FTableRowBase
{
    FName SkillId;              // 技能ID
    EDBAZodiacType Zodiac;     // 所属生肖
    EDBAElement Element;       // 技能元素
    float BaseDamage;          // 基础伤害
    float EnergyCost;          // 能量消耗
    float Cooldown;            // 冷却时间
    float CastRange;          // 施法范围
    FGameplayTagContainer GrantedTags;     // 授予标签
    FGameplayTagContainer ActivationTags;  // 激活标签
};
```

### 4.2 技能规格句柄

```cpp
FGameplayAbilitySpec
├── Handle (唯一标识)
├── Ability (技能类)
├── Level (技能等级) = 1 (固定)
├── ActiveCount (活跃计数)
└── CooldownEndTime (冷却结束时间)
```

---

## 5. 非功能需求

### 5.1 性能指标

| 指标 | 要求 |
|------|------|
| 技能激活响应 | < 50ms (客户端预测) |
| 服务端验证延迟 | < 20ms |
| 技能Tick频率 | 按需 (非持续性) |

### 5.2 可靠性

| 需求 | 说明 |
|------|------|
| 服务端权威 | 所有伤害判定在服务端执行 |
| 乐观更新 | 客户端先播放效果，服务端验证 |
| 失败回滚 | 验证失败时撤销客户端效果 |

---

## 6. 用户故事

### 6.1 技能激活
```
作为玩家，我希望按下技能键后技能立即响应
以便获得流畅的操作手感

验收标准:
- 客户端激活后立即播放特效
- 服务端验证后同步最终命中结果
```

### 6.2 能量管理
```
作为玩家，我需要合理规划能量使用
以便在关键时刻释放技能

验收标准:
- 能量不足时技能按钮变灰
- 能量消耗实时显示
```

### 6.3 冷却管理
```
作为玩家，我希望了解技能冷却状态
以便规划下一步操作

验收标准:
- 冷却中显示倒计时
- 冷却结束有视觉提示
```

---

## 7. Open Questions

| 问题 | 选项 | 决策 |
|------|------|------|
| 技能图标 | 静态 / 动态 | 静态 |
| 施法指示器 | 圆形 / 扇形 / 线性 | 根据技能类型 |
| 技能音效 | 必须 / 可选 | 必须 |

---

## 8. 参考文档

- DBAElementAbilityBase.h - 元素技能基类
- DBAZodiacAbilityBase.h - 生肖技能基类
- DBABattleAttributeSet.h - 战斗属性
- NetworkSync_Requirements.md - 网络同步需求

---

*文档生成时间: 2026-05-05*