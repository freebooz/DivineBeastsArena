# DivineBeastsArena GAS 系统文档

> Gameplay Ability System 核心架构、Ability/AttributeSet/Effect 详细 API 参考。

---

## 1. 系统概览

### 1.1 GAS 架构图

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                              UAbilitySystemComponent                            │
│  ┌─────────────────────────────────────────────────────────────────────────┐ │
│  │                   UDBAMobaAbilitySystemComponentBase                      │ │
│  │  - Ability/Effect/AttributeSet 批量赋予/移除                            │ │
│  │  - 输入绑定到 Ability 机制                                                │ │
│  └─────────────────────────────────────────────────────────────────────────┘ │
│  ┌─────────────────────────────────────────────────────────────────────────┐ │
│  │                      UDBAAbilitySystemComponent                         │ │
│  │  - UltimateEnergy / ChainLevel / ResonanceLevel 管理                    │ │
│  │  - 技能激活合法性校验 (能量/冷却/目标)                                  │ │
│  │  - 队伍敌我关系验证                                                      │ │
│  └─────────────────────────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────────────────────┐
│                              UGameplayAbility                                │
│  ┌─────────────────────────────────────────────────────────────────────────┐ │
│  │                    UDBAMobaGameplayAbilityBase                           │ │
│  │  - 激活校验 / 冷却检测 / 伤害计算 / 暴击判定                             │ │
│  │  - 客户端预测回调 / 服务端裁定回调                                        │ │
│  │  - GameplayCue 桥接接口                                                  │ │
│  └─────────────────────────────────────────────────────────────────────────┘ │
│  ┌──────────────────────┐ ┌──────────────────────┐ ┌──────────────────────┐  │
│  │  UDBAElementAbilityBase  │ │  UDBAZodiacAbilityBase │ │ UDBAResonanceAbilityBase│  │
│  │  - 技能01~04           │ │  - Passive           │ │  - 共鸣被动           │  │
│  │  - 元素类型            │ │  - 所属生肖          │ │  - 共鸣等级加成        │  │
│  │  - EnergyCost          │ │                     │ │                     │  │
│  └──────────────────────┘ └──────────────────────┘ └──────────────────────┘  │
│                                          │                                    │
│                            ┌─────────────┴─────────────┐                     │
│                            │ UDBAZodiacUltimateAbilityBase │                     │
│                            │  - 生肖大招                 │                     │
│                            │  - UltimateEnergy = 100    │                     │
│                            └───────────────────────────┘                     │
└──────────────────────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────────────────────┐
│                                UAttributeSet                                 │
│  ┌─────────────────────────────────────────────────────────────────────────┐ │
│  │                       UDBABattleAttributeSet                              │ │
│  │  Health / Energy / Attack / Defense / MoveSpeed / Critical / Shield     │ │
│  └─────────────────────────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────────────────────────┘
```

### 1.2 继承层级

```cpp
UAbilitySystemComponent
└── UDBAMobaAbilitySystemComponentBase      // MOBA ASC 扩展 (批量赋予)
    └── UDBAAbilitySystemComponent          // 项目专用 ASC (Ultimate/Chain/Resonance)

UGameplayAbility
├── UDBAMobaGameplayAbilityBase             // MOBA Ability 基类
│   ├── UDBAElementAbilityBase             // 元素技能 (Skill01~04)
│   ├── UDBAZodiacAbilityBase             // 生肖技能基类
│   │   └── UDBAZodiacUltimateAbilityBase // 生肖大招
│   └── UDBAResonanceAbilityBase          // 共鸣被动

UAttributeSet
└── UDBABattleAttributeSet                 // 战斗属性

UGameplayEffect
└── UDBEEnergyCostEffect                 // 能量消耗 GE
└── UDBEResonanceBuffEffect              // 共鸣 Buff GE
```

---

## 2. 属性系统 (DBABattleAttributeSet)

### 2.1 属性列表

| 属性 | 类型 | 默认值 | 分类 |
|------|------|--------|------|
| `MaxHealth` | float | 1000 | Health |
| `CurrentHealth` | float | 1000 | Health |
| `MaxEnergy` | float | 100 | Energy |
| `CurrentEnergy` | float | 100 | Energy |
| `EnergyRegen` | float | 5.0 | Energy |
| `AttackPower` | float | 100 | Attack |
| `Defense` | float | 50 | Attack |
| `MoveSpeed` | float | 600 | Movement |
| `CriticalRate` | float | 0.1 | Critical (0~1) |
| `CriticalMultiplier` | float | 2.0 | Critical |
| `MaxShield` | float | 0 | Shield |
| `CurrentShield` | float | 0 | Shield |

### 2.2 属性宏

```cpp
// BATTLE_ATTRIBUTE_ACCESSORS 宏简化属性访问器定义
#define BATTLE_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

// 使用示例
UPROPERTY(BlueprintReadOnly, Category = "Battle|Health", ReplicatedUsing = OnRep_MaxHealth)
FGameplayAttributeData MaxHealth;
BATTLE_ATTRIBUTE_ACCESSORS(UDBABattleAttributeSet, MaxHealth)
```

### 2.3 复制配置

```cpp
void UDBABattleAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // COND_None: 无条件复制
    // REPNOTIFY_Always: 即使值相同也复制
    DOREPLIFETIME_CONDITION_NOTIFY(UDBABattleAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UDBABattleAttributeSet, CurrentHealth, COND_None, REPNOTIFY_Always);
    // ... 其他属性
}
```

### 2.4 数值钳制 (Clamping)

**PreAttributeChange**: 实时钳制
```cpp
void UDBABattleAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    if (Attribute == GetCurrentHealthAttribute())
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
    else if (Attribute == GetCurrentEnergyAttribute())
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxEnergy());
    else if (Attribute == GetCurrentShieldAttribute())
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxShield());
    else if (Attribute == GetCriticalRateAttribute())
        NewValue = FMath::Clamp(NewValue, 0.0f, 1.0f);
}
```

**PostGameplayEffectExecute**: GE 修改后再次钳制
```cpp
void UDBABattleAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    if (Data.EvaluatedData.Attribute == GetCurrentHealthAttribute())
        SetCurrentHealth(FMath::Clamp(GetCurrentHealth(), 0.0f, GetMaxHealth()));
    // ... 其他属性
}
```

### 2.5 战斗计算函数

**伤害减免公式**:
```cpp
float UDBABattleAttributeSet::CalculatePhysicalDamageReduction() const
{
    float DefenseValue = GetDefense();
    return DefenseValue / (DefenseValue + 100.0f);
}
```

**暴击判定**:
```cpp
bool UDBABattleAttributeSet::RollCriticalHit() const
{
    float RandomValue = FMath::FRand();
    return RandomValue < GetCriticalRate();
}
```

---

## 3. Ability 基类 (DBAMobaGameplayAbilityBase)

### 3.1 配置属性

```cpp
// 激活策略
EDBAMobaAbilityActivationPolicy ActivationPolicy;
enum { OnInputTriggered, OnSpawn, Passive };

// 网络配置
bool bServerAuthority = true;      // 是否在服务端执行
bool bClientPrediction = false;     // 是否支持客户端预测

// Tag 配置
FGameplayTag InputTag;               // 输入 Tag
FGameplayTag AbilityTag;            // Ability 标识 Tag
FGameplayTag CooldownTag;           // 冷却 Tag
FGameplayTagContainer BlockTags;    // 阻止激活的 Tag
```

### 3.2 激活策略

| 策略 | 说明 | 示例 |
|------|------|------|
| `OnInputTriggered` | 收到输入时激活 | Skill01~04、BasicAttack |
| `OnSpawn` | Avatar 生成时自动激活 | Passive 被动技能 |
| `Passive` | 始终激活 | Resonance 共鸣被动 |

### 3.3 核心接口

```cpp
// 激活校验 (含冷却检测、阻止 Tag 检测)
virtual bool CanActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayTagContainer* SourceTags,
    const FGameplayTagContainer* TargetTags,
    FGameplayTagContainer* OptionalRelevantTags) const override;

// 激活 Ability
virtual void ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData) override;

// 结束 Ability
virtual void EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled) override;
```

### 3.4 客户端/服务端回调

```cpp
// 客户端本地反馈 (预测) - 播放动画/音效/粒子
UFUNCTION(BlueprintNativeEvent)
void OnClientLocalFeedback();
virtual void OnClientLocalFeedback_Implementation();

// 服务端裁定完成回调 - 接收裁定结果
UFUNCTION(BlueprintNativeEvent)
void OnServerAuthorityConfirmed();
virtual void OnServerAuthorityConfirmed_Implementation();

// 服务端裁定失败回调 - 预测失败回滚
UFUNCTION(BlueprintNativeEvent)
void OnServerAuthorityRejected();
virtual void OnServerAuthorityRejected_Implementation();

// 服务端权威激活逻辑 - 子类重写
UFUNCTION(BlueprintNativeEvent)
void OnServerActivate();
virtual void OnServerActivate_Implementation();
```

### 3.5 战斗辅助函数

```cpp
// 计算防御减免后的伤害
UFUNCTION(BlueprintCallable, Category = "Ability|Combat")
float CalculateDamageWithDefense(float BaseDamage, float TargetDefense) const;

// 应用暴击判定
UFUNCTION(BlueprintCallable, Category = "Ability|Combat")
float ApplyCriticalHit(float Damage, float CriticalRate, float CriticalMultiplier) const;

// 对目标应用伤害 (服务端调用)
UFUNCTION(BlueprintCallable, Category = "Ability|Combat")
void ApplyDamageToTarget(AActor* TargetActor, float BaseDamage, const FGameplayTagContainer& DamageTags);

// 触发 GameplayCue
UFUNCTION(BlueprintCallable, Category = "Ability|Cue")
void TriggerGameplayCueOnTarget(FGameplayTag CueTag, AActor* TargetActor, float Magnitude = 0.0f);
```

### 3.6 能量消耗实现

```cpp
bool UDBAElementAbilityBase::CommitAbilityCost(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    OUT FGameplayTagContainer* OptionalRelevantTags)
{
    // 防御性检查
    ensure(ActorInfo != nullptr);
    ensure(ActorInfo->AbilitySystemComponent.IsValid());

    // 通过 GameplayEffect 应用能量消耗，确保预测和复制系统正常工作
    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
    EffectContext.AddSourceObject(ActorInfo->AvatarActor.Get());

    TSubclassOf<UGameplayEffect> EnergyCostEffectClass = UDBEEnergyCostEffect::StaticClass();
    FGameplayEffectSpec EffectSpec(EnergyCostEffectClass, EffectContext, EnergyCost);

    FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(EffectSpec);

    return ActiveHandle.IsValid();
}
```

---

## 4. 元素技能 (DBAElementAbilityBase)

### 4.1 头文件

```cpp
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBAElementAbilityBase : public UDBAMobaGameplayAbilityBase
{
    GENERATED_BODY()

public:
    UDBAElementAbilityBase();

    /** 获取技能所属的自然元素类型 */
    UFUNCTION(BlueprintCallable, Category = "DBA|Element")
    EDBAElement GetElementType() const { return ElementType; }

    /** 所属自然元素之力 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Element")
    EDBAElement ElementType = EDBAElement::None;

    /** 是否消耗 CurrentEnergy */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Element")
    float EnergyCost;

protected:
    virtual bool CanActivateAbility(...) const override;
    virtual bool CommitAbilityCost(...) override;
};
```

### 4.2 激活校验

```cpp
bool UDBAElementAbilityBase::CanActivateAbility(...) const
{
    if (!Super::CanActivateAbility(...)) return false;

    // 校验 CurrentEnergy 是否足够
    const UDBABattleAttributeSet* CombatAttrSet = ASC->GetSet<UDBABattleAttributeSet>();
    float CurrentEnergy = CombatAttrSet->GetCurrentEnergy();
    float MaxEnergy = CombatAttrSet->GetMaxEnergy();

    if (CurrentEnergy < EnergyCost)
    {
        UE_LOG(LogDBACombat, Warning, TEXT("能量不足：需要 %.1f，当前 %.1f / %.1f"),
            EnergyCost, CurrentEnergy, MaxEnergy);
        return false;
    }

    return true;
}
```

---

## 5. 生肖技能 (DBAZodiacAbilityBase / DBAZodiacUltimateAbilityBase)

### 5.1 生肖基类

```cpp
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBAZodiacAbilityBase : public UDBAMobaGameplayAbilityBase
{
    GENERATED_BODY()

public:
    UDBAZodiacAbilityBase();

    /** 所属生肖标识 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Zodiac")
    EDBAZodiacType ZodiacType;
};
```

### 5.2 大招基类

```cpp
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBAZodiacUltimateAbilityBase : public UDBAZodiacAbilityBase
{
    GENERATED_BODY()

public:
    UDBAZodiacUltimateAbilityBase();

protected:
    // 检查 UltimateEnergy 是否达到 100
    virtual bool CanActivateAbility(...) const override;

    // 消耗 100 点 UltimateEnergy
    virtual bool CommitAbilityCost(...) override;
};
```

---

## 6. 共鸣被动 (DBAResonanceAbilityBase)

### 6.1 共鸣效果

| 等级 | 同元素技能数 | 控制加成 | 护盾加成 |
|------|-------------|---------|----------|
| Lv.0 | < 2 | - | - |
| Lv.1 | 2 | +0.25s | +5% |
| Lv.2 | 3 | +0.50s | +10% |
| Lv.3 | 4 | +0.75s | +15% |
| Lv.4 | 5+ | +1.00s | +20% |

### 6.2 激活逻辑

```cpp
void UDBAResonanceAbilityBase::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    UDBAAbilitySystemComponent* ASC = Cast<UDBAAbilitySystemComponent>(
        ActorInfo->AbilitySystemComponent.Get());

    if (ASC && ActorInfo->IsNetAuthority())
    {
        // 根据 ResonanceLevel 应用共鸣效果
        ApplyResonanceEffect(ASC->GetResonanceLevel());
    }
}
```

### 6.3 共鸣效果应用

```cpp
void UDBAResonanceAbilityBase::ApplyResonanceEffect(int32 CurrentResonanceLevel)
{
    if (CurrentResonanceLevel <= 0) return;

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    // 创建共鸣效果 GE 实例
    UDBEResonanceBuffEffect* ResonanceGE = NewObject<UDBEResonanceBuffEffect>();
    ResonanceGE->ConfigureForResonanceLevel(CurrentResonanceLevel);

    // 应用到自身
    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
        ResonanceGE->GetClass(), 1, EffectContext);

    ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
```

---

## 7. AbilitySystemComponent (项目专用)

### 7.1 UltimateEnergy 管理

```cpp
// 增加 UltimateEnergy (服务端权威)
void UDBAAbilitySystemComponent::AddUltimateEnergy(float Amount)
{
    if (GetOwnerRole() != ROLE_Authority) return;
    UltimateEnergy = FMath::Clamp(UltimateEnergy + Amount, 0.0f, 100.0f);
}

// 消耗 UltimateEnergy (服务端权威)
bool UDBAAbilitySystemComponent::ConsumeUltimateEnergy(float Amount)
{
    if (GetOwnerRole() != ROLE_Authority) return false;
    if (UltimateEnergy >= Amount)
    {
        UltimateEnergy -= Amount;
        return true;
    }
    return false;
}
```

### 7.2 ChainLevel 管理

```cpp
// 增加 ChainLevel
void UDBAAbilitySystemComponent::AddChainLevel(int32 Amount)
{
    if (GetOwnerRole() != ROLE_Authority) return;

    ChainLevel = FMath::Clamp(ChainLevel + Amount, 0, 10);
    LastHitTime = GetWorld()->GetTimeSeconds();

    // 重置 6 秒归零计时器
    GetWorld()->GetTimerManager().SetTimer(
        ChainResetTimerHandle, this,
        &UDBAAbilitySystemComponent::CheckChainReset,
        6.0f, false);
}

// 6 秒未命中归零
void UDBAAbilitySystemComponent::CheckChainReset()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastHitTime >= 6.0f)
    {
        ResetChainLevel();
    }
}
```

### 7.3 ResonanceLevel 管理

```cpp
// 设置 ResonanceLevel (根据同元素技能数量)
void UDBAAbilitySystemComponent::SetResonanceLevel(int32 Level)
{
    if (GetOwnerRole() != ROLE_Authority) return;
    ResonanceLevel = FMath::Clamp(Level, 0, 4);
}

// 共鸣等级计算逻辑
// 在 GrantAbilitiesFromFixedSkillGroup 中统计同元素技能数量
SetResonanceLevel(SameElementCount >= 5 ? 4 :
                   SameElementCount >= 4 ? 3 :
                   SameElementCount >= 3 ? 2 :
                   SameElementCount >= 2 ? 1 : 0);
```

### 7.4 目标验证

```cpp
bool UDBAAbilitySystemComponent::IsValidTarget(AActor* Target, bool bRequireEnemy) const
{
    if (GetOwnerRole() != ROLE_Authority) return false;
    if (!Target || !IsValid(Target)) return false;

    // 检查目标是否已死亡
    if (UAbilitySystemComponent* TargetASC =
        UAbilitySystemGlobals::Get().GetAbilitySystemComponentFromActor(Target))
    {
        if (const UDBABattleAttributeSet* TargetAttrSet =
            TargetASC->GetSet<UDBABattleAttributeSet>())
        {
            if (TargetAttrSet->GetCurrentHealth() <= 0.0f) return false;
        }
    }

    // 队伍敌我关系检查
    if (bRequireEnemy && Target->Implements<UDBATeamAgentInterface>())
    {
        AActor* SourceActor = GetOwner();
        int32 SourceTeamId = -1, TargetTeamId = -1;

        if (SourceActor && SourceActor->Implements<UDBATeamAgentInterface>())
            SourceTeamId = IDBATeamAgentInterface::Execute_GetTeamId(SourceActor);

        TargetTeamId = IDBATeamAgentInterface::Execute_GetTeamId(Target);

        // 中立目标 (-1) 不能作为敌人
        if (TargetTeamId == -1) return false;

        // 不同团队 = 敌人
        return SourceTeamId != TargetTeamId;
    }

    return true;
}
```

---

## 8. GameplayEffect

### 8.1 能量消耗 GE (DBEEnergyCostEffect)

```cpp
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBEEnergyCostEffect : public UGameplayEffect
{
    GENERATED_BODY()

public:
    UDBEEnergyCostEffect();
};
```

**用途**:
- 用于技能消耗 CurrentEnergy
- 通过 GAS 预测和复制系统确保客户端/服务端同步

### 8.2 共鸣 Buff GE (DBEResonanceBuffEffect)

```cpp
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBEResonanceBuffEffect : public UGameplayEffect
{
    GENERATED_BODY()

public:
    /** 根据共鸣等级配置 GE */
    void ConfigureForResonanceLevel(int32 ResonanceLevel);
};
```

---

## 9. 技能激活流程

### 9.1 输入触发流程

```
玩家按下技能键
    ↓
EnhancedInputComponent 触发 BindAction
    ↓
UDBAInputRouterComponent::OnSkillXXTriggered
    ↓
GAS InputTag 绑定自动调用 TryActivateAbilityByInputTag
    ↓
UAbilitySystemComponent::TryActivateAbility
    ↓
CanActivateAbility (冷却/能量/阻止 Tag 检查)
    ↓
CommitAbilityCost (能量消耗 GE 应用)
    ↓
ActivateAbility
    ├── 客户端: OnClientLocalFeedback (本地表现)
    └── 服务端: OnServerActivate (技能逻辑)
    ↓
EndAbility
```

### 9.2 Ultimate 激活流程

```
UltimateEnergy 达到 100
    ↓
玩家按下 Ultimate 键
    ↓
UDBAZodiacUltimateAbilityBase::CanActivateAbility
    ↓
检查 UltimateEnergy >= 100
    ↓
CommitAbilityCost
    ↓
ConsumeUltimateEnergy(100)
    ↓
ActivateAbility
    ↓
技能特定逻辑 (伤害/控制/视觉表现)
```

---

## 10. 复制属性

### 10.1 UDBAAbilitySystemComponent 复制

```cpp
void UDBAAbilitySystemComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UDBAAbilitySystemComponent, UltimateEnergy);
    DOREPLIFETIME(UDBAAbilitySystemComponent, ChainLevel);
    DOREPLIFETIME(UDBAAbilitySystemComponent, ResonanceLevel);
}
```

### 10.2 复制条件说明

| 条件 | 说明 |
|------|------|
| `COND_None` | 无条件复制 |
| `COND_OwnerOnly` | 仅 Owner 复制 |
| `COND_SkipOwner` | Owner 跳过 |
| `REPNOTIFY_Always` | 即使值相同也复制 |

---

*文档版本: 1.0*
*生成时间: 2026-04-27*
*引擎版本: UE 5.7.1*
