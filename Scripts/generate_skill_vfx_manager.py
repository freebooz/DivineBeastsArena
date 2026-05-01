#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
生成技能VFX/SFX管理器脚本

功能:
- 管理所有技能的VFX/SFX资源加载和播放
- 提供技能特效的统一的接口
- 支持技能连锁和组合特效

用法:
    python generate_skill_vfx_manager.py
"""

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

# ============== 模板 ==============

# 技能VFX管理器头文件
SKILL_VFX_MANAGER_HEADER = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能VFX/SFX管理器

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DBA skillVFXManager.generated.h"

class UParticleSystem;
class USoundCue;
class UAnimMontage;

/**
 * DBA skillVFXManager
 * 技能VFX/SFX管理器
 * 统一管理所有技能的视觉和音效资源
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBA skillVFXManager : public UActorComponent
{{
	GENERATED_BODY()

public:
	UDBA skillVFXManager();

public:
	// ==================== 技能特效播放 ====================

	/** 播放技能特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX")
	void PlaySkillVFX(FName SkillId, AActor* Target, AActor* Owner);

	/** 停止技能特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX")
	void StopSkillVFX(FName SkillId);

	/** 播放技能音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillSFX")
	void PlaySkillSFX(FName SkillId, AActor* Owner);

	/** 停止技能音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillSFX")
	void StopSkillSFX(FName SkillId);

public:
	// ==================== 资源加载 ====================

	/** 预加载所有技能资源 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Resources")
	void PreloadAllSkillResources();

	/** 卸载所有技能资源 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Resources")
	void UnloadAllSkillResources();

	/** 加载指定生肖的技能资源 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Resources")
	void PreloadZodiacResources(FName Zodiac);

protected:
	// ==================== 资源路径映射 ====================

	/** 获取技能VFX路径 */
	UFUNCTION(BlueprintPure, Category = "DBA|Resources")
	FString GetSkillVFXPath(FName SkillId);

	/** 获取技能SFX路径 */
	UFUNCTION(BlueprintPure, Category = "DBA|Resources")
	FString GetSkillSFXPath(FName SkillId);

	/** 获取技能动画路径 */
	UFUNCTION(BlueprintPure, Category = "DBA|Resources")
	FString GetSkillAnimPath(FName SkillId);

protected:
	// ==================== 资源数据 ====================

	/** 技能施法特效映射 */
	UPROPERTY()
	TMap<FName, TSoftObjectPtr<UParticleSystem>> SkillCastingVFX;

	/** 技能命中特效映射 */
	UPROPERTY()
	TMap<FName, TSoftObjectPtr<UParticleSystem>> SkillImpactVFX;

	/** 技能弹道特效映射 */
	UPROPERTY()
	TMap<FName, TSoftObjectPtr<UParticleSystem>> SkillProjectileVFX;

	/** 技能施法音效映射 */
	UPROPERTY()
	TMap<FName, TSoftObjectPtr<USoundCue>> SkillCastingSFX;

	/** 技能命中音效映射 */
	UPROPERTY()
	TMap<FName, TSoftObjectPtr<USoundCue>> SkillImpactSFX;

	/** 技能动画映射 */
	UPROPERTY()
	TMap<FName, TSoftObjectPtr<UAnimMontage>> SkillMontages;
}};
"""

# 技能VFX管理器cpp文件
SKILL_VFX_MANAGER_CPP = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能VFX/SFX管理器

#include "DBA/VFX/DBA skillVFXManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UDBA skillVFXManager::UDBA skillVFXManager()
{{
	PrimaryComponentTick.bCanEverTick = false;
}}

FString UDBA skillVFXManager::GetSkillVFXPath(FName SkillId)
{{
	FString SkillStr = SkillId.ToString();
	FString Zodiac = TEXT("Unknown");
	FString SkillType = TEXT("Passive");

	// 解析技能ID
	if (SkillStr.Contains(TEXT("_")))
	{{
		TArray<FString> Parts;
		SkillStr.ParseIntoArray(Parts, TEXT("_"), true);
		if (Parts.Num() >= 2)
		{{
			Zodiac = Parts[0];
			SkillType = Parts[1];
		}}
	}}

	return FString::Printf(TEXT("/Game/VFX/Skills/%s/%s/P_%s_%s_Cast"), *Zodiac, *SkillType, *Zodiac, *SkillType);
}}

FString UDBA skillVFXManager::GetSkillSFXPath(FName SkillId)
{{
	FString SkillStr = SkillId.ToString();
	FString Zodiac = TEXT("Unknown");
	FString SkillType = TEXT("Passive");

	if (SkillStr.Contains(TEXT("_")))
	{{
		TArray<FString> Parts;
		SkillStr.ParseIntoArray(Parts, TEXT("_"), true);
		if (Parts.Num() >= 2)
		{{
			Zodiac = Parts[0];
			SkillType = Parts[1];
		}}
	}}

	return FString::Printf(TEXT("/Game/Audio/SFX/Skills/%s/%s/S_%s_%s_Cast"), *Zodiac, *SkillType, *Zodiac, *SkillType);
}}

FString UDBA skillVFXManager::GetSkillAnimPath(FName SkillId)
{{
	FString SkillStr = SkillId.ToString();
	FString Zodiac = TEXT("Unknown");
	FString SkillType = TEXT("Passive");

	if (SkillStr.Contains(TEXT("_")))
	{{
		TArray<FString> Parts;
		SkillStr.ParseIntoArray(Parts, TEXT("_"), true);
		if (Parts.Num() >= 2)
		{{
			Zodiac = Parts[0];
			SkillType = Parts[1];
		}}
	}}

	return FString::Printf(TEXT("/Game/Animation/Skills/%s/%s/AM_%s_%s_Cast"), *Zodiac, *SkillType, *Zodiac, *SkillType);
}}

void UDBA skillVFXManager::PlaySkillVFX(FName SkillId, AActor* Target, AActor* Owner)
{{
	// 获取VFX路径
	FString VFXPath = GetSkillVFXPath(SkillId);

	// 加载粒子特效
	UParticleSystem* VFX = LoadObject<UParticleSystem>(nullptr, *VFXPath);
	if (VFX && Owner)
	{{
		FVector Location = Target ? Target->GetActorLocation() : Owner->GetActorLocation();
		FRotator Rotation = Target ? Target->GetActorRotation() : Owner->GetActorRotation();
		UGameplayStatics::SpawnEmitterAtLocation(Owner, VFX, Location, Rotation, true);
	}}
}}

void UDBA skillVFXManager::StopSkillVFX(FName SkillId)
{{
	// 停止指定技能的特效
}}

void UDBA skillVFXManager::PlaySkillSFX(FName SkillId, AActor* Owner)
{{
	// 获取SFX路径
	FString SFXPath = GetSkillSFXPath(SkillId);

	// 加载音效
	USoundCue* SFX = LoadObject<USoundCue>(nullptr, *SFXPath);
	if (SFX && Owner)
	{{
		UGameplayStatics::PlaySoundAtLocation(Owner, SFX, Owner->GetActorLocation());
	}}
}}

void UDBA skillVFXManager::StopSkillSFX(FName SkillId)
{{
	// 停止指定技能的音效
}}

void UDBA skillVFXManager::PreloadAllSkillResources()
{{
	// 遍历所有生肖和技能类型，预加载资源
	for (const auto& Zodiac : ZodiacTypes)
	{{
		for (const auto& Skill : SkillTypes)
		{{
			FName SkillId = FName(*FString::Printf(TEXT("%s_%s"), *Zodiac.first, *Skill.first));

			// 加载施法特效
			FString VFXPath = GetSkillVFXPath(SkillId);
			TSoftObjectPtr<UParticleSystem> VFX(VFXPath);
			VFX.LoadSynchronous();
			SkillCastingVFX.Add(SkillId, VFX);

			// 加载施法音效
			FString SFXPath = GetSkillSFXPath(SkillId);
			TSoftObjectPtr<USoundCue> SFX(SFXPath);
			SFX.LoadSynchronous();
			SkillCastingSFX.Add(SkillId, SFX);

			// 加载技能动画
			FString AnimPath = GetSkillAnimPath(SkillId);
			TSoftObjectPtr<UAnimMontage> Montage(AnimPath);
			Montage.LoadSynchronous();
			SkillMontages.Add(SkillId, Montage);
		}}
	}}
}}

void UDBA skillVFXManager::UnloadAllSkillResources()
{{
	SkillCastingVFX.Empty();
	SkillImpactVFX.Empty();
	SkillProjectileVFX.Empty();
	SkillCastingSFX.Empty();
	SkillImpactSFX.Empty();
	SkillMontages.Empty();
}}

void UDBA skillVFXManager::PreloadZodiacResources(FName Zodiac)
{{
	// 加载指定生肖的所有技能资源
}}
"""


def main():
    project_root = Path(__file__).parent.parent / "Source" / "DivineBeastsArena"

    # VFX管理器目录
    vfx_public_dir = project_root / "Public" / "DBA" / "VFX"
    vfx_private_dir = project_root / "Private" / "DBA" / "VFX"
    vfx_public_dir.mkdir(parents=True, exist_ok=True)
    vfx_private_dir.mkdir(parents=True, exist_ok=True)

    # 生成管理器头文件
    header_path = vfx_public_dir / "DBA skillVFXManager.h"
    with open(header_path, "w", encoding="utf-8") as f:
        f.write(SKILL_VFX_MANAGER_HEADER)

    # 生成管理器cpp文件
    cpp_path = vfx_private_dir / "DBA skillVFXManager.cpp"
    with open(cpp_path, "w", encoding="utf-8") as f:
        f.write(SKILL_VFX_MANAGER_CPP)

    print("  生成: DBA skillVFXManager.h/cpp")
    print("\n完成! 共生成:")
    print("  - 1 个技能VFX/SFX管理器")


if __name__ == "__main__":
    main()