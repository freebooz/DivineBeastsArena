// Copyright Freebooz Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/Data/DBADataAssetBase.h"
#include "GameplayEffect.h"
#include "DBATypedDataAssets.generated.h"

/**
 * 五行阵营 DataAsset
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAFiveCampDataAsset : public UDBADataAssetBase
{
    GENERATED_BODY()

public:
    /** 阵营 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|FiveCamp")
    FName CampId;

    /** 阵营标签 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|FiveCamp")
    FGameplayTag CampTag;

    /** 关联元素标签 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|FiveCamp")
    FGameplayTag ElementTag;

    /** 守护神兽 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|FiveCamp")
    FText GuardianBeast;

    /** 战斗风格 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|FiveCamp")
    FText CombatStyle;

    /** 推荐英雄 ID 列表 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|FiveCamp")
    TArray<FName> RecommendedHeroIds;
};


/**
 * 元素 DataAsset
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAElementDataAsset : public UDBADataAssetBase
{
    GENERATED_BODY()

public:
    /** 元素 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Element")
    FName ElementId;

    /** 元素标签 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Element")
    FGameplayTag ElementTag;

    /** 克制元素 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Element")
    FName StrengthAgainst;

    /** 被克制元素 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Element")
    FName WeakAgainst;

    /** 核心机制描述 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Element")
    FText CoreMechanic;
};


/**
 * 生肖 DataAsset
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAZodiacDataAsset : public UDBADataAssetBase
{
    GENERATED_BODY()

public:
    /** 生肖 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac")
    FName ZodiacId;

    /** 默认关联元素 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac")
    FName DefaultElementId;

    /** 被动加成描述 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac")
    FText PassiveBonus;

    /** 加成数值 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac")
    float BonusValue = 0.0f;
};


/**
 * 英雄角色定位枚举
 */
UENUM(BlueprintType)
enum class EDBAHeroRole : uint8
{
    Fighter     UMETA(DisplayName = "战士"),
    Tank        UMETA(DisplayName = "坦克"),
    Support     UMETA(DisplayName = "辅助"),
    Controller  UMETA(DisplayName = "控制"),
    Assassin    UMETA(DisplayName = "刺客"),
    Healer      UMETA(DisplayName = "治疗"),
    Summoner    UMETA(DisplayName = "召唤"),
    Marksman    UMETA(DisplayName = "射手")
};

/**
 * 英雄基础属性结构体
 */
USTRUCT(BlueprintType)
struct FDBAHeroBaseStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Stats")
    float MaxHP = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Stats")
    float MaxMana = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Stats")
    float Attack = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Stats")
    float AbilityPower = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Stats")
    float Armor = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Stats")
    float MagicResist = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Stats")
    float MoveSpeed = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hero|Stats")
    float AttackRange = 150.0f;
};

/**
 * 英雄 DataAsset
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAHeroDataAsset : public UDBADataAssetBase
{
    GENERATED_BODY()

public:
    /** 英雄 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Hero")
    FName HeroId;

    /** 所属阵营 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Hero")
    FName CampId;

    /** 所属元素 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Hero")
    FName ElementId;

    /** 英雄定位 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Hero")
    EDBAHeroRole Role = EDBAHeroRole::Fighter;

    /** 难度等级 (1-5) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Hero")
    int32 Difficulty = 1;

    /** 主属性 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Hero")
    FName PrimaryAttribute;

    /** 基础属性 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Hero")
    FDBAHeroBaseStats BaseStats;

    /** 英雄蓝图类 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Hero")
    TSoftClassPtr<UObject> HeroBlueprintClass;

    /** 被动技能组引用 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Hero|Ability")
    TSoftObjectPtr<UDataAsset> PassiveAbilitySet;

    /** Q 技能组引用 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Hero|Ability")
    TSoftObjectPtr<UDataAsset> QAbilitySet;

    /** W 技能组引用 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Hero|Ability")
    TSoftObjectPtr<UDataAsset> WAbilitySet;

    /** E 技能组引用 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Hero|Ability")
    TSoftObjectPtr<UDataAsset> EAbilitySet;

    /** R 技能组引用 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Hero|Ability")
    TSoftObjectPtr<UDataAsset> RAbilitySet;
};


/**
 * 技能槽位枚举
 */
UENUM(BlueprintType)
enum class EDBAAbilitySlot : uint8
{
    Passive   UMETA(DisplayName = "被动"),
    Q         UMETA(DisplayName = "Q技能"),
    W         UMETA(DisplayName = "W技能"),
    E         UMETA(DisplayName = "E技能"),
    R         UMETA(DisplayName = "R技能"),
    D         UMETA(DisplayName = "D技能"),
    F         UMETA(DisplayName = "F技能")
};

/**
 * 技能类型枚举
 */
UENUM(BlueprintType)
enum class EDBAAbilityType : uint8
{
    Passive   UMETA(DisplayName = "被动"),
    Active    UMETA(DisplayName = "主动"),
    Mobility  UMETA(DisplayName = "位移"),
    Ultimate  UMETA(DisplayName = "终极技能"),
    Summon    UMETA(DisplayName = "召唤"),
    Heal      UMETA(DisplayName = "治疗"),
    Control   UMETA(DisplayName = "控制")
};

/**
 * 技能组配置 DataAsset（槽位配置版本）
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAAbilitySlotConfigAsset : public UDBADataAssetBase
{
    GENERATED_BODY()

public:
    /** 技能组 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet")
    FName AbilitySetId;

    /** 关联英雄 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet")
    FName HeroId;

    /** 技能槽位 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet")
    EDBAAbilitySlot Slot = EDBAAbilitySlot::Q;

    /** 技能类型 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet")
    EDBAAbilityType AbilityType = EDBAAbilityType::Active;

    /** 输入标签 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet")
    FGameplayTag InputTag;

    /** 元素标签 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet")
    FGameplayTag ElementTag;

    /** 技能标签 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet")
    FGameplayTag AbilityTag;

    /** 冷却标签 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet")
    FGameplayTag CooldownTag;

    /** 技能等级 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet")
    int32 AbilityLevel = 1;

    /** 冷却时间 (秒) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet")
    float Cooldown = 0.0f;

    /** 能量消耗 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet")
    float ManaCost = 0.0f;

    /** 技能类 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet")
    TSoftClassPtr<UGameplayAbility> AbilityClass;

    /** 启动效果列表 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet")
    TArray<TSoftClassPtr<UGameplayEffect>> StartupEffects;

    // VFX 资源路径
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet|VFX")
    FSoftObjectPath CastVFXPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet|VFX")
    FSoftObjectPath ImpactVFXPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet|VFX")
    FSoftObjectPath LoopVFXPath;

    // SFX 资源路径
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet|SFX")
    FSoftObjectPath CastSFXPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet|SFX")
    FSoftObjectPath ImpactSFXPath;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|AbilitySet|SFX")
    FSoftObjectPath LoopSFXPath;
};


/**
 * 对象池类型枚举（槽位版本）
 */
UENUM(BlueprintType)
enum class EDBATypedObjectPoolType : uint8
{
    Projectile     UMETA(DisplayName = "弹道"),
    Effect         UMETA(DisplayName = "特效"),
    UI             UMETA(DisplayName = "UI"),
    Audio          UMETA(DisplayName = "音频"),
    GameplayActor  UMETA(DisplayName = "游戏对象")
};

/**
 * 对象池配置 DataAsset
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAObjectPoolConfigDataAsset : public UDBADataAssetBase
{
    GENERATED_BODY()

public:
    /** 对象池 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|ObjectPool")
    FName PoolId;

    /** 对象池类型 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|ObjectPool")
    EDBATypedObjectPoolType PoolType = EDBATypedObjectPoolType::Projectile;

    /** 默认大小 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|ObjectPool")
    int32 DefaultSize = 32;

    /** 最大大小 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|ObjectPool")
    int32 MaxSize = 128;

    /** 是否可扩展 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|ObjectPool")
    bool bExpandable = true;

    /** 是否自动归还 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|ObjectPool")
    bool bAutoReturn = true;

    /** 最大生命周期 (秒) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|ObjectPool")
    float MaxLifetime = 5.0f;

    /** 对象类 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|ObjectPool")
    TSoftClassPtr<UObject> PooledClass;
};


/**
 * 固定技能组 DataAsset（槽位版本）
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBATypedFixedSkillGroupAsset : public UDBADataAssetBase
{
    GENERATED_BODY()

public:
    /** 技能组 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|SkillGroup")
    FName GroupId;

    /** 模式 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|SkillGroup")
    FName ModeId;

    /** 槽位索引 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|SkillGroup")
    int32 SlotIndex = 0;

    /** 推荐角色 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|SkillGroup")
    FName RecommendedRole;

    /** 被动技能 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|SkillGroup")
    FName PassiveSkillId;

    /** Q 技能 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|SkillGroup")
    FName QSkillId;

    /** W 技能 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|SkillGroup")
    FName WSkillId;

    /** E 技能 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|SkillGroup")
    FName ESkillId;

    /** R 技能 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|SkillGroup")
    FName RSkillId;

    /** 召唤师技能 1 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|SkillGroup")
    FName BattleSpell1;

    /** 召唤师技能 2 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|SkillGroup")
    FName BattleSpell2;
};