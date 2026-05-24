// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 技能VFX/SFX管理器

#include "GameDBA/VFX/DBASkillVFXManager.h"
#include "GameDBA/Utilities/DBAAsyncAssetLoader.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"

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

	FSoftObjectPath VFXAssetPath(VFXPath);
	TSoftObjectPtr<UParticleSystem> VFXAsset(VFXAssetPath);

	if (UParticleSystem* VFX = VFXAsset.Get())
	{
		FVector Location = Target ? Target->GetActorLocation() : Owner->GetActorLocation();
		FRotator Rotation = Target ? Target->GetActorRotation() : Owner->GetActorRotation();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}
	else
	{
		TArray<FSoftObjectPath> Paths;
		Paths.Add(VFXAssetPath);
		DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
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

	FSoftObjectPath SFXAssetPath(SFXPath);
	TSoftObjectPtr<USoundBase> SFXAsset(SFXAssetPath);

	if (USoundBase* SFX = SFXAsset.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, Owner->GetActorLocation());
	}
	else
	{
		TArray<FSoftObjectPath> Paths;
		Paths.Add(SFXAssetPath);
		DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
	}
}

void UDBASkillVFXManager::StopSkillSFX(FName SkillId)
{
	// 音效停止逻辑
}

void UDBASkillVFXManager::PreloadAllSkillResources()
{
	TArray<FSoftObjectPath> Paths;
	for (auto& Pair : SkillCastingVFX)
	{
		DBAAsyncAssetLoader::AddPreloadPath(Pair.Value, Paths);
	}
	for (auto& Pair : SkillImpactVFX)
	{
		DBAAsyncAssetLoader::AddPreloadPath(Pair.Value, Paths);
	}
	for (auto& Pair : SkillCastingSFX)
	{
		DBAAsyncAssetLoader::AddPreloadPath(Pair.Value, Paths);
	}
	for (auto& Pair : SkillImpactSFX)
	{
		DBAAsyncAssetLoader::AddPreloadPath(Pair.Value, Paths);
	}
	DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
}

void UDBASkillVFXManager::UnloadAllSkillResources()
{
	// 卸载所有技能资源
	// 对于TSoftObjectPtr，不需要显式卸载，让GC处理
}

FString UDBASkillVFXManager::GetSkillVFXPath(FName SkillId)
{
	if (const TSoftObjectPtr<UParticleSystem>* CastingVFX = SkillCastingVFX.Find(SkillId))
	{
		return CastingVFX->ToSoftObjectPath().ToString();
	}
	if (const TSoftObjectPtr<UParticleSystem>* ImpactVFX = SkillImpactVFX.Find(SkillId))
	{
		return ImpactVFX->ToSoftObjectPath().ToString();
	}
	return FString();
}

FString UDBASkillVFXManager::GetSkillSFXPath(FName SkillId)
{
	if (const TSoftObjectPtr<USoundBase>* CastingSFX = SkillCastingSFX.Find(SkillId))
	{
		return CastingSFX->ToSoftObjectPath().ToString();
	}
	if (const TSoftObjectPtr<USoundBase>* ImpactSFX = SkillImpactSFX.Find(SkillId))
	{
		return ImpactSFX->ToSoftObjectPath().ToString();
	}
	return FString();
}
