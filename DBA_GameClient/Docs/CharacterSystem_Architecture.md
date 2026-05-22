# 角色系统架构设计文档

> 项目: DivineBeastsArena 神兽竞技场
> 版本: 0.2.0
> 日期: 2026-05-05

---

## 1. 系统概述

### 1.1 目的

定义神兽竞技场游戏角色系统的架构设计，包括角色分类、继承关系、组件构成、网络同步等。

### 1.2 角色分类

| 角色类型 | 说明 | 数量 |
|----------|------|------|
| Zodiac (生肖) | 玩家控制的英雄角色 | 12 |
| Monster (怪物) | AI控制的野怪/小兵 | 5种 |
| Guardian (守卫) | 防御建筑 (塔/水晶/雕像) | 3种 |

---

## 2. 架构设计

### 2.1 角色继承体系

```
UE原生基类
└── AActor / ACharacter
        │
        ├── ADBAMonsterBase (怪物基类)
        │       ├── ADBAMonster_Slime
        │       ├── ADBAMonster_Ghost
        │       ├── ADBAMonster_Golem
        │       ├── ADBAMonster_Imp
        │       └── ADBAMonster_Skeleton
        │
        ├── ADBAGuardianBase (守卫基类)
        │       ├── ADBAGuardian_Tower (防御塔)
        │       ├── ADBAGuardian_Crystal (水晶)
        │       └── ADBAGuardian_Statue (雕像)
        │
        └── ADBAZodiacCharacterBase (生肖角色基类) ← 玩家角色
                ├── ADBAZodiacCharacter_Rat
                ├── ADBAZodiacCharacter_Ox
                ├── ADBAZodiacCharacter_Tiger
                └── ... (12生肖)
```

### 2.2 组件架构

#### 玩家角色 (DBAZodiacCharacterBase)

| 组件 | 类型 | 用途 |
|------|------|------|
| AbilitySystemComponent | UDBAAbilitySystemComponent | GAS技能系统 |
| RpcHandler | ADBARpcHandler | 网络RPC处理 |
| CharacterMovementComponent | UCharacterMovementComponent | 移动控制 |
| CapsuleComponent | UCapsuleComponent | 碰撞检测 |
| Mesh | USkeletalMeshComponent | 角色模型 |
| AnimInstance | UDBAZodiacAnimInstance | 动画控制 |

#### 怪物角色 (DBAMonsterBase)

| 组件 | 类型 | 用途 |
|------|------|------|
| AbilitySystemComponent | UAbilitySystemComponent | 技能/属性 |
| AIController | ADBAMonsterAIController | AI控制 |
| CharacterMovementComponent | UCharacterMovementComponent | 移动控制 |
| CapsuleComponent | UCapsuleComponent | 碰撞检测 |

#### 守卫角色 (DBAGuardianBase)

| 组件 | 类型 | 用途 |
|------|------|------|
| AbilitySystemComponent | UAbilitySystemComponent | 技能/属性 |
| StaticMeshComponent | UStaticMeshComponent | 守卫模型 |
| BoxComponent | UBoxComponent | 攻击范围检测 |

### 2.3 核心接口

#### IDBARpcInterface (RPC接口)
```cpp
// 服务端方法 (客户端调用)
ServerTryActivateAbility(Params)
ServerCancelAbility(Handle)
ServerMoveTo(Location)
ServerLockTarget(TargetActor)

// 客户端回调 (服务端调用)
ClientReportHit(AbilityHandle, HitLocation, HitActor)
ClientFullStateSync(Health, Energy, Shield, ...)
ClientMoveCorrection(ServerLocation, ServerTime)
ClientHitConfirmed(AbilityHandle, Damage, DamageType)
ClientHitRejected(AbilityHandle)
```

---

## 3. 角色配置

### 3.1 生肖角色配置

| 配置项 | 类型 | 说明 |
|--------|------|------|
| ZodiacType | EDBAZodiacType | 生肖类型 (12种) |
| ElementType | EDBAElement | 元素类型 (5种+None) |
| AnimBlueprint | UClass* | 动画蓝图类 |
| DefaultAbilities | TArray<TSubclassOf<UGameplayAbility>> | 默认技能 |
| AttributeSet | UClass* | 属性集类 |

### 3.2 怪物配置

| 配置项 | 类型 | 说明 |
|--------|------|------|
| MonsterType | FName | 怪物类型 |
| Level | int32 | 等级 |
| MaxHealth | float | 最大生命 |
| AttackDamage | float | 攻击力 |
| MoveSpeed | float | 移动速度 |
| AIBehaviorTree | UBehaviorTree* | AI行为树 |

### 3.3 守卫配置

| 配置项 | 类型 | 说明 |
|--------|------|------|
| GuardianType | FName | 守卫类型 |
| MaxHealth | float | 最大生命 |
| AttackDamage | float | 攻击力 |
| AttackRange | float | 攻击范围 |
| AttackInterval | float | 攻击间隔 |

---

## 4. 网络同步设计

### 4.1 复制策略

| 数据 | 复制模式 | 频率 |
|------|----------|------|
| ActorLocation | DOREPLIFETIME | 10-30Hz |
| ActorRotation | DOREPLIFETIME | 10Hz |
| Health | DOREPLIFETIME | 按需 |
| Energy | DOREPLIFETIME | 按需 |
| ChainLevel | DOREPLIFETIME | 按需 |
| UltimateEnergy | DOREPLIFETIME | 每tick |

### 4.2 客户端预测

```
客户端输入 → 本地预测 → 发送RPC → 服务端验证 → 校正回调
```

**预测内容**:
- 移动位置
- 技能激活
- 动画播放

**校正内容**:
- 服务端权威位置
- 技能激活结果
- 属性变化

---

## 5. 目录结构

```
GameDBA/Character/
├── DBAZodiacCharacterBase.h/cpp      (生肖角色基类)
├── Zodiac/
│   ├── DBAZodiacCharacter_Rat.h/cpp
│   ├── DBAZodiacCharacter_Ox.h/cpp
│   └── ... (12生肖)
├── Monster/
│   ├── DBAMonsterBase.h/cpp          (怪物基类)
│   ├── DBAMonster_Slime.h/cpp
│   ├── DBAMonster_Ghost.h/cpp
│   └── ... (5种怪物)
└── Guardian/
    ├── DBAGuardianBase.h/cpp         (守卫基类)
    ├── DBAGuardian_Tower.h/cpp
    ├── DBAGuardian_Crystal.h/cpp
    └── DBAGuardian_Statue.h/cpp
```

---

## 6. 实现优先级

| 优先级 | 内容 | 说明 |
|--------|------|------|
| P0 | ZodiacCharacterBase完善 | 玩家角色核心功能 |
| P1 | 怪物AI系统 | 野怪行为 |
| P2 | 守卫攻击系统 | 防御建筑 |
| P3 | 角色外观/动画 | 美术资源 |

---

## 7. 参考文件

- `GameDBA/Character/DBAZodiacCharacterBase.h`
- `GameDBA/Character/Monster/DBAMonsterBase.h`
- `GameDBA/Character/Guardian/DBAGuardianBase.h`
- `GameMoba/RPC/DBARpcHandler.h`

---

*文档生成时间: 2026-05-05*