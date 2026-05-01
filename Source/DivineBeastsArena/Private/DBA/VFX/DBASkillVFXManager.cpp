// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能VFX/SFX管理器

#include "DBA/VFX/DBASkillVFXManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"

UDBASkillVFXManager::UDBASkillVFXManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDBASkillVFXManager::PlaySkillVFX(FName SkillId, AActor* Target, AActor* Owner)
{
	if (!Owner || SkillId == NAME_None)
		return;

	FString VFXPath = GetSkillVFXPath(SkillId);
	if (VFXPath.IsEmpty())
		return;

	// 动态加载VFX资源
	FSoftObjectPath VFXAssetPath(VFXPath);
	TSoftObjectPtr<UParticleSystem> VFXAsset(VFXAssetPath);

	if (UParticleSystem* VFX = VFXAsset.LoadSynchronous())
	{
		FVector Location = Target ? Target->GetActorLocation() : Owner->GetActorLocation();
		FRotator Rotation = Target ? Target->GetActorRotation() : Owner->GetActorRotation();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}
}

void UDBASkillVFXManager::StopSkillVFX(FName SkillId)
{
	// 对于点状特效，停止意味着销毁
	// 对于持续特效，可以在子类中实现更复杂的停止逻辑
}

void UDBASkillVFXManager::PlaySkillSFX(FName SkillId, AActor* Owner)
{
	if (!Owner || SkillId == NAME_None)
		return;

	FString SFXPath = GetSkillSFXPath(SkillId);
	if (SFXPath.IsEmpty())
		return;

	// 动态加载SFX资源
	FSoftObjectPath SFXAssetPath(SFXPath);
	TSoftObjectPtr<USoundCue> SFXAsset(SFXAssetPath);

	if (USoundBase* SFX = SFXAsset.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, Owner->GetActorLocation());
	}
}

void UDBASkillVFXManager::StopSkillSFX(FName SkillId)
{
	// 音效停止逻辑
}

void UDBASkillVFXManager::PreloadAllSkillResources()
{
	// 预加载所有技能资源
	// 遍历所有TMap，调用LoadSynchronous()
	for (auto& Pair : SkillCastingVFX)
	{
		if (Pair.Value.IsValid())
		{
			Pair.Value.LoadSynchronous();
		}
	}
	for (auto& Pair : SkillImpactVFX)
	{
		if (Pair.Value.IsValid())
		{
			Pair.Value.LoadSynchronous();
		}
	}
	for (auto& Pair : SkillCastingSFX)
	{
		if (Pair.Value.IsValid())
		{
			Pair.Value.LoadSynchronous();
		}
	}
	for (auto& Pair : SkillImpactSFX)
	{
		if (Pair.Value.IsValid())
		{
			Pair.Value.LoadSynchronous();
		}
	}
}

void UDBASkillVFXManager::UnloadAllSkillResources()
{
	// 卸载所有技能资源
	// 对于TSoftObjectPtr，不需要显式卸载，让GC处理
}

FString UDBASkillVFXManager::GetSkillVFXPath(FName SkillId)
{
	// 从配置表或资源映射中获取VFX路径
	// 这里返回空字符串，子类或蓝图可以实现具体逻辑
	return FString();
}

FString UDBASkillVFXManager::GetSkillSFXPath(FName SkillId)
{
	// 从配置表或资源映射中获取SFX路径
	return FString();
}