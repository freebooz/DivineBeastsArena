# 技能系统完善需求规格

> 项目: DivineBeastsArena 神兽竞技场
> 版本: 0.1.0
> 日期: 2026-05-05
> 状态: 待评审

---

## 1. 目标与范围

### 1.1 目标
完善技能系统的核心功能，包括冷却管理、伤害公式修正、技能取消和效果应用。

### 1.2 范围
**包含:**
- 冷却管理系统 (通过GameplayEffect实现)
- 伤害公式完善 (元素克制、连锁加成)
- 技能取消功能 (ServerCancelAbility)
- 施法范围检查

**不包含:**
- 具体技能配置数据 (DataTable)
- UI技能图标和状态显示
- 技能解锁和升级系统

---

## 2. 功能需求

### 2.1 冷却管理系统

#### 2.1.1 冷却结构体
```cpp
USTRUCT(BlueprintType)
struct FDBAAbilityCooldown
{
    GENERATED_BODY()

    UPROPERTY()
    FGameplayTag CooldownTag;

    UPROPERTY()
    float RemainingTime = 0.0f;

    UPROPERTY()
    float TotalDuration = 0.0f;
};
```

#### 2.1.2 冷却GameplayEffect
```cpp
UCLASS()
class UDBECooldownEffect : public UGameplayEffect
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
    float CooldownDuration = 1.0f;
};
```

#### 2.1.3 冷却应用接口
```cpp
UFUNCTION(BlueprintCallable, Category = "Ability|Cooldown")
void ApplyCooldown(float Duration);

UFUNCTION(BlueprintCallable, Category = "Ability|Cooldown")
float GetCooldownTimeRemaining() const;

UFUNCTION(BlueprintCallable, Category = "Ability|Cooldown")
bool IsOnCooldown() const;
```

### 2.2 伤害公式完善

#### 2.2.1 元素克制实现
```cpp
// 在 DBAMobaGameplayAbilityBase 中
UFUNCTION(BlueprintCallable, Category = "Ability|Damage")
float GetElementMultiplier(EDBAElement AttackElement, EDBAElement DefenseElement);

// 返回: 1.2 (克制) / 1.0 (无关系) / 0.8 (被克制)
```

#### 2.2.2 连锁加成实现
```cpp
UFUNCTION(BlueprintCallable, Category = "Ability|Damage")
float GetChainBonus(int32 ChainLevel);

// Chain 1-5: 1.2x
// Chain 6-9: 1.35x
// Chain 10 (Final): 0.2x MaxHP special
```

#### 2.2.3 完整伤害公式
```cpp
float FinalDamage = BaseDamage
    × ElementMultiplier (1.2/1.0/0.8)
    × ResonanceBonus (1.0~1.2)
    × ChainBonus (1.0/1.2/1.35/0.2MaxHP)
    × (1 - DefenseReduction)
    × (bIsCritical ? CriticalMultiplier : 1.0)
```

### 2.3 技能取消功能

#### 2.3.1 ServerCancelAbility 实现
```cpp
void ADBARpcHandler::ServerCancelAbility_Implementation(FGameplayAbilitySpecHandle Handle)
{
    if (ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner()))
    {
        if (UDBAAbilitySystemComponent* ASC = Character->GetDBAAbilitySystemComponent())
        {
            ASC->CancelAbilitySpec(Handle);
        }
    }
}
```

#### 2.3.2 取消触发条件
- 移动时取消引导类技能
- 受到硬控制时取消所有技能
- 使用新技能时取消当前技能

### 2.4 施法范围检查

#### 2.4.1 范围验证接口
```cpp
UFUNCTION(BlueprintCallable, Category = "Ability|Target")
bool IsInCastRange(AActor* Target, float CastRange) const;

UFUNCTION(BlueprintCallable, Category = "Ability|Target")
bool CanActivateAbilityWithTarget(AActor* Target) const;
```

#### 2.4.2 检查实现
```cpp
bool UDBAMobaGameplayAbilityBase::IsInCastRange(AActor* Target, float CastRange) const
{
    if (!Target) return false;
    float Distance = GetDistanceTo(Target);
    return Distance <= CastRange;
}
```

---

## 3. 非功能需求

### 3.1 性能需求
- 技能激活检测 < 0.5ms
- 冷却状态查询 < 0.1ms

### 3.2 网络同步
- 冷却状态通过GE复制
- 技能取消立即同步

---

## 4. 技术方案

### 4.1 目录结构

```
GameDBA/GAS/
├── Abilities/
│   └── (existing files)
├── Effects/
│   ├── DBAGE_Base.h/cpp
│   └── DBAGE_Cooldown.h/cpp      (新增)
└── Components/
    └── DBAAbilitySystemComponent.h/cpp (扩展)
```

### 4.2 关键类设计

#### 4.2.1 UDBECooldownEffect
```cpp
UCLASS()
class UDBECooldownEffect : public UGameplayEffect
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
    float CooldownDuration = 1.0f;

    // 持续设置为 Duration
    // 移除时自动清除冷却Tag
};
```

#### 4.2.2 DBAMobaGameplayAbilityBase 扩展
```cpp
public:
// 冷却管理
UFUNCTION(BlueprintCallable, Category = "Ability|Cooldown")
void ApplyCooldown(float Duration);

UFUNCTION(BlueprintCallable, Category = "Ability|Cooldown")
float GetCooldownTimeRemaining() const;

// 伤害计算
UFUNCTION(BlueprintCallable, Category = "Ability|Damage")
float GetElementMultiplier(EDBAElement AttackElement, EDBAElement DefenseElement);

UFUNCTION(BlueprintCallable, Category = "Ability|Damage")
float GetChainBonus(int32 ChainLevel);

// 目标检查
UFUNCTION(BlueprintCallable, Category = "Ability|Target")
bool IsInCastRange(AActor* Target, float CastRange) const;
```

---

## 5. 用户故事

| ID | 场景 | 预期行为 |
|----|------|----------|
| US-01 | 激活技能 | 消耗能量，应用冷却GE |
| US-02 | 技能冷却中 | IsOnCooldown() 返回true，无法激活 |
| US-03 | 取消技能 | ServerCancelAbility 取消当前技能 |
| US-04 | 超出范围 | 技能激活失败，提示范围不足 |

---

## 6. 验收标准

### 6.1 功能验收
- [ ] 技能激活时正确应用冷却
- [ ] 冷却期间技能无法激活
- [ ] ServerCancelAbility 可以取消技能
- [ ] 元素克制正确影响伤害

### 6.2 性能验收
- [ ] 冷却查询 < 0.1ms
- [ ] 技能激活检测 < 0.5ms

---

## 7. 参考文档

- `DBAMobaGameplayAbilityBase.h/cpp` - MOBA技能基类
- `DBAElementAbilityBase.h/cpp` - 元素技能
- `DBADamageCalculator.h/cpp` - 伤害计算器

---

*文档生成时间: 2026-05-05*