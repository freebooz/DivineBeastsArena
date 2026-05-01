#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
生成技能 VFX/SFX 挂载脚本

功能:
- 为 12 生肖 × 5 技能 = 60 个技能生成 VFX/SFX 挂载代码
- 技能包括: Passive, Q, W, E, R

用法:
    python generate_skill_vfx_sfx.py
"""

import os
from pathlib import Path

# ============== 配置 ==============
ZODIAC_TYPES = [
    ("Rat", "夜隐灵鼠"),
    ("Ox", "镇岳神牛"),
    ("Tiger", "裂风虎君"),
    ("Rabbit", "月华灵兔"),
    ("Dragon", "云巡龙君"),
    ("Snake", "玄花灵蛇"),
    ("Horse", "踏风天驹"),
    ("Goat", "灵泽仙羊"),
    ("Monkey", "幻云灵猿"),
    ("Rooster", "曜鸣神鸡"),
    ("Dog", "镇魄灵犬"),
    ("Pig", "福岳灵猪"),
]

SKILL_TYPES = [
    ("Passive", "被动"),
    ("Q", "技能Q"),
    ("W", "技能W"),
    ("E", "技能E"),
    ("R", "终极技能"),
]

# 技能特效类型
SKILL_VFX_TYPES = [
    ("Impact", "冲击特效"),      # 命中时
    ("Projectile", "弹道特效"),  # 飞行物
    ("AOE", "范围特效"),         # 范围效果
    ("Channel", "引导特效"),     # 持续施法
    ("Buff", "增益特效"),        # Buff应用
    ("Debuff", "减益特效"),      # Debuff应用
]

# ============== 模板 ==============

# 技能 VFX 挂载组件头文件模板
SKILL_VFX_COMPONENT_HEADER = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能VFX/SFX挂载组件 - {zodiac_cn}{skill_name}

#pragma once

#include "CoreMinimal.h"
#include "DBAZodiacSkillVFXComponent_{zodiac}_{skill}.generated.h"
#include "Components/ActorComponent.h"

class UParticleSystem;
class USoundCue;
class UAnimMontage;

/**
 * DBAZodiacSkillVFXComponent_{zodiac}_{skill}
 * 技能VFX/SFX挂载组件
 * 负责管理技能的视觉和音效资源
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "DBA Skill VFX {zodiac} {skill}"))
class DIVINEBEASTSARENA_API UDBAZodiacSkillVFXComponent_{zodiac}_{skill} : public UActorComponent
{{
	GENERATED_BODY()

public:
	UDBAZodiacSkillVFXComponent_{zodiac}_{skill}();

public:
	// ==================== 技能特效接口 ====================

	/** 播放施法特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|{zodiac_cn}{skill_name}")
	void PlayCastingVFX(AActor* Target);

	/** 播放命中特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|{zodiac_cn}{skill_name}")
	void PlayImpactVFX(AActor* HitTarget);

	/** 播放飞行特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|{zodiac_cn}{skill_name}")
	void PlayProjectileVFX(FVector Start, FVector End);

	/** 播放范围特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|{zodiac_cn}{skill_name}")
	void PlayAOEVFX(FVector Center, float Radius);

	/** 播放引导特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|{zodiac_cn}{skill_name}")
	void PlayChannelVFX();

	/** 停止引导特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|{zodiac_cn}{skill_name}")
	void StopChannelVFX();

	// ==================== 技能音效接口 ====================

	/** 播放施法音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillSFX|{zodiac_cn}{skill_name}")
	void PlayCastingSFX();

	/** 播放飞行音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillSFX|{zodiac_cn}{skill_name}")
	void PlayProjectileSFX();

	/** 播放命中音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillSFX|{zodiac_cn}{skill_name}")
	void PlayImpactSFX();

protected:
	// ==================== 特效资源 ====================

	/** 施法特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Skill")
	TSoftObjectPtr<UParticleSystem> CastingVFX;

	/** 命中特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Skill")
	TSoftObjectPtr<UParticleSystem> ImpactVFX;

	/** 飞行弹道特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Skill")
	TSoftObjectPtr<UParticleSystem> ProjectileVFX;

	/** 范围爆炸特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Skill")
	TSoftObjectPtr<UParticleSystem> AOEVFX;

	/** 引导持续特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Skill")
	TSoftObjectPtr<UParticleSystem> ChannelVFX;

	/** 增益特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Skill")
	TSoftObjectPtr<UParticleSystem> BuffVFX;

	/** 减益特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Skill")
	TSoftObjectPtr<UParticleSystem> DebuffVFX;

	// ==================== 音效资源 ====================

	/** 施法音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|Skill")
	TSoftObjectPtr<USoundCue> CastingSFX;

	/** 飞行弹道音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|Skill")
	TSoftObjectPtr<USoundCue> ProjectileSFX;

	/** 命中音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|Skill")
	TSoftObjectPtr<USoundCue> ImpactSFX;

	// ==================== 动画资源 ====================

	/** 施法动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Skill")
	TSoftObjectPtr<UAnimMontage> CastingMontage;

	/** 命中动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Skill")
	TSoftObjectPtr<UAnimMontage> ImpactMontage;

protected:
	/** 引导特效的粒子组件 */
	UPROPERTY()
	TObjectPtr<UParticleSystemComponent> ChannelVFXComponent;
}};
"""

# 技能 VFX/SFX 挂载组件 cpp 文件模板
SKILL_VFX_COMPONENT_CPP = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能VFX/SFX挂载组件 - {zodiac_cn}{skill_name}

#include "DBA/VFX/Components/Skill/DBAZodiacSkillVFXComponent_{zodiac}_{skill}.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UDBAZodiacSkillVFXComponent_{zodiac}_{skill}::UDBAZodiacSkillVFXComponent_{zodiac}_{skill}()
{{
	PrimaryComponentTick.bCanEverTick = true;
}}

void UDBAZodiacSkillVFXComponent_{zodiac}_{skill}::PlayCastingVFX(AActor* Target)
{{
	if (UParticleSystem* VFX = CastingVFX.LoadSynchronous())
	{{
		FVector Location = Target ? Target->GetActorLocation() : GetOwner()->GetActorLocation();
		FRotator Rotation = GetOwner()->GetActorRotation();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}}

	if (USoundCue* SFX = CastingSFX.LoadSynchronous())
	{{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}}

	// 播放施法动画
	if (UAnimMontage* Montage = CastingMontage.LoadSynchronous())
	{{
		if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
		{{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}}
	}}
}}

void UDBAZodiacSkillVFXComponent_{zodiac}_{skill}::PlayImpactVFX(AActor* HitTarget)
{{
	if (UParticleSystem* VFX = ImpactVFX.LoadSynchronous())
	{{
		FVector Location = HitTarget ? HitTarget->GetActorLocation() : GetOwner()->GetActorLocation();
		FRotator Rotation = FRotator::ZeroRotator;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}}

	if (USoundCue* SFX = ImpactSFX.LoadSynchronous())
	{{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}}

	// 播放命中动画
	if (UAnimMontage* Montage = ImpactMontage.LoadSynchronous())
	{{
		if (USkeletalMeshComponent* Mesh = HitTarget->FindComponentByClass<USkeletalMeshComponent>())
		{{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}}
	}}
}}

void UDBAZodiacSkillVFXComponent_{zodiac}_{skill}::PlayProjectileVFX(FVector Start, FVector End)
{{
	if (UParticleSystem* VFX = ProjectileVFX.LoadSynchronous())
	{{
		// 计算方向
		FVector Direction = (End - Start).GetSafeNormal();
		FRotator Rotation = Direction.Rotation();

		// 在起点生成飞行特效
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Start, Rotation, true);
	}}

	if (USoundCue* SFX = ProjectileSFX.LoadSynchronous())
	{{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, Start);
	}}
}}

void UDBAZodiacSkillVFXComponent_{zodiac}_{skill}::PlayAOEVFX(FVector Center, float Radius)
{{
	if (UParticleSystem* VFX = AOEVFX.LoadSynchronous())
	{{
		FRotator Rotation = FRotator::ZeroRotator;
		UParticleSystemComponent* PSystem = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Center, Rotation, true);
		if (PSystem)
		{{
			// 缩放特效范围
			PSystem->SetVectorParameter(FName(TEXT("Radius")), FVector(Radius));
		}}
	}}
}}

void UDBAZodiacSkillVFXComponent_{zodiac}_{skill}::PlayChannelVFX()
{{
	if (UParticleSystem* VFX = ChannelVFX.LoadSynchronous())
	{{
		FVector Location = GetOwner()->GetActorLocation();
		FRotator Rotation = GetOwner()->GetActorRotation();
		ChannelVFXComponent = UGameplayStatics::SpawnEmitterAttached(
			VFX, GetOwner()->GetRootComponent(), NAME_None, Location, Rotation, EAttachLocation::KeepRelativeOffset, true);
	}}

	if (USoundCue* SFX = CastingSFX.LoadSynchronous())
	{{
		UGameplayStatics::PlaySoundAttached(SFX, GetOwner()->GetRootComponent());
	}}
}}

void UDBAZodiacSkillVFXComponent_{zodiac}_{skill}::StopChannelVFX()
{{
	if (ChannelVFXComponent)
	{{
		ChannelVFXComponent->Deactivate();
		ChannelVFXComponent = nullptr;
	}}
}}

void UDBAZodiacSkillVFXComponent_{zodiac}_{skill}::PlayProjectileSFX()
{{
	if (USoundCue* SFX = ProjectileSFX.LoadSynchronous())
	{{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}}
}}

void UDBAZodiacSkillVFXComponent_{zodiac}_{skill}::PlayImpactSFX()
{{
	if (USoundCue* SFX = ImpactSFX.LoadSynchronous())
	{{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}}
}}
"""


def generate_skill_vfx_component(zodiac_id: str, zodiac_cn: str, skill_id: str, skill_name: str) -> dict:
    """生成单个技能的 VFX/SFX 组件"""
    header = SKILL_VFX_COMPONENT_HEADER.format(
        zodiac_cn=zodiac_cn,
        skill_name=skill_name,
        zodiac=zodiac_id,
        skill=skill_id,
    )
    cpp = SKILL_VFX_COMPONENT_CPP.format(
        zodiac_cn=zodiac_cn,
        skill_name=skill_name,
        zodiac=zodiac_id,
        skill=skill_id,
    )
    return {"header": header, "cpp": cpp}


def main():
    project_root = Path(__file__).parent.parent / "Source" / "DivineBeastsArena"

    # 技能 VFX 组件目录
    skill_vfx_public_dir = project_root / "Public" / "DBA" / "VFX" / "Components" / "Skill"
    skill_vfx_private_dir = project_root / "Private" / "DBA" / "VFX" / "Components" / "Skill"
    skill_vfx_public_dir.mkdir(parents=True, exist_ok=True)
    skill_vfx_private_dir.mkdir(parents=True, exist_ok=True)

    count = 0

    for zodiac_id, zodiac_cn in ZODIAC_TYPES:
        for skill_id, skill_name in SKILL_TYPES:
            # 生成技能 VFX 组件
            vfx_code = generate_skill_vfx_component(zodiac_id, zodiac_cn, skill_id, skill_name)
            vfx_header_path = skill_vfx_public_dir / f"DBAZodiacSkillVFXComponent_{zodiac_id}_{skill_id}.h"
            vfx_cpp_path = skill_vfx_private_dir / f"DBAZodiacSkillVFXComponent_{zodiac_id}_{skill_id}.cpp"
            with open(vfx_header_path, "w", encoding="utf-8") as f:
                f.write(vfx_code["header"])
            with open(vfx_cpp_path, "w", encoding="utf-8") as f:
                f.write(vfx_code["cpp"])
            count += 1

    print(f"\n完成! 共生成:")
    print(f"  - {count} 个技能 VFX/SFX 组件")
    print(f"\n资源目录结构:")
    print(f"  VFX: /Game/VFX/Skills/{{zodiac}}/{{skill}}/")
    print(f"  SFX: /Game/Audio/SFX/Skills/{{zodiac}}/{{skill}}/")
    print(f"  Animation: /Game/Animation/Skills/{{zodiac}}/{{skill}}/")


if __name__ == "__main__":
    main()