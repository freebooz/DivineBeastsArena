# DivineBeastsArena 战斗系统文档

> 详细说明战斗系统核心机制：技能系统、元素克制、连锁系统、共鸣系统、终极能量。

---

## 1. 系统概览

### 1.1 战斗系统架构图

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                              战斗系统                                            │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                  │
│  ┌─────────────────────────────────────────────────────────────────────────┐    │
│  │                          技能系统                                          │    │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐   │    │
│  │  │ BasicAttack │ │  Skill01  │ │  Skill02  │ │  Skill03  │ │  Skill04  │   │    │
│  │  │  (普攻)   │ │ (元素技能) │ │ (元素技能) │ │ (元素技能) │ │ (元素技能) │   │    │
│  │  └──────────┘ └──────────┘ └──────────┘ └──────────┘ └──────────┘   │    │
│  │                                                                              │    │
│  │  ┌─────────────────────────────────────────────────────────────────┐  │    │
│  │  │                    ZodiacUltimate (生肖大招)                      │  │    │
│  │  │                    消耗 100 UltimateEnergy                       │  │    │
│  │  └─────────────────────────────────────────────────────────────────┘  │    │
│  │                                                                              │    │
│  │  ┌─────────────────────────────────────────────────────────────────┐  │    │
│  │  │                    Resonance (共鸣被动)                          │  │    │
│  │  │                    根据同元素数量激活                            │  │    │
│  │  └─────────────────────────────────────────────────────────────────┘  │    │
│  └─────────────────────────────────────────────────────────────────────────┘    │
│                                                                                  │
│  ┌────────────────┐  ┌────────────────┐  ┌────────────────┐                    │
│  │   元素克制     │  │   连锁系统      │  │  共鸣系统      │                    │
│  │  ElementCounter │  │    Chain       │  │  Resonance     │                    │
│  │  五行相生相克   │  │  Combo 伤害加成 │  │  属性加成      │                    │
│  └────────────────┘  └────────────────┘  └────────────────┘                    │
│                                                                                  │
│  ┌─────────────────────────────────────────────────────────────────────────┐    │
│  │                          终极能量系统                                     │    │
│  │                    UltimateEnergy (0~100)                               │    │
│  │  技能命中 +3  |  击杀英雄 +20  |  助攻 +10  |  被动 +1/s                │    │
│  └─────────────────────────────────────────────────────────────────────────┘    │
│                                                                                  │
└─────────────────────────────────────────────────────────────────────────────────┘
```

---

## 2. 技能系统

### 2.1 技能分类

| 技能类型 | 输入绑定 | 能量消耗 | 特殊机制 |
|----------|----------|----------|----------|
| BasicAttack | 普攻键 | 无 | 自动触发 |
| Skill01~04 | Q/W/E/R | CurrentEnergy | 元素克制 |
| ZodiacUltimate | G | 100 UltimateEnergy | 连锁加成 |
| Resonance | 无 | 无 | 被动生效 |

### 2.2 技能激活流程

```
玩家按下技能键
    ↓
输入路由层 (DBAInputRouterComponent)
    ↓
GAS 输入绑定 → TryActivateAbilityByInputTag
    ↓
CanActivateAbility
├── 冷却检查 (CooldownTag)
├── 阻止 Tag 检查 (BlockTags)
├── 能量检查 (CurrentEnergy / UltimateEnergy)
└── 目标检查 (IsValidTarget)
    ↓
CommitAbilityCost
├── 能量消耗 GE 应用 (Skill01~04)
└── UltimateEnergy 消耗 (ZodiacUltimate)
    ↓
ActivateAbility
├── 客户端: OnClientLocalFeedback (本地表现)
└── 服务端: OnServerActivate (技能逻辑)
    ↓
EndAbility
```

---

## 3. 元素克制系统

### 3.1 五行相克关系

```
       金 ──────→ 木
       ↑              │
       │              ↓
       │          土 ──────→ 水
       │          ↑              │
       │          │              ↓
       └─ 火 ←────┘          火
                   ↑              │
                   └──────────────┘
```

### 3.2 克制关系定义

| 攻击方元素 | 克制 | 被克制 |
|-----------|------|--------|
| 金 (Metal) | 木 (Wood) | 火 (Fire) |
| 木 (Wood) | 土 (Earth) | 金 (Metal) |
| 水 (Water) | 火 (Fire) | 土 (Earth) |
| 火 (Fire) | 金 (Metal) | 水 (Water) |
| 土 (Earth) | 水 (Water) | 木 (Wood) |

### 3.3 伤害倍率

| 克制类型 | 普通技能 | 生肖大招 |
|----------|---------|----------|
| 克制 | 1.25x | 1.35x |
| 被克制 | 0.80x | 0.65x |
| 无关系 | 1.00x | 1.00x |

### 3.4 克制计算代码

```cpp
namespace DBAElementCounter
{
    // 获取元素克制的目标元素
    inline EDBAElementType GetCounteredElement(EDBAElementType Element)
    {
        switch (Element)
        {
        case EDBAElementType::Metal:  return EDBAElementType::Wood;   // 金克木
        case EDBAElementType::Wood:   return EDBAElementType::Earth;  // 木克土
        case EDBAElementType::Water:  return EDBAElementType::Fire;   // 水克火
        case EDBAElementType::Fire:  return EDBAElementType::Metal;  // 火克金
        case EDBAElementType::Earth:  return EDBAElementType::Water;  // 土克水
        default:                     return EDBAElementType::None;
        }
    }

    // 获取克制倍率
    inline float GetCounterMultiplier(EDBAElementCounterResult CounterResult, bool bIsUltimate)
    {
        if (CounterResult == EDBAElementCounterResult::Counter)
            return bIsUltimate ? 1.35f : 1.25f;
        else if (CounterResult == EDBAElementCounterResult::Countered)
            return bIsUltimate ? 0.65f : 0.80f;
        return 1.00f;
    }
}
```

---

## 4. 连锁系统 (Chain System)

### 4.1 连锁等级

| 连锁等级 | 命中门槛 | 伤害加成 | 超时时间 |
|----------|----------|----------|----------|
| Lv.0 | 0 | 1.00x | - |
| Lv.1~5 | 1~5 连续命中 | +2% 每级 | 6 秒 |
| Lv.6 | 6 连 | 1.10x | 6 秒 |
| Lv.7~9 | 7~9 连 | +2% 每级 | 6 秒 |
| Lv.10 | 10 连 (连锁终结) | 1.35x | 触发终结 |

### 4.2 连锁终结 (Chain Finisher)

当 ChainLevel 达到 10 且下一次有效命中时触发连锁终结：

```
连锁终结伤害计算：
FinalDamage = BaseDamage × ChainFinisherDamageBonus + TargetMaxHealth × 20%
```

### 4.3 连锁加成常量

```cpp
namespace DBAConstants
{
    constexpr int32 MaxChainLevel = 10;              // 最大连锁等级
    constexpr float ChainTier1DamageBonus = 1.20f;   // 6连伤害加成
    constexpr float ChainTier2DamageBonus = 1.35f;   // 10连(终结)伤害加成
    constexpr float ChainFinisher_HealthPercentDamage = 0.20f;  // 终结已损生命 20%
    constexpr float ChainTimeout = 6.0f;              // 连锁超时 6 秒
}
```

### 4.4 ChainLevel 管理

```cpp
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

bool UDBAAbilitySystemComponent::ShouldTriggerChainFinisher() const
{
    return ChainLevel >= 10;
}
```

---

## 5. 共鸣系统 (Resonance System)

### 5.1 共鸣等级

共鸣等级由同元素技能数量决定：

| 共鸣等级 | 同元素技能数 | 控制时间加成 | 护盾加成 |
|----------|-------------|-------------|----------|
| Lv.0 | < 2 | - | - |
| Lv.1 | 2 | +0.25 秒 | +5% |
| Lv.2 | 3 | +0.50 秒 | +10% |
| Lv.3 | 4 | +0.75 秒 | +15% |
| Lv.4 | 5+ | +1.00 秒 | +20% |

### 5.2 共鸣计算常量

```cpp
namespace DBAConstants
{
    // 等级门槛
    constexpr int32 ResonanceLevel1_SkillCount = 2;
    constexpr int32 ResonanceLevel2_SkillCount = 3;
    constexpr int32 ResonanceLevel3_SkillCount = 4;
    constexpr int32 ResonanceLevel4_SkillCount = 5;

    // 控制时间加成
    constexpr float ResonanceLevel1_CCDuration = 0.25f;
    constexpr float ResonanceLevel2_CCDuration = 0.50f;
    constexpr float ResonanceLevel3_CCDuration = 0.75f;
    constexpr float ResonanceLevel4_CCDuration = 1.00f;

    // 护盾加成
    constexpr float ResonanceLevel1_ShieldBonus = 0.05f;
    constexpr float ResonanceLevel2_ShieldBonus = 0.10f;
    constexpr float ResonanceLevel3_ShieldBonus = 0.15f;
    constexpr float ResonanceLevel4_ShieldBonus = 0.20f;
}
```

### 5.3 共鸣效果应用

```cpp
void UDBAResonanceAbilityBase::ApplyResonanceEffect(int32 CurrentResonanceLevel)
{
    if (CurrentResonanceLevel <= 0) return;

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    // 创建共鸣效果 GE 实例
    UDBEResonanceBuffEffect* ResonanceGE = NewObject<UDBEResonanceBuffEffect>();
    ResonanceGE->ConfigureForResonanceLevel(CurrentResonanceLevel);

    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
        ResonanceGE->GetClass(), 1, EffectContext);

    ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
```

---

## 6. 终极能量系统 (Ultimate Energy)

### 6.1 能量获取

| 来源 | 能量值 | 说明 |
|------|--------|------|
| 技能命中 | +3 | 每次有效命中 |
| 击杀英雄 | +20 | 击杀敌人英雄 |
| 助攻 | +10 | 参与击杀 |
| 被动回复 | +1/秒 | 持续回复 |

### 6.2 能量消耗

| 技能类型 | 消耗 | 说明 |
|----------|------|------|
| BasicAttack | 0 | 无消耗 |
| Skill01~04 | CurrentEnergy | 技能配置值 |
| ZodiacUltimate | 100 | 满能量释放 |
| Resonance | 0 | 被动不消耗 |

### 6.3 能量管理常量

```cpp
namespace DBAConstants
{
    constexpr float MaxUltimateEnergy = 100.0f;    // 最大终极能量
    constexpr float UltimateEnergy_SkillHit = 3.0f;      // 技能命中
    constexpr float UltimateEnergy_HeroKill = 20.0f;      // 击杀英雄
    constexpr float UltimateEnergy_Assist = 10.0f;       // 助攻
    constexpr float UltimateEnergy_PassiveRegen = 1.0f;   // 被动回复/秒
}
```

### 6.4 UltimateEnergy 管理

```cpp
// 增加能量 (服务端权威)
void UDBAAbilitySystemComponent::AddUltimateEnergy(float Amount)
{
    if (GetOwnerRole() != ROLE_Authority) return;
    UltimateEnergy = FMath::Clamp(UltimateEnergy + Amount, 0.0f, MaxUltimateEnergy);
}

// 消耗能量 (服务端权威)
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

// 被动回复 (计时器驱动)
void UDBAAbilitySystemComponent::PassiveRegenUltimateEnergy()
{
    AddUltimateEnergy(1.0f);
}
```

---

## 7. 伤害计算

### 7.1 最终伤害公式

```
最终伤害 = 基础伤害 × 元素克制倍率 × 连锁加成 × (1 - 防御减免)
```

### 7.2 防御减免公式

```
防御减免 = Defense / (Defense + 100)
实际减免 = 基础伤害 × (1 - 防御减免)
```

### 7.3 伤害计算代码

```cpp
float UDBABattleAttributeSet::CalculatePhysicalDamageReduction() const
{
    float DefenseValue = GetDefense();
    return DefenseValue / (DefenseValue + 100.0f);
}

float UDBAMobaGameplayAbilityBase::CalculateDamageWithDefense(
    float BaseDamage, float TargetDefense) const
{
    float DamageReduction = TargetDefense / (TargetDefense + 100.0f);
    float FinalDamage = BaseDamage * (1.0f - DamageReduction);
    return FMath::Max(FinalDamage, 0.0f);
}

float UDBAMobaGameplayAbilityBase::ApplyCriticalHit(
    float Damage, float CriticalRate, float CriticalMultiplier) const
{
    float CritChance = FMath::Clamp(CriticalRate, 0.0f, 1.0f);
    if (FMath::FRand() < CritChance)
    {
        UE_LOG(LogDBACombat, Log, TEXT("暴击！原始伤害: %.1f, 倍率: %.2f"),
            Damage, CriticalMultiplier);
        return Damage * CriticalMultiplier;
    }
    return Damage;
}
```

### 7.4 完整伤害应用

```cpp
void UDBAMobaGameplayAbilityBase::ApplyDamageToTarget(
    AActor* TargetActor, float BaseDamage, const FGameplayTagContainer& DamageTags)
{
    if (!TargetActor) return;

    // 获取目标 ASC 和防御力
    UAbilitySystemComponent* TargetASC =
        UAbilitySystemGlobals::Get().GetAbilitySystemComponentFromActor(TargetActor);

    float TargetDefense = 0.0f;
    if (UDBAMobaAbilitySystemComponentBase* TargetDBASC =
        Cast<UDBAMobaAbilitySystemComponentBase>(TargetASC))
    {
        if (const UDBABattleAttributeSet* TargetAttrSet =
            TargetDBASC->GetSet<UDBABattleAttributeSet>())
        {
            TargetDefense = TargetAttrSet->GetDefense();
        }
    }

    // 计算最终伤害（防御减免）
    float FinalDamage = CalculateDamageWithDefense(BaseDamage, TargetDefense);

    // 创建 GE Context
    FGameplayEffectContextHandle EffectContext =
        MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
    EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());
    EffectContext.AddActors(TArray<TWeakObjectPtr<AActor>>({ TargetActor }));

    // 触发伤害 Cue
    FGameplayCueParameters CueParams;
    CueParams.SourceObject = GetAvatarActorFromActorInfo();
    CueParams.Instigator = GetAvatarActorFromActorInfo();
    CueParams.EffectContext = EffectContext;
    ExecuteGameplayCueOnTarget(FGameplayTag::RequestGameplayTag(TEXT("Cue.Damage")), CueParams);
}
```

---

## 8. 属性系统

### 8.1 战斗属性

| 属性 | 默认值 | 说明 |
|------|--------|------|
| MaxHealth | 1000 | 最大生命值 |
| CurrentHealth | 1000 | 当前生命值 |
| MaxEnergy | 100 | 最大能量值 |
| CurrentEnergy | 100 | 当前能量值 |
| EnergyRegen | 5.0 | 能量回复/秒 |
| AttackPower | 100 | 攻击力 |
| Defense | 50 | 防御力 |
| MoveSpeed | 600 | 移动速度 |
| CriticalRate | 0.1 | 暴击率 (0~1) |
| CriticalMultiplier | 2.0 | 暴击倍率 |
| MaxShield | 0 | 最大护盾值 |
| CurrentShield | 0 | 当前护盾值 |

### 8.2 属性钳制

```cpp
void UDBABattleAttributeSet::PreAttributeChange(
    const FGameplayAttribute& Attribute, float& NewValue)
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

---

## 9. 对局配置

### 9.1 技能系统配置

```cpp
namespace DBAConstants
{
    constexpr int32 CoreCombatInputCount = 6;   // BasicAttack + Skill01~04 + Ultimate
    constexpr int32 ActiveSkillCount = 4;      // Skill01~04
    constexpr int32 MaxSkillLevel = 5;        // 最大技能等级
    constexpr int32 MaxHeroLevel = 18;        // 最大英雄等级
}
```

### 9.2 重生时间计算

```
重生时间 = RespawnTimeBase + (英雄等级 - 1) × RespawnTimePerLevel
最大值 = 60 秒

示例：英雄 10 级重生时间 = 5 + (10-1) × 2 = 23 秒
```

---

## 10. 战斗流程总结

### 10.1 技能命中完整流程

```
1. 玩家按下技能键
2. 输入路由层发送 RPC
3. 服务端校验技能激活条件
   ├── 冷却检查
   ├── 能量检查
   └── 目标有效性检查
4. 服务端执行技能效果
   ├── 元素克制倍率计算
   ├── 连锁等级加成
   ├── 防御减免计算
   └── 伤害应用
5. 服务端广播结果
   ├── 伤害复制
   ├── GameplayCue 触发
   └── UI 更新
6. 客户端接收复制
   ├── 生命值更新
   ├── 技能冷却更新
   └── 视觉表现播放
```

---

## 11. 常量速查表

| 分类 | 常量名 | 值 | 说明 |
|------|--------|-----|------|
| 元素克制 | ElementCounter_Normal | 1.25 | 普通技能克制倍率 |
| 元素克制 | ElementCounter_Ultimate | 1.35 | 大招克制倍率 |
| 元素克制 | ElementCountered_Normal | 0.80 | 普通技能被克制倍率 |
| 元素克制 | ElementCountered_Ultimate | 0.65 | 大招被克制倍率 |
| 连锁 | MaxChainLevel | 10 | 最大连锁等级 |
| 连锁 | ChainTier1DamageBonus | 1.20 | 6连伤害加成 |
| 连锁 | ChainTier2DamageBonus | 1.35 | 10连(终结)加成 |
| 连锁 | ChainTimeout | 6.0s | 连锁超时时间 |
| 共鸣 | ResonanceLevel1_ShieldBonus | 5% | 1级护盾加成 |
| 共鸣 | ResonanceLevel4_ShieldBonus | 20% | 4级护盾加成 |
| 终极能量 | MaxUltimateEnergy | 100 | 最大终极能量 |
| 终极能量 | UltimateEnergy_SkillHit | 3 | 技能命中获得 |
| 终极能量 | UltimateEnergy_HeroKill | 20 | 击杀英雄获得 |
| 终极能量 | UltimateEnergy_PassiveRegen | 1/s | 被动回复 |

---

*文档版本: 1.0*
*生成时间: 2026-04-27*
*引擎版本: UE 5.7.1*
