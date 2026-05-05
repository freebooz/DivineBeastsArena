# 战斗机制需求规格

> 项目: DivineBeastsArena 神兽竞技场
> 版本: 0.1.0
> 日期: 2026-05-05
> 状态: 待评审

---

## 1. 目标与范围

### 1.1 目标
实现完整的战斗机制系统，包括伤害计算、智能目标锁定和战斗属性应用。

### 1.2 范围
**包含:**
- 伤害计算增强（防御减免、暴击应用）
- 智能目标锁定系统
- 战斗属性应用接口
- 完整伤害流程

**不包含:**
- 技能具体释放逻辑（GAS层）
- 网络同步细节（已有框架）
- UI伤害数字显示

---

## 2. 功能需求

### 2.1 伤害计算增强

#### 2.1.1 最终伤害公式
```
FinalDamage = (BaseDamage × ElementMultiplier × (1 + ResonanceBonus) × ChainMultiplier)
           × (1 - PhysicalReduction) × CriticalMultiplier
```

**计算顺序:**
1. BaseDamage - 基础伤害
2. × ElementMultiplier - 元素克制 (1.2 / 1.0 / 0.8)
3. × (1 + ResonanceBonus) - 共鸣加成 (0 ~ 20%)
4. × ChainMultiplier - 连锁加成 (1.0 / 1.2 / 1.35 / 0.2MaxHP)
5. × (1 - PhysicalReduction) - 防御减免 (0 ~ 1)
6. × CriticalMultiplier - 暴击倍率 (1.0 / 2.0+)

#### 2.1.2 防御伤害减免
```cpp
PhysicalReduction = Defense / (Defense + 100)
// Defense = 0  →  0% 减免
// Defense = 50  →  33% 减免
// Defense = 100 →  50% 减免
// Defense = 200 →  67% 减免
```

#### 2.1.3 暴击判定
```cpp
bIsCritical = FMath::FRand() < CriticalRate
CriticalDamage = NormalDamage × CriticalMultiplier
```

### 2.2 智能目标锁定

#### 2.2.1 锁定策略
| 策略 | 条件 | 优先级 |
|------|------|--------|
| **智能锁定** | 自动选择威胁最高的目标 | P1 |
| **血量锁定** | 优先锁定低血量目标 | P2 |
| **距离锁定** | 优先锁定最近目标 | P3 |

#### 2.2.2 智能锁定算法
```
1. 获取范围内所有敌方目标
2. 按威胁值排序（基于仇恨系统）
3. 返回威胁值最高的目标
4. 无有效目标返回 nullptr
```

#### 2.2.3 锁定范围
- 默认锁定半径: 500
- 可配置属性: `AutoLockRadius`

### 2.3 战斗流程

#### 2.3.1 普通攻击流程
```
1. ServerRequestAttack() 调用
2. FindTarget() 获取锁定目标
3. CalculateDamage() 计算伤害
4. ApplyDamage() 应用伤害到目标
5. ClientReportHit() 通知客户端
6. 播放命中效果
```

#### 2.3.2 伤害应用接口
```cpp
UFUNCTION(BlueprintCallable, Category = "DBA|Combat")
void ApplyDamage(AActor* Target, float Damage, EDBAElement Element);

UFUNCTION(BlueprintCallable, Category = "DBA|Combat")
float CalculateFinalDamage(float BaseDamage, EDBAElement AttackElement,
                          EDBAElement DefenseElement, int32 ResonanceLevel,
                          int32 ChainLevel, float Defense, float CriticalRate,
                          float CriticalMultiplier);
```

---

## 3. 非功能需求

### 3.1 性能需求
- 伤害计算 < 0.1ms
- 目标锁定 < 1ms

### 3.2 网络同步
- 伤害判定仅服务端执行
- 客户端通过RPC接收命中结果

---

## 4. 技术方案

### 4.1 战斗计算器扩展

#### 4.1.1 DBADamageCalculator 新增方法
```cpp
// 计算最终伤害（包含防御和暴击）
UFUNCTION(BlueprintCallable, Category = "DBA|Combat")
static float CalculateFinalDamage(
    float BaseDamage,
    EDBAElement AttackElement,
    EDBAElement DefenseElement,
    int32 ResonanceLevel,
    int32 ChainLevel,
    float Defense,
    float CriticalRate,
    float CriticalMultiplier);

// 应用伤害到目标
UFUNCTION(BlueprintCallable, Category = "DBA|Combat")
static void ApplyDamageToTarget(
    AActor* Attacker,
    AActor* Target,
    float FinalDamage,
    EDBAElement Element,
    bool bIsCritical);
```

### 4.2 智能目标锁定组件

#### 4.2.1 UDBAAutoTargetComponent
```cpp
UCLASS(Blueprintable, BlueprintType, meta = (BlueprintSpawnableComponent))
class DIVINEBEASTSARENA_API UDBAAutoTargetComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // 锁定半径
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
    float AutoLockRadius = 500.0f;

    // 锁定策略
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
    EDBATargetLockStrategy LockStrategy = EDBATargetLockStrategy::Smart;

    // 获取当前锁定目标
    UFUNCTION(BlueprintCallable, Category = "Target")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    // 刷新目标（每帧调用）
    UFUNCTION(BlueprintCallable, Category = "Target")
    void RefreshTarget();

    // 锁定目标
    UFUNCTION(BlueprintCallable, Category = "Target")
    void LockTarget(AActor* NewTarget);

    // 解锁目标
    UFUNCTION(BlueprintCallable, Category = "Target")
    void UnlockTarget();

protected:
    AActor* FindSmartTarget();
    AActor* FindLowestHPTarget();
    AActor* FindNearestTarget();

private:
    UPROPERTY()
    AActor* CurrentTarget = nullptr;
};
```

#### 4.2.2 锁定策略枚举
```cpp
UENUM(BlueprintType)
enum class EDBATargetLockStrategy : uint8
{
    Smart      UMETA(DisplayName = "Smart", Tooltip = "智能锁定（威胁最高）"),
    LowestHP   UMETA(DisplayName = "LowestHP", Tooltip = "血量锁定（最低血量）"),
    Nearest    UMETA(DisplayName = "Nearest", Tooltip = "距离锁定（最近目标）")
};
```

### 4.3 目录结构

```
GameDBA/Combat/
├── DBADamageCalculator.h/cpp      (扩展)
├── DBACombatTypes.h               (新增：ETargetLockStrategy等)
└── Target/
    └── DBAAutoTargetComponent.h/cpp (新增)
```

---

## 5. 用户故事

| ID | 场景 | 预期行为 |
|----|------|----------|
| US-01 | 普通攻击 | 客户端发送ServerRequestAttack，服务端计算伤害并应用 |
| US-02 | 智能锁定 | 自动选择威胁值最高的目标作为CurrentTarget |
| US-03 | 防御减免 | 伤害经过防御力计算后减少 |
| US-04 | 暴击判定 | 暴击时伤害翻倍并播放暴击特效 |
| US-05 | 目标死亡 | 目标血量归零时触发死亡逻辑 |

---

## 6. 验收标准

### 6.1 功能验收
- [ ] 伤害计算包含防御减免
- [ ] 暴击判定正确触发
- [ ] 智能锁定选择威胁最高目标
- [ ] 伤害正确应用到目标

### 6.2 性能验收
- [ ] 伤害计算 < 0.1ms
- [ ] 目标锁定 < 1ms

---

## 7. 开放问题

| 问题 | 说明 | 优先级 | 决策 |
|------|------|--------|------|
| Q-01 | 暴击特效如何通知客户端？ | 中 | 通过RPC回调 `ClientHitConfirmed` 附带 `bIsCritical` 标志，客户端播放暴击特效 |
| Q-02 | 目标死亡动画谁来触发？ | 低 | 服务端判断目标死亡后调用 `PlayDeathAnimation()`，客户端通过复制属性同步死亡状态 |

---

## 7.1 暴击特效通知机制

### 客户端暴击特效流程
```
1. 服务端 ServerRequestAttack()
2. 服务端计算暴击，设置 bIsCritical = true
3. 服务端 ApplyDamageToTarget()
4. 服务端调用 ClientHitConfirmed(Handle, Damage, bIsCritical)
5. 客户端接收后:
   - 如果 bIsCritical: 播放暴击特效 + 伤害数字特效
   - 否则: 播放普通命中特效
```

### RPC回调扩展
```cpp
// 在 IDBARpcClient 中新增方法
UFUNCTION(Client, Reliable)
void ClientHitConfirmedWithCritical(
    FGameplayAbilitySpecHandle AbilityHandle,
    float Damage,
    bool bIsCritical,
    FVector HitLocation);
```

## 7.2 目标死亡触发机制

### 死亡判断
- 服务端 `CurrentHealth <= 0` 时判定死亡
- 服务端设置死亡状态属性
- 属性复制到客户端触发死亡动画

### 死亡流程
```
1. 服务端 ApplyDamageToTarget()
2. 目标ASC.SetCurrentHealth(0)
3. 服务端检测到死亡:
   - 调用 Target->PlayDeathAnimation()
   - 调用 Target->TransitionTo(EDeathState::Dead)
4. 客户端接收属性复制
5. 客户端播放死亡动画
```

### 死亡状态枚举
```cpp
UENUM(BlueprintType)
enum class EDADeathState : uint8
{
    Alive    UMETA(DisplayName = "Alive"),
    Dying    UMETA(DisplayName = "Dying"),
    Dead     UMETA(DisplayName = "Dead"),
    Respawning UMETA(DisplayName = "Respawning")
};
```

---

## 8. 参考文档

- `DBADamageCalculator.h/cpp` - 伤害计算器
- `DBABattleAttributeSet.h/cpp` - 战斗属性
- `IDBARpcServer.h` - RPC服务端接口

---

*文档生成时间: 2026-05-05*