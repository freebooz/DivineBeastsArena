# 技能反馈系统需求规格

> 项目: DivineBeastsArena 神兽竞技场
> 版本: 0.1.0
> 日期: 2026-05-05
> 状态: 待评审

---

## 1. 目标与范围

### 1.1 目标
为技能系统添加完整的视觉/听觉反馈，提升打击感。

### 1.2 核心功能
- GameplayTag 驱动的技能反馈事件通道
- SkillEffectTable 数据表配置每个技能的特效和音效
- Niagara 粒子系统驱动的浮动伤害数字
- 屏幕震动和命中冲击反馈

### 1.3 范围
**包含:**
- 技能释放特效 (Niagara)
- 技能命中特效 (Niagara)
- 浮动伤害数字 (Niagara)
- 释放/命中音效
- 屏幕震动
- GameplayTag 事件分发系统

**不包含:**
- 技能编辑器 (后续独立迭代)
- 技能数据表配置 (美术/策划填充)

---

## 2. 功能需求

### 2.1 技能反馈事件通道

#### 2.1.1 GameplayTag 事件定义
```
Event.Combat.Damage        - 普通伤害
Event.Combat.Critical      - 暴击伤害
Event.Combat.Kill          - 击杀
Event.Combat.Element.Fire  - 火属性伤害
Event.Combat.Element.Ice   - 冰属性伤害
Event.Combat.Element.Lightning - 雷属性伤害
Event.Combat.Element.Earth - 土属性伤害
Event.Combat.Element.Water - 水属性伤害
Event.Ability.Ready       - 技能冷却完成
Event.Ability.Ultimate    - 终极技能就绪
```

#### 2.1.2 事件数据结构
```cpp
USTRUCT(BlueprintType)
struct FDBACombatEventData
{
    GENERATED_BODY()

    /** 事件标签 */
    UPROPERTY(BlueprintReadOnly)
    FGameplayTag EventTag;

    /** 触发者 */
    UPROPERTY(BlueprintReadOnly)
    TWeakObjectPtr<AActor> Instigator;

    /** 目标 */
    UPROPERTY(BlueprintReadOnly)
    TWeakObjectPtr<AActor> Target;

    /** 技能ID */
    UPROPERTY(BlueprintReadOnly)
    FName SkillID;

    /** 伤害值 */
    UPROPERTY(BlueprintReadOnly)
    float DamageValue;

    /** 是否暴击 */
    UPROPERTY(BlueprintReadOnly)
    bool bIsCritical;

    /** 元素类型 */
    UPROPERTY(BlueprintReadOnly)
    EDBAElement Element;
};
```

#### 2.1.3 事件分发接口
```cpp
UCLASS()
class DIVINEBEASTSARENA_API UDBACombatFeedbackSubsystem : public UGameInstanceSubsystem
{
public:
    /** 分发战斗反馈事件 */
    UFUNCTION(BlueprintCallable, Category = "DBA|Feedback")
    void DispatchCombatEvent(const FDBACombatEventData& EventData);

    /** 监听事件 */
    void ListenForEvent(FGameplayTag EventTag, FDBACombatEventDelegate Delegate);

    /** 停止监听 */
    void StopListeningForEvent(FGameplayTag EventTag, FDBACombatEventDelegate Delegate);
};
```

---

### 2.2 技能特效配置表

#### 2.2.1 数据表结构 (FDBASkillEffectRow)
| 列名 | 类型 | 说明 |
|------|------|------|
| SkillID | FName | 技能唯一标识 |
| SkillName | FText | 技能显示名称 |
| ReleaseEffect | TSubclassOf<UNiagaraSystem> | 释放特效 (可选) |
| HitEffect | TSubclassOf<UNiagaraSystem> | 命中特效 (可选) |
| CastSound | USoundBase* | 释放音效 (可选) |
| HitSound | USoundBase* | 命中音效 (可选) |
| ScreenShakeClass | TSubclassOf<UCameraShakeBase> | 屏幕震动 (可选) |
| ShakeScale | float | 震动强度系数 (默认1.0) |
| DamageNumberColor | FLinearColor | 伤害数字颜色 (普通) |
| CriticalColor | FLinearColor | 暴击数字颜色 |

#### 2.2.2 元素颜色配置
| 元素 | 颜色 | 说明 |
|------|------|------|
| Fire | #FF4500 (橙红色) | 火属性伤害 |
| Ice | #00BFFF (冰蓝色) | 冰属性伤害 |
| Lightning | #FFD700 (金黄色) | 雷属性伤害 |
| Earth | #8B4513 (棕色) | 土属性伤害 |
| Water | #1E90FF (蓝色) | 水属性伤害 |
| Wood | #32CD32 (绿色) | 木属性伤害 |

> **颜色获取方式**: 从 EDBAElement 枚举直接映射，不在数据表中存储

#### 2.2.2 数据表注册
```cpp
UCLASS()
class DIVINEBEASTSARENA_API UDBAEffectTableManager : public UGameInstanceSubsystem
{
public:
    /** 加载技能特效表 */
    void LoadSkillEffectTable(const TSoftObjectPtr<UDataTable>& TablePath);

    /** 查询技能特效 */
    FDBASkillEffectRow* GetSkillEffect(FName SkillID) const;

private:
    UPROPERTY()
    TObjectPtr<UDataTable> SkillEffectTable;
};
```

---

### 2.3 浮动伤害数字 (Niagara)

#### 2.3.1 伤害数字组件
```cpp
UCLASS()
class DIVINEBEASTSARENA_API UDBAFloatingDamageComponent : public USceneComponent
{
public:
    /** 生成伤害数字 */
    UFUNCTION(BlueprintCallable, Category = "DBA|Feedback|DamageNumber")
    void SpawnDamageNumber(float Damage, bool bIsCritical, EDBAElement Element, FVector ImpactPoint);

    /** 设置Niagara系统类 */
    UFUNCTION(BlueprintCallable, Category = "DBA|Feedback|DamageNumber")
    void SetDamageNumberSystem(TSubclassOf<UNiagaraSystem> InDamageNumberSystem);

protected:
    /** Niagara系统 - 伤害数字 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Feedback|DamageNumber")
    TSubclassOf<UNiagaraSystem> DamageNumberSystem;

    /** 伤害数字池大小 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Feedback|DamageNumber", meta = (UIMin = 8, UIMax = 64))
    int32 PoolSize = 16;
};
```

#### 2.3.2 伤害数字颜色规则
```
优先级: 暴击 > 元素 > 普通
1. 如果 bIsCritical = true → 使用 CriticalColor (红色系)
2. 否则如果 Element != None → 使用元素对应颜色
3. 否则 → 使用 DamageNumberColor (白色/灰色)
```

#### 2.3.3 伤害数字动效参数
```
位置: ImpactPoint (命中点)
初始偏移: Z + 50~100 (随机)
上浮速度: 150~200 单位/秒
水平飘移: X/Y ±30 (随机)
持续时间: 1.0~1.5 秒
渐隐: 最后 0.3 秒 alpha 1→0
```

---

### 2.4 技能反馈播放

#### 2.4.1 反馈播放接口
```cpp
UCLASS()
class DIVINEBEASTSARENA_API UDBAEffectPlayer : public USubsystem
{
public:
    /** 播放技能释放效果 */
    UFUNCTION(BlueprintCallable, Category = "DBA|Feedback")
    void PlayReleaseEffect(AActor* Caster, FName SkillID, FVector Location, FRotator Direction);

    /** 播放技能命中效果 */
    UFUNCTION(BlueprintCallable, Category = "DBA|Feedback")
    void PlayHitEffect(AActor* Target, FName SkillID, FVector ImpactPoint);

    /** 播放音效 */
    UFUNCTION(BlueprintCallable, Category = "DBA|Feedback")
    void PlaySound(AActor* Target, FName SkillID, bool bIsHit);

    /** 触发屏幕震动 */
    UFUNCTION(BlueprintCallable, Category = "DBA|Feedback")
    void TriggerScreenShake(AActor* Target, FName SkillID, float Scale = 1.0f);

    /** 生成伤害数字 */
    UFUNCTION(BlueprintCallable, Category = "DBA|Feedback")
    void SpawnDamageNumber(AActor* Target, float Damage, bool bIsCritical, EDBAElement Element, FVector ImpactPoint);
};
```

#### 2.4.2 播放规则
```
1. 释放效果: 在 Caster 位置播放，不绑定目标
2. 命中效果: 在 ImpactPoint 位置播放
3. 音效: 2D音效 → 命中音效在 Target 位置; 3D音效 → 距离衰减
4. 屏幕震动: 仅对 PlayerController 触发，APawn 持有者
5. 伤害数字: 在 Target 位置生成，跟随 Target 头顶
```

---

### 2.5 与现有技能系统集成

#### 2.5.1 DBAMobaGameplayAbilityBase 扩展
```cpp
// 在技能激活时
void UDBAMobaGameplayAbilityBase::OnAbilityActivated()
{
    Super::OnAbilityActivated();

    // 触发释放特效
    if (UDBAEffectPlayer::Get())
    {
        UDBAEffectPlayer::Get()->PlayReleaseEffect(
            GetAvatarActorFromActorInfo(),
            GetSkillID(),
            GetActorLocation(),
            GetActorRotation()
        );
    }
}

// 在造成伤害时
void UDBAMobaGameplayAbilityBase::OnDamageDealt(AActor* Target, float Damage, bool bIsCritical)
{
    FDBACombatEventData EventData;
    EventData.EventTag = bIsCritical ?
        FGameplayTag::RequestGameplayTag(FName("Event.Combat.Critical")) :
        FGameplayTag::RequestGameplayTag(FName("Event.Combat.Damage"));
    EventData.Instigator = GetAvatarActorFromActorInfo();
    EventData.Target = Target;
    EventData.SkillID = GetSkillID();
    EventData.DamageValue = Damage;
    EventData.bIsCritical = bIsCritical;

    UDBACombatFeedbackSubsystem::Get()->DispatchCombatEvent(EventData);
}
```

---

## 3. 非功能需求

### 3.1 性能需求
- 特效播放 < 2ms (不阻塞主线程)
- 伤害数字同时存在 < 32 个
- Niagara 系统对象池复用
- 音效支持距离衰减和优先级

### 3.2 可配置性
- 所有特效路径可从 DataTable 配置
- 颜色、震动强度等参数可配置
- 支持技能等级差异化配置 (预留列)

### 3.3 扩展性
- 易于添加新元素类型
- 易于添加新特效类型 (如击退、闪烁)
- 事件通道对其他系统开放

---

## 4. 目录结构

```
GameDBA/
├── Combat/
│   ├── Feedback/
│   │   ├── DBAFloatingDamageComponent.h/cpp     (新)
│   │   ├── DBAEffectPlayer.h/cpp                 (新)
│   │   └── DBACombatFeedbackSubsystem.h/cpp      (新)
│   └── DataTable/
│       └── DBASkillEffectTable.csv               (新，数据文件)
├── System/
│   └── DBAEffectTableManager.h/cpp                (新)
└── Ability/
    └── DBAMobaGameplayAbilityBase.h/cpp          (修改)
```

---

## 5. 用户故事

| ID | 场景 | 预期行为 |
|----|------|----------|
| SF-01 | 技能命中 | 显示浮动伤害数字，上浮+渐隐 |
| SF-02 | 暴击发生 | 伤害数字变大变红，触发屏幕震动 |
| SF-03 | 技能释放 | 播放Niagara特效和释放音效 |
| SF-04 | 元素伤害 | 根据元素显示对应颜色伤害数字 |
| SF-05 | 击杀目标 | 触发击杀音效和特效 |

---

## 6. 验收标准

### 6.1 功能验收
- [ ] 技能命中时显示浮动伤害数字
- [ ] 暴击时伤害数字变大变红
- [ ] 释放技能时播放Niagara特效
- [ ] 命中目标时播放命中音效
- [ ] 暴击时触发屏幕震动
- [ ] 不同元素伤害显示不同颜色
- [ ] 所有特效/音效路径可配置

### 6.2 性能验收
- [ ] 特效播放不阻塞主线程
- [ ] 伤害数字对象正确复用

### 6.3 扩展性验收
- [ ] 可通过DataTable添加新技能特效
- [ ] GameplayTag事件可被其他系统监听

---

## 7. 依赖关系

```
DBASkillEffectTable (数据)
       ↓
DBAEffectTableManager (加载)
       ↓
DBAEffectPlayer (查询播放)
       ↓
DBAMobaGameplayAbilityBase (触发)
       ↓
DBAFloatingDamageComponent (显示数字)
       ↓
DBACombatFeedbackSubsystem (事件分发)
```

---

## 8. 参考文档

- `Docs/UISystemEnhancement_Requirements.md` - UI系统需求
- `Docs/NavigationSystem_Requirements.md` - 导航系统需求
- `Public/GameMoba/GAS/DBAMobaGameplayAbilityBase.h` - 技能基类

---

## 9. 数据表配置指南

### 9.1 SkillEffectTable 创建步骤

1. 在 UE 编辑器中打开项目
2. 在 `Content/DBA/Data/` 下创建新 DataTable
3. 选择 `FDBASkillEffectRow` 作为 Row Struct
4. 配置每个技能的特效和音效

### 9.2 现有 VFX 资源参考

**命中特效** (可复用):
```
Content/DBA/VFX/Common/Impact/NS_Impact_Magic_Burst.uasset
Content/DBA/VFX/Common/Impact/NS_Impact_Physical_Burst.uasset
Content/DBA/VFX/Common/Impact/NS_Impact_Critical_Hit.uasset
Content/DBA/VFX/Common/Impact/NS_Impact_Generic_Hit.uasset
```

**技能特效** (按英雄区分):
```
Content/DBA/VFX/Abilities/FireLion/NS_FireLion_Q_FlameClaw_Impact.uasset
Content/DBA/VFX/Abilities/FireLion/NS_FireLion_Q_FlameClaw_Slash.uasset
Content/DBA/VFX/Abilities/WaterDragon/NS_WaterDragon_Q_WaterBlast_Impact.uasset
Content/DBA/VFX/Abilities/EarthBear/NS_EarthBear_Q_GroundSlam_Impact.uasset
Content/DBA/VFX/Abilities/GoldPhoenix/NS_GoldPhoenix_Q_GoldFeather_Impact.uasset
Content/DBA/VFX/Abilities/WoodCrane/NS_WoodCrane_Q_HealingBurst_Impact.uasset
```

### 9.3 示例数据

| SkillID | SkillName | ReleaseEffect | HitEffect | CastSound | HitSound | CriticalColor |
|---------|-----------|---------------|-----------|-----------|---------|---------------|
| FireLion_Q | 烈焰利爪 | NS_FireLion_Q_FlameClaw_Slash | NS_FireLion_Q_FlameClaw_Impact | SFX_FireLion_Q_Cast | SFX_FireLion_Q_Impact | (1,0,0,1) |
| WaterDragon_Q | 水弹冲击 | NS_WaterDragon_Q_WaterBlast_Projectile | NS_WaterDragon_Q_WaterBlast_Impact | SFX_WaterDragon_Q_Cast | SFX_WaterDragon_Q_Impact | (1,0,0,1) |
| Critical_Hit | 暴击特效 | - | NS_Impact_Critical_Hit | - | SFX_Critical_Hit | (1,0,0,1) |

---

*文档生成时间: 2026-05-05*
