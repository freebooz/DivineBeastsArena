#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
生成生肖角色 VFX/SFX 资源挂载脚本

功能:
- 为 12 生肖角色生成独立的 VFX/SFX 挂载组件
- 生成动画相关接口
- 创建资源引用结构

用法:
    python generate_zodiac_vfx_sfx.py
"""

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

# ============== 模板 ==============

# VFX/SFX 挂载组件头文件模板
VFX_COMPONENT_HEADER = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// VFX/SFX 挂载组件 - {zodiac_cn}

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DBAZodiacVFXComponent_{zodiac}.generated.h"

class UParticleSystem;
class USoundCue;
class UAnimMontage;

/**
 * UDBAZodiacVFXComponent_{zodiac}
 * 生肖角色 VFX/SFX 挂载组件
 * 负责管理{zodiac_cn}的视觉和音效资源
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "DBA Zodiac VFX Component {zodiac}"))
class DIVINEBEASTSARENA_API UDBAZodiacVFXComponent_{zodiac} : public UActorComponent
{{
	GENERATED_BODY()

public:
	UDBAZodiacVFXComponent_{zodiac}();

public:
	// ==================== VFX 接口 ====================

	/** 播放攻击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX|{zodiac_cn}")
	void PlayAttackVFX(AActor* Target);

	/** 播放受击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX|{zodiac_cn}")
	void PlayHitVFX(AActor* Attacker);

	/** 播放移动特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX|{zodiac_cn}")
	void PlayMoveVFX(const FVector& Direction);

	/** 播放死亡特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX|{zodiac_cn}")
	void PlayDeathVFX();

	/** 播放重生特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX|{zodiac_cn}")
	void PlayRespawnVFX();

	// ==================== SFX 接口 ====================

	/** 播放攻击音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX|{zodiac_cn}")
	void PlayAttackSFX();

	/** 播放受击音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX|{zodiac_cn}")
	void PlayHitSFX();

	/** 播放移动音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX|{zodiac_cn}")
	void PlayMoveSFX();

	/** 播放死亡音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX|{zodiac_cn}")
	void PlayDeathSFX();

	/** 播放技能音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX|{zodiac_cn}")
	void PlaySkillSFX(FName SkillId);

	// ==================== 动画接口 ====================

	/** 播放攻击动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation|{zodiac_cn}")
	void PlayAttackAnimation();

	/** 播放受击动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation|{zodiac_cn}")
	void PlayHitAnimation();

	/** 播放移动动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation|{zodiac_cn}")
	void PlayMoveAnimation(float Speed);

	/** 播放死亡动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation|{zodiac_cn}")
	void PlayDeathAnimation();

	/** 播放技能动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation|{zodiac_cn}")
	void PlaySkillAnimation(FName SkillId);

	/** 获取动画蓝图 */
	UFUNCTION(BlueprintPure, Category = "DBA|Animation|{zodiac_cn}")
	UAnimBlueprint* GetAnimBlueprint() const;

protected:
	// ==================== VFX 资源 ====================

	/** 攻击特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|{zodiac_cn}")
	TSoftObjectPtr<UParticleSystem> AttackVFX;

	/** 受击特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|{zodiac_cn}")
	TSoftObjectPtr<UParticleSystem> HitVFX;

	/** 移动特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|{zodiac_cn}")
	TSoftObjectPtr<UParticleSystem> MoveVFX;

	/** 死亡特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|{zodiac_cn}")
	TSoftObjectPtr<UParticleSystem> DeathVFX;

	/** 重生特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|{zodiac_cn}")
	TSoftObjectPtr<UParticleSystem> RespawnVFX;

	// ==================== SFX 资源 ====================

	/** 攻击音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|{zodiac_cn}")
	TSoftObjectPtr<USoundCue> AttackSFX;

	/** 受击音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|{zodiac_cn}")
	TSoftObjectPtr<USoundCue> HitSFX;

	/** 移动音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|{zodiac_cn}")
	TSoftObjectPtr<USoundCue> MoveSFX;

	/** 死亡音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|{zodiac_cn}")
	TSoftObjectPtr<USoundCue> DeathSFX;

	// ==================== 动画资源 ====================

	/** 动画蓝图 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|{zodiac_cn}")
	TSoftClassPtr<UAnimInstance> AnimBlueprintClass;

	/** 攻击动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|{zodiac_cn}")
	TSoftObjectPtr<UAnimMontage> AttackMontage;

	/** 受击动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|{zodiac_cn}")
	TSoftObjectPtr<UAnimMontage> HitMontage;

	/** 死亡动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|{zodiac_cn}")
	TSoftObjectPtr<UAnimMontage> DeathMontage;

	/** 技能动画映射 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|{zodiac_cn}")
	TMap<FName, TSoftObjectPtr<UAnimMontage>> SkillMontages;

	/** 角色元素类型 (用于加载对应元素特效) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	FName ElementType = FName(TEXT("{element}"));
}};
"""

# VFX/SFX 挂载组件 cpp 文件模板
VFX_COMPONENT_CPP = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// VFX/SFX 挂载组件 - {zodiac_cn}

#include "DBA/VFX/Components/DBAZodiacVFXComponent_{zodiac}.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UDBAZodiacVFXComponent_{zodiac}::UDBAZodiacVFXComponent_{zodiac}()
{{
	PrimaryComponentTick.bCanEverTick = false;

	// 设置元素类型
	ElementType = FName(TEXT("{element}"));

	// 加载默认资源路径
	LoadDefaultAssets();
}}

void UDBAZodiacVFXComponent_{zodiac}::LoadDefaultAssets()
{{
	// VFX 资源路径 - 设计师需要在UE编辑器中配置实际资源
	// AttackVFX.LoadSynchronous();
	// HitVFX.LoadSynchronous();
	// MoveVFX.LoadSynchronous();
	// DeathVFX.LoadSynchronous();
	// RespawnVFX.LoadSynchronous();

	// SFX 资源路径
	// AttackSFX.LoadSynchronous();
	// HitSFX.LoadSynchronous();
	// MoveSFX.LoadSynchronous();
	// DeathSFX.LoadSynchronous();

	// 动画资源路径
	// AnimBlueprintClass.LoadSynchronous();
	// AttackMontage.LoadSynchronous();
	// HitMontage.LoadSynchronous();
	// DeathMontage.LoadSynchronous();
}}

// ==================== VFX 实现 ====================

void UDBAZodiacVFXComponent_{zodiac}::PlayAttackVFX(AActor* Target)
{{
	if (AttackVFX.IsValid())
	{{
		UParticleSystem* VFX = AttackVFX.LoadSynchronous();
		if (VFX)
		{{
			FVector Location = Target ? Target->GetActorLocation() : GetOwner()->GetActorLocation();
			FRotator Rotation = Target ? Target->GetActorRotation() : FRotator::ZeroRotator;
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
		}}
	}}
}}

void UDBAZodiacVFXComponent_{zodiac}::PlayHitVFX(AActor* Attacker)
{{
	if (HitVFX.IsValid())
	{{
		UParticleSystem* VFX = HitVFX.LoadSynchronous();
		if (VFX)
		{{
			FVector Location = GetOwner()->GetActorLocation();
			FRotator Rotation = Attacker ? (Location - Attacker->GetActorLocation()).Rotation() : FRotator::ZeroRotator;
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
		}}
	}}
}}

void UDBAZodiacVFXComponent_{zodiac}::PlayMoveVFX(const FVector& Direction)
{{
	if (MoveVFX.IsValid())
	{{
		UParticleSystem* VFX = MoveVFX.LoadSynchronous();
		if (VFX)
		{{
			FVector Location = GetOwner()->GetActorLocation();
			FRotator Rotation = Direction.Rotation();
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
		}}
	}}
}}

void UDBAZodiacVFXComponent_{zodiac}::PlayDeathVFX()
{{
	if (DeathVFX.IsValid())
	{{
		UParticleSystem* VFX = DeathVFX.LoadSynchronous();
		if (VFX)
		{{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, GetOwner()->GetActorLocation(), FRotator::ZeroRotator, true);
		}}
	}}
}}

void UDBAZodiacVFXComponent_{zodiac}::PlayRespawnVFX()
{{
	if (RespawnVFX.IsValid())
	{{
		UParticleSystem* VFX = RespawnVFX.LoadSynchronous();
		if (VFX)
		{{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, GetOwner()->GetActorLocation(), FRotator::ZeroRotator, true);
		}}
	}}
}}

// ==================== SFX 实现 ====================

void UDBAZodiacVFXComponent_{zodiac}::PlayAttackSFX()
{{
	if (AttackSFX.IsValid())
	{{
		USoundCue* SFX = AttackSFX.LoadSynchronous();
		if (SFX)
		{{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
		}}
	}}
}}

void UDBAZodiacVFXComponent_{zodiac}::PlayHitSFX()
{{
	if (HitSFX.IsValid())
	{{
		USoundCue* SFX = HitSFX.LoadSynchronous();
		if (SFX)
		{{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
		}}
	}}
}}

void UDBAZodiacVFXComponent_{zodiac}::PlayMoveSFX()
{{
	if (MoveSFX.IsValid())
	{{
		USoundCue* SFX = MoveSFX.LoadSynchronous();
		if (SFX)
		{{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
		}}
	}}
}}

void UDBAZodiacVFXComponent_{zodiac}::PlayDeathSFX()
{{
	if (DeathSFX.IsValid())
	{{
		USoundCue* SFX = DeathSFX.LoadSynchronous();
		if (SFX)
		{{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
		}}
	}}
}}

void UDBAZodiacVFXComponent_{zodiac}::PlaySkillSFX(FName SkillId)
{{
	// 根据技能ID播放对应音效 - 设计师需要在UE编辑器中配置资源
	// FString Key = SkillId.ToString();
	// if (Key.Contains("Passive")) {{ }}
	// else if (Key.Contains("Q")) {{ }}
	// else if (Key.Contains("W")) {{ }}
	// else if (Key.Contains("E")) {{ }}
	// else if (Key.Contains("R")) {{ }}
}}

// ==================== 动画实现 ====================

void UDBAZodiacVFXComponent_{zodiac}::PlayAttackAnimation()
{{
	if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{{
		if (UAnimMontage* Montage = AttackMontage.LoadSynchronous())
		{{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}}
	}}
}}

void UDBAZodiacVFXComponent_{zodiac}::PlayHitAnimation()
{{
	if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{{
		if (UAnimMontage* Montage = HitMontage.LoadSynchronous())
		{{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}}
	}}
}}

void UDBAZodiacVFXComponent_{zodiac}::PlayMoveAnimation(float Speed)
{{
	if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{{
		if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
		{{
			AnimInstance->SetFloatValue(FName(TEXT("Speed")), Speed);
		}}
	}}
}}

void UDBAZodiacVFXComponent_{zodiac}::PlayDeathAnimation()
{{
	if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{{
		if (UAnimMontage* Montage = DeathMontage.LoadSynchronous())
		{{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}}
	}}
}}

void UDBAZodiacVFXComponent_{zodiac}::PlaySkillAnimation(FName SkillId)
{{
	if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{{
		if (UAnimMontage** MontagePtr = SkillMontages.Find(SkillId))
		{{
			if (UAnimMontage* Montage = MontagePtr->LoadSynchronous())
			{{
				Mesh->GetAnimInstance()->Montage_Play(Montage);
			}}
		}}
	}}
}}

UAnimBlueprint* UDBAZodiacVFXComponent_{zodiac}::GetAnimBlueprint() const
{{
	if (UClass* BPClass = AnimBlueprintClass.LoadSynchronous())
	{{
		return Cast<UAnimBlueprint>(BPClass->GetDefaultObject());
	}}
	return nullptr;
}}
"""


def generate_vfx_component(zodiac_id: str, zodiac_cn: str, element: str) -> dict:
    """生成单个生肖的 VFX/SFX 组件"""
    header = VFX_COMPONENT_HEADER.format(
        zodiac_cn=zodiac_cn,
        zodiac=zodiac_id,
        element=element,
    )
    cpp = VFX_COMPONENT_CPP.format(
        zodiac_cn=zodiac_cn,
        zodiac=zodiac_id,
        element=element,
    )
    return {"header": header, "cpp": cpp}


def main():
    project_root = Path(__file__).parent.parent / "Source" / "DivineBeastsArena"

    # VFX 组件目录
    vfx_public_dir = project_root / "Public" / "DBA" / "VFX" / "Components"
    vfx_private_dir = project_root / "Private" / "DBA" / "VFX" / "Components"
    vfx_public_dir.mkdir(parents=True, exist_ok=True)
    vfx_private_dir.mkdir(parents=True, exist_ok=True)

    count = 0

    for zodiac_id, zodiac_cn, element in ZODIAC_TYPES:
        # 生成 VFX 组件
        vfx_code = generate_vfx_component(zodiac_id, zodiac_cn, element)
        vfx_header_path = vfx_public_dir / f"DBAZodiacVFXComponent_{zodiac_id}.h"
        vfx_cpp_path = vfx_private_dir / f"DBAZodiacVFXComponent_{zodiac_id}.cpp"
        with open(vfx_header_path, "w", encoding="utf-8") as f:
            f.write(vfx_code["header"])
        with open(vfx_cpp_path, "w", encoding="utf-8") as f:
            f.write(vfx_code["cpp"])
        count += 1
        print(f"  生成: DBAZodiacVFXComponent_{zodiac_id}.h/cpp")

    print(f"\n完成! 共生成:")
    print(f"  - {count} 个生肖 VFX/SFX 组件")
    print(f"\n资源目录结构:")
    print(f"  VFX: /Game/VFX/Zodiac/{zodiac_id}/")
    print(f"  SFX: /Game/Audio/SFX/Zodiac/{zodiac_id}/")
    print(f"  Animation: /Game/Animation/Zodiac/{zodiac_id}/")
    print(f"\n设计师需要在UE编辑器中配置实际资源路径!")


if __name__ == "__main__":
    main()