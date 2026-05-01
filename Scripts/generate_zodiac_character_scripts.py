#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
生成十二生肖角色模型、动画蓝图和技能VFX/SFX脚本

功能:
- 生成角色模型组件基类
- 生成动画蓝图支持类
- 生成技能VFX/SFX管理器
- 生成12生肖各自的角色配置

用法:
    python generate_zodiac_character_scripts.py
"""

import os
from pathlib import Path

# ============== 配置 ==============
ZODIAC_TYPES = [
    ("Rat", "夜隐灵鼠", "Fire"),
    ("Ox", "镇岳神牛", "Water"),
    ("Tiger", "裂风虎君", "Wood"),
    ("Rabbit", "月华灵兔", "Gold"),
    ("Dragon", "云巡龙君", "Fire"),
    ("Snake", "玄花灵蛇", "Water"),
    ("Horse", "踏风天驹", "Wood"),
    ("Goat", "灵泽仙羊", "Gold"),
    ("Monkey", "幻云灵猿", "Fire"),
    ("Rooster", "曜鸣神鸡", "Water"),
    ("Dog", "镇魄灵犬", "Wood"),
    ("Pig", "福岳灵猪", "Gold"),
]

SKILL_TYPES = [
    ("Passive", "被动"),
    ("Q", "技能Q"),
    ("W", "技能W"),
    ("E", "技能E"),
    ("R", "终极技能"),
]

ELEMENTS = [
    ("Fire", "火"),
    ("Water", "水"),
    ("Wood", "木"),
    ("Gold", "金"),
    ("Earth", "土"),
]

# 怪物类型
MONSTER_TYPES = [
    ("Imp", "小魔怪", "Fire"),
    ("Slime", "史莱姆", "Water"),
    ("Skeleton", "骷髅", "Earth"),
    ("Ghost", "幽灵", "Wood"),
    ("Golem", "石魔", "Earth"),
    ("Wisp", "光球", "Gold"),
]

# 守卫类型
GUARDIAN_TYPES = [
    ("Tower", "防御塔", "Stone"),
    ("Crystal", "水晶", "Light"),
    ("Statue", "石像", "Earth"),
]

# ============== 模板 ==============

# 怪物模型基类头文件
MONSTER_BASE_HEADER = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 怪物模型基类

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DBAMonsterBase.generated.h"

class UDBAZodiacVFXComponent;

/**
 * DBAMonsterBase
 * 怪物模型基类
 * 提供怪物公共功能
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAMonsterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ADBAMonsterBase();

protected:
	virtual void BeginPlay() override;

public:
	/** 获取VFX组件 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Monster")
	UDBAZodiacVFXComponent* GetVFXComponent() const { return VFXComponent; }

	/** 播放受击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Monster")
	void PlayHitVFX(AActor* Attacker);

	/** 播放死亡特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Monster")
	void PlayDeathVFX();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|Components")
	TObjectPtr<UDBAZodiacVFXComponent> VFXComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	FName MonsterType = FName(TEXT("None"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	float MaxHealth = 100.0f;
};"""

# 怪物模型基类cpp
MONSTER_BASE_CPP = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 怪物模型基类

#include "Character/Monster/DBAMonsterBase.h"
#include "DBA/VFX/Components/DBAZodiacVFXComponent.h"
#include "Components/CapsuleComponent.h"

ADBAMonsterBase::ADBAMonsterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	VFXComponent = CreateDefaultSubobject<UDBAZodiacVFXComponent>(TEXT("VFXComponent"));

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
}

void ADBAMonsterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ADBAMonsterBase::PlayHitVFX(AActor* Attacker)
{
	if (VFXComponent)
	{
		VFXComponent->PlayHitVFX(Attacker);
	}
}

void ADBAMonsterBase::PlayDeathVFX()
{
	if (VFXComponent)
	{
		VFXComponent->PlayDeathVFX();
	}
}"""

# 守卫模型基类头文件
GUARDIAN_BASE_HEADER = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 守卫模型基类

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DBAGuardianBase.generated.h"

class UDBAZodiacVFXComponent;

/**
 * DBAGuardianBase
 * 守卫模型基类
 * 提供守卫公共功能
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAGuardianBase : public AActor
{
	GENERATED_BODY()

public:
	ADBAGuardianBase();

protected:
	virtual void BeginPlay() override;

public:
	/** 获取VFX组件 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Guardian")
	UDBAZodiacVFXComponent* GetVFXComponent() const { return VFXComponent; }

	/** 播放攻击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Guardian")
	void PlayAttackVFX(AActor* Target);

	/** 播放受击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Guardian")
	void PlayHitVFX(AActor* Attacker);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|Components")
	TObjectPtr<UDBAZodiacVFXComponent> VFXComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	FName GuardianType = FName(TEXT("None"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	float MaxHealth = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	float AttackDamage = 25.0f;
};"""

# 守卫模型基类cpp
GUARDIAN_BASE_CPP = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 守卫模型基类

#include "Character/Guardian/DBAGuardianBase.h"
#include "DBA/VFX/Components/DBAZodiacVFXComponent.h"
#include "Components/StaticMeshComponent.h"

ADBAGuardianBase::ADBAGuardianBase()
{
	PrimaryActorTick.bCanEverTick = true;

	VFXComponent = CreateDefaultSubobject<UDBAZodiacVFXComponent>(TEXT("VFXComponent"));

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ADBAGuardianBase::BeginPlay()
{
	Super::BeginPlay();
}

void ADBAGuardianBase::PlayAttackVFX(AActor* Target)
{
	if (VFXComponent)
	{
		VFXComponent->PlayAttackVFX(Target);
	}
}

void ADBAGuardianBase::PlayHitVFX(AActor* Attacker)
{
	if (VFXComponent)
	{
		VFXComponent->PlayHitVFX(Attacker);
	}
}"""

# 单个怪物头文件模板
MONSTER_HEADER = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 怪物 - {monster_cn}

#pragma once

#include "CoreMinimal.h"
#include "Character/Monster/DBAMonsterBase.h"
#include "DBAMonster_{monster}.generated.h"

UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAMonster_{monster} : public ADBAMonsterBase
{{
	GENERATED_BODY()

public:
	ADBAMonster_{monster}();
}};"""

# 单个怪物cpp模板
MONSTER_CPP = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 怪物 - {monster_cn}

#include "Character/Monster/DBAMonster_{monster}.h"

ADBAMonster_{monster}::ADBAMonster_{monster}()
{{
	MonsterType = FName(TEXT("{monster}"));
}}"""

# 单个守卫头文件模板
GUARDIAN_HEADER = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 守卫 - {guardian_cn}

#pragma once

#include "CoreMinimal.h"
#include "Character/Guardian/DBAGuardianBase.h"
#include "DBAGuardian_{guardian}.generated.h"

UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAGuardian_{guardian} : public ADBAGuardianBase
{{
	GENERATED_BODY()

public:
	ADBAGuardian_{guardian}();
}};"""

# 单个守卫cpp模板
GUARDIAN_CPP = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 守卫 - {guardian_cn}

#include "Character/Guardian/DBAGuardian_{guardian}.h"

ADBAGuardian_{guardian}::ADBAGuardian_{guardian}()
{{
	GuardianType = FName(TEXT("{guardian}"));
}}"""

# 角色模型基类头文件
CHARACTER_BASE_HEADER = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色模型基类

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DBAZodiacCharacterBase.generated.h"

class UDBAZodiacVFXComponent;
class UDBAZodiacAnimInstance;
class UDBAAbilitySystemComponent;

/**
 * DBAZodiacCharacterBase
 * 生肖角色基类
 * 提供角色公共功能：VFX/SFX播放、动画控制、属性同步
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAZodiacCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ADBAZodiacCharacterBase();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
	// ==================== 组件获取 ====================

	/** 获取VFX组件 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	UDBAZodiacVFXComponent* GetVFXComponent() const { return VFXComponent; }

	/** 获取动画实例 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	UDBAZodiacAnimInstance* GetZodiacAnimInstance() const;

	/** 获取能力系统组件 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Character")
	UDBAAbilitySystemComponent* GetDBAAbilitySystemComponent() const;

public:
	// ==================== VFX/SFX 接口 ====================

	/** 播放攻击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX")
	void PlayAttackVFX(AActor* Target);

	/** 播放受击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX")
	void PlayHitVFX(AActor* Attacker);

	/** 播放技能特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX")
	void PlaySkillVFX(FName SkillId, AActor* Target);

	/** 播放攻击音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX")
	void PlayAttackSFX();

	/** 播放受击音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX")
	void PlayHitSFX();

	/** 播放技能音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX")
	void PlaySkillSFX(FName SkillId);

public:
	// ==================== 动画接口 ====================

	/** 播放攻击动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void PlayAttackAnimation();

	/** 播放受击动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void PlayHitAnimation();

	/** 播放死亡动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void PlayDeathAnimation();

	/** 设置移动速度 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void SetAnimMoveSpeed(float Speed);

protected:
	// ==================== 组件 ====================

	/** VFX/SFX管理组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|Components")
	TObjectPtr<UDBAZodiacVFXComponent> VFXComponent;

	/** 角色元素类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	FName ElementType = FName(TEXT("Fire"));

	/** 角色生肖类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	FName ZodiacType = FName(TEXT("None"));
};
"""

# 角色模型基类cpp文件
CHARACTER_BASE_CPP = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色模型基类

#include "Character/DBAZodiacCharacterBase.h"
#include "DBA/VFX/Components/DBAZodiacVFXComponent.h"
#include "DBA/GAS/DBAAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"

ADBAZodiacCharacterBase::ADBAZodiacCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建VFX组件
	VFXComponent = CreateDefaultSubobject<UDBAZodiacVFXComponent>(TEXT("VFXComponent"));

	// 配置碰撞
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
}

void ADBAZodiacCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// 初始化元素类型
	if (UClass* AnimClass = GetMesh()->GetAnimClass())
	{
		// 动画类已设置
	}
}

void ADBAZodiacCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UDBAZodiacAnimInstance* ADBAZodiacCharacterBase::GetZodiacAnimInstance() const
{
	if (USkeletalMeshComponent* Mesh = GetMesh())
	{
		return Cast<UDBAZodiacAnimInstance>(Mesh->GetAnimInstance());
	}
	return nullptr;
}

UDBAAbilitySystemComponent* ADBAZodiacCharacterBase::GetDBAAbilitySystemComponent() const
{
	return Cast<UDBAAbilitySystemComponent>(GetAbilitySystemComponent());
}

// ==================== VFX 实现 ====================

void ADBAZodiacCharacterBase::PlayAttackVFX(AActor* Target)
{
	if (VFXComponent)
	{
		VFXComponent->PlayAttackVFX(Target);
	}
}

void ADBAZodiacCharacterBase::PlayHitVFX(AActor* Attacker)
{
	if (VFXComponent)
	{
		VFXComponent->PlayHitVFX(Attacker);
	}
}

void ADBAZodiacCharacterBase::PlaySkillVFX(FName SkillId, AActor* Target)
{
	if (VFXComponent)
	{
		// 根据技能ID播放对应特效
		FString SkillStr = SkillId.ToString();
		if (SkillStr.Contains("Passive"))
		{
			// 被动技能特效
		}
		else if (SkillStr.Contains("Q"))
		{
			// Q技能特效
		}
		else if (SkillStr.Contains("W"))
		{
			// W技能特效
		}
		else if (SkillStr.Contains("E"))
		{
			// E技能特效
		}
		else if (SkillStr.Contains("R"))
		{
			// R技能特效
		}
	}
}

// ==================== SFX 实现 ====================

void ADBAZodiacCharacterBase::PlayAttackSFX()
{
	if (VFXComponent)
	{
		VFXComponent->PlayAttackSFX();
	}
}

void ADBAZodiacCharacterBase::PlayHitSFX()
{
	if (VFXComponent)
	{
		VFXComponent->PlayHitSFX();
	}
}

void ADBAZodiacCharacterBase::PlaySkillSFX(FName SkillId)
{
	if (VFXComponent)
	{
		VFXComponent->PlaySkillSFX(SkillId);
	}
}

// ==================== Animation 实现 ====================

void ADBAZodiacCharacterBase::PlayAttackAnimation()
{
	if (VFXComponent)
	{
		VFXComponent->PlayAttackAnimation();
	}
}

void ADBAZodiacCharacterBase::PlayHitAnimation()
{
	if (VFXComponent)
	{
		VFXComponent->PlayHitAnimation();
	}
}

void ADBAZodiacCharacterBase::PlayDeathAnimation()
{
	if (VFXComponent)
	{
		VFXComponent->PlayDeathAnimation();
	}
}

void ADBAZodiacCharacterBase::SetAnimMoveSpeed(float Speed)
{
	if (UDBAZodiacAnimInstance* AnimInst = GetZodiacAnimInstance())
	{
		AnimInst->SetMoveSpeed(Speed);
	}
}
"""

# 动画实例基类头文件
ANIM_INSTANCE_HEADER = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色动画实例基类

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DBAZodiacAnimInstance.generated.h"

/**
 * DBAZodiacAnimInstance
 * 生肖角色动画实例基类
 * 提供动画状态管理和公共接口
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAZodiacAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UDBAZodiacAnimInstance();

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

public:
	// ==================== 动画状态接口 ====================

	/** 设置移动速度 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void SetMoveSpeed(float Speed) { MoveSpeed = Speed; }

	/** 获取移动速度 */
	UFUNCTION(BlueprintPure, Category = "DBA|Animation")
	float GetMoveSpeed() const { return MoveSpeed; }

	/** 设置是否在移动 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void SetIsMoving(bool bInMoving) { bIsMoving = bInMoving; }

	/** 获取是否在移动 */
	UFUNCTION(BlueprintPure, Category = "DBA|Animation")
	bool IsMoving() const { return bIsMoving; }

	/** 设置是否攻击中 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void SetIsAttacking(bool bInAttacking) { bIsAttacking = bInAttacking; }

	/** 获取是否攻击中 */
	UFUNCTION(BlueprintPure, Category = "DBA|Animation")
	bool IsAttacking() const { return bIsAttacking; }

	/** 设置是否受击中 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void SetIsHit(bool bInHit) { bIsHit = bInHit; }

	/** 获取是否受击中 */
	UFUNCTION(BlueprintPure, Category = "DBA|Animation")
	bool IsHit() const { return bIsHit; }

	/** 设置是否死亡 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void SetIsDead(bool bInDead) { bIsDead = bInDead; }

	/** 获取是否死亡 */
	UFUNCTION(BlueprintPure, Category = "DBA|Animation")
	bool IsDead() const { return bIsDead; }

	/** 设置当前技能ID */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation")
	void SetCurrentSkillId(FName InSkillId) { CurrentSkillId = InSkillId; }

	/** 获取当前技能ID */
	UFUNCTION(BlueprintPure, Category = "DBA|Animation")
	FName GetCurrentSkillId() const { return CurrentSkillId; }

public:
	// ==================== 动画曲线/变量 ====================

	/** 移动速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Animation")
	float MoveSpeed = 0.0f;

	/** 是否在移动 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Animation")
	bool bIsMoving = false;

	/** 是否在攻击中 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Animation")
	bool bIsAttacking = false;

	/** 是否受击 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Animation")
	bool bIsHit = false;

	/** 是否死亡 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Animation")
	bool bIsDead = false;

	/** 当前技能ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Animation")
	FName CurrentSkillId = NAME_None;

	/** 角色朝向 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Animation")
	FRotator RotationRate = FRotator::ZeroRotator;

protected:
	/** 所属Pawn引用 */
	UPROPERTY()
	TObjectPtr<APawn> OwningPawn;
};
"""

# 动画实例基类cpp文件
ANIM_INSTANCE_CPP = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色动画实例基类

#include "Animation/DBAZodiacAnimInstance.h"
#include "GameFramework/Pawn.h"

UDBAZodiacAnimInstance::UDBAZodiacAnimInstance()
{
}

void UDBAZodiacAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningPawn = TryGetPawnOwner();
}

void UDBAZodiacAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (OwningPawn)
	{
		// 更新移动速度
		if (APlayerController* PC = OwningPawn->GetController<APlayerController>())
		{
			if (APlayerCameraManager* CamManager = PC->PlayerCameraManager)
			{
				// 可以在这里更新相机相关的动画变量
			}
		}
	}
}
"""

# 单个生肖角色头文件模板
ZODIAC_CHARACTER_HEADER = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - {zodiac_cn}

#pragma once

#include "CoreMinimal.h"
#include "Character/DBAZodiacCharacterBase.h"
#include "DBAZodiacCharacter_{zodiac}.generated.h"

/**
 * ADBAZodiacCharacter_{zodiac}
 * {zodiac_cn}角色类
 * 继承自生肖角色基类，配置{zodiac_cn}特有的资源和行为
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAZodiacCharacter_{zodiac} : public ADBAZodiacCharacterBase
{{
	GENERATED_BODY()

public:
	ADBAZodiacCharacter_{zodiac}();

protected:
	virtual void BeginPlay() override;

public:
	// ==================== 角色配置 ====================

	/** 获取角色名称 */
	UFUNCTION(BlueprintPure, Category = "DBA|Config")
	FText GetCharacterName() const {{ return FText::FromString(TEXT("{zodiac_cn}")); }}

	/** 获取角色元素 */
	UFUNCTION(BlueprintPure, Category = "DBA|Config")
	FName GetElement() const {{ return FName(TEXT("{element}")); }}

protected:
	// ==================== 资源路径 ====================

	/** 骨骼网格体路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString SkeletalMeshPath = TEXT("/Game/Models/Zodiac/{zodiac}/SK_{zodiac}_Mesh.SK_{zodiac}_Mesh");

	/** 动画蓝图路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString AnimBlueprintPath = TEXT("/Game/Animation/Zodiac/{zodiac}/ABP_{zodiac}.ABP_{zodiac}");

	/** 角色图标路径 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	FString IconPath = TEXT("/Game/UI/Icons/Zodiac/{zodiac}_Icon.{zodiac}_Icon");
}};
"""

# 单个生肖角色cpp文件模板
ZODIAC_CHARACTER_CPP = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - {zodiac_cn}

#include "Character/Zodiac/DBAZodiacCharacter_{zodiac}.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetRegistry.h"

ADBAZodiacCharacter_{zodiac}::ADBAZodiacCharacter_{zodiac}()
{{
	// 设置元素类型
	ElementType = FName(TEXT("{element}"));
	ZodiacType = FName(TEXT("{zodiac}"));

	// 加载骨骼网格体
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("{skeletal_mesh_path}"));
	if (MeshFinder.Succeeded())
	{{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}}

	// 设置动画蓝图
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("{anim_blueprint_path}"));
	if (AnimBPFinder.Succeeded())
	{{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
	}}

	// 配置角色描述
	PrimaryActorTick.bCanEverTick = true;
}}

void ADBAZodiacCharacter_{zodiac}::BeginPlay()
{
	Super::BeginPlay();

	// {zodiac_cn}角色特定初始化
}}
"""


def generate_zodiac_character(zodiac_id: str, zodiac_cn: str, element: str) -> dict:
    """生成单个生肖角色类"""
    skeletal_mesh_path = f"/Game/Models/Zodiac/{zodiac_id}/SK_{zodiac_id}_Mesh.SK_{zodiac_id}_Mesh"
    anim_blueprint_path = f"/Game/Animation/Zodiac/{zodiac_id}/ABP_{zodiac_id}.ABP_{zodiac_id}"

    header = ZODIAC_CHARACTER_HEADER.format(
        zodiac_cn=zodiac_cn,
        zodiac=zodiac_id,
        element=element,
        skeletal_mesh_path=skeletal_mesh_path,
        anim_blueprint_path=anim_blueprint_path,
    )

    # cpp 模板需要特殊处理 Super::BeginPlay() 中的 {{
    cpp_template = '''// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色 - {zodiac_cn}

#include "Character/Zodiac/DBAZodiacCharacter_{zodiac}.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetRegistry.h"

ADBAZodiacCharacter_{zodiac}::ADBAZodiacCharacter_{zodiac}()
{{
	// 设置元素类型
	ElementType = FName(TEXT("{element}"));
	ZodiacType = FName(TEXT("{zodiac}"));

	// 加载骨骼网格体
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("{skeletal_mesh_path}"));
	if (MeshFinder.Succeeded())
	{{
		GetMesh()->SetSkeletalMesh(MeshFinder.Object);
	}}

	// 设置动画蓝图
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPFinder(TEXT("{anim_blueprint_path}"));
	if (AnimBPFinder.Succeeded())
	{{
		GetMesh()->SetAnimClass(AnimBPFinder.Class);
	}}

	// 配置角色描述
	PrimaryActorTick.bCanEverTick = true;
}}

void ADBAZodiacCharacter_{zodiac}::BeginPlay()
{{
	Super::BeginPlay();

	// {zodiac_cn}角色特定初始化
}}
'''
    cpp = cpp_template.format(
        zodiac_cn=zodiac_cn,
        zodiac=zodiac_id,
        element=element,
        skeletal_mesh_path=skeletal_mesh_path,
        anim_blueprint_path=anim_blueprint_path,
    )
    return {"header": header, "cpp": cpp}


def main():
    project_root = Path(__file__).parent.parent / "Source" / "DivineBeastsArena"

    # 目录结构
    char_public_dir = project_root / "Public" / "Character" / "Zodiac"
    char_private_dir = project_root / "Private" / "Character" / "Zodiac"
    char_public_dir.mkdir(parents=True, exist_ok=True)
    char_private_dir.mkdir(parents=True, exist_ok=True)

    # 动画目录
    anim_public_dir = project_root / "Public" / "Animation"
    anim_private_dir = project_root / "Private" / "Animation"
    anim_public_dir.mkdir(parents=True, exist_ok=True)
    anim_private_dir.mkdir(parents=True, exist_ok=True)

    # 基类目录
    base_char_public_dir = project_root / "Public" / "Character"
    base_char_private_dir = project_root / "Private" / "Character"
    base_char_public_dir.mkdir(parents=True, exist_ok=True)
    base_char_private_dir.mkdir(parents=True, exist_ok=True)

    # 怪物目录
    monster_public_dir = project_root / "Public" / "Character" / "Monster"
    monster_private_dir = project_root / "Private" / "Character" / "Monster"
    monster_public_dir.mkdir(parents=True, exist_ok=True)
    monster_private_dir.mkdir(parents=True, exist_ok=True)

    # 守卫目录
    guardian_public_dir = project_root / "Public" / "Character" / "Guardian"
    guardian_private_dir = project_root / "Private" / "Character" / "Guardian"
    guardian_public_dir.mkdir(parents=True, exist_ok=True)
    guardian_private_dir.mkdir(parents=True, exist_ok=True)

    # 1. 生成角色基类
    base_header = CHARACTER_BASE_HEADER
    base_cpp = CHARACTER_BASE_CPP
    with open(base_char_public_dir / "DBAZodiacCharacterBase.h", "w", encoding="utf-8") as f:
        f.write(base_header)
    with open(base_char_private_dir / "DBAZodiacCharacterBase.cpp", "w", encoding="utf-8") as f:
        f.write(base_cpp)
    print("  生成: DBAZodiacCharacterBase.h/cpp")

    # 2. 生成动画实例基类
    anim_header = ANIM_INSTANCE_HEADER
    anim_cpp = ANIM_INSTANCE_CPP
    with open(anim_public_dir / "DBAZodiacAnimInstance.h", "w", encoding="utf-8") as f:
        f.write(anim_header)
    with open(anim_private_dir / "DBAZodiacAnimInstance.cpp", "w", encoding="utf-8") as f:
        f.write(anim_cpp)
    print("  生成: DBAZodiacAnimInstance.h/cpp")

    # 3. 生成怪物基类
    with open(monster_public_dir / "DBAMonsterBase.h", "w", encoding="utf-8") as f:
        f.write(MONSTER_BASE_HEADER)
    with open(monster_private_dir / "DBAMonsterBase.cpp", "w", encoding="utf-8") as f:
        f.write(MONSTER_BASE_CPP)
    print("  生成: DBAMonsterBase.h/cpp")

    # 4. 生成守卫基类
    with open(guardian_public_dir / "DBAGuardianBase.h", "w", encoding="utf-8") as f:
        f.write(GUARDIAN_BASE_HEADER)
    with open(guardian_private_dir / "DBAGuardianBase.cpp", "w", encoding="utf-8") as f:
        f.write(GUARDIAN_BASE_CPP)
    print("  生成: DBAGuardianBase.h/cpp")

    # 5. 生成12个生肖角色类
    zodiac_count = 0
    for zodiac_id, zodiac_cn, element in ZODIAC_TYPES:
        char_code = generate_zodiac_character(zodiac_id, zodiac_cn, element)
        header_path = char_public_dir / f"DBAZodiacCharacter_{zodiac_id}.h"
        cpp_path = char_private_dir / f"DBAZodiacCharacter_{zodiac_id}.cpp"
        with open(header_path, "w", encoding="utf-8") as f:
            f.write(char_code["header"])
        with open(cpp_path, "w", encoding="utf-8") as f:
            f.write(char_code["cpp"])
        zodiac_count += 1
        print(f"  生成: DBAZodiacCharacter_{zodiac_id}.h/cpp")

    # 6. 生成怪物类
    monster_count = 0
    for monster_id, monster_cn, element in MONSTER_TYPES:
        header = MONSTER_HEADER.format(monster=monster_id, monster_cn=monster_cn)
        cpp = MONSTER_CPP.format(monster=monster_id, monster_cn=monster_cn)
        with open(monster_public_dir / f"DBAMonster_{monster_id}.h", "w", encoding="utf-8") as f:
            f.write(header)
        with open(monster_private_dir / f"DBAMonster_{monster_id}.cpp", "w", encoding="utf-8") as f:
            f.write(cpp)
        monster_count += 1
        print(f"  生成: DBAMonster_{monster_id}.h/cpp")

    # 7. 生成守卫类
    guardian_count = 0
    for guardian_id, guardian_cn, element in GUARDIAN_TYPES:
        header = GUARDIAN_HEADER.format(guardian=guardian_id, guardian_cn=guardian_cn)
        cpp = GUARDIAN_CPP.format(guardian=guardian_id, guardian_cn=guardian_cn)
        with open(guardian_public_dir / f"DBAGuardian_{guardian_id}.h", "w", encoding="utf-8") as f:
            f.write(header)
        with open(guardian_private_dir / f"DBAGuardian_{guardian_id}.cpp", "w", encoding="utf-8") as f:
            f.write(cpp)
        guardian_count += 1
        print(f"  生成: DBAGuardian_{guardian_id}.h/cpp")

    print(f"\n完成! 共生成:")
    print(f"  - 1 个角色基类 (DBAZodiacCharacterBase)")
    print(f"  - 1 个动画实例基类 (DBAZodiacAnimInstance)")
    print(f"  - 1 个怪物基类 (DBAMonsterBase)")
    print(f"  - 1 个守卫基类 (DBAGuardianBase)")
    print(f"  - {zodiac_count} 个生肖角色类")
    print(f"  - {monster_count} 个怪物类")
    print(f"  - {guardian_count} 个守卫类")
    print(f"\n资源目录结构:")
    print(f"  Zodiac Models: /Game/Models/Zodiac/{{zodiac}}/")
    print(f"  Monster Models: /Game/Models/Monsters/{{monster}}/")
    print(f"  Guardian Models: /Game/Models/Guardians/{{guardian}}/")
    print(f"  Animation: /Game/Animation/Zodiac/{{zodiac}}/")
    print(f"  Icons: /Game/UI/Icons/Zodiac/{{zodiac}}/")
    print(f"  VFX: /Game/VFX/Zodiac/{{zodiac}}/")
    print(f"  SFX: /Game/Audio/SFX/Zodiac/{{zodiac}}/")


if __name__ == "__main__":
    main()