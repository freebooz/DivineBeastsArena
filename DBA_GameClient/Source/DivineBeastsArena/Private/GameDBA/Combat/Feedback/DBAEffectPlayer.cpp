// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Combat/Feedback/DBAEffectPlayer.h"

#include "Camera/PlayerCameraManager.h"
#include "GameDBA/Combat/DBANiagaraSkillParameters.h"
#include "GameDBA/Combat/Feedback/DBAEffectTableManager.h"
#include "GameDBA/Combat/Feedback/DBAFloatingDamageComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

UDBAEffectPlayer::UDBAEffectPlayer() {}

void UDBAEffectPlayer::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CachedWorld = GetWorld();
}

void UDBAEffectPlayer::Deinitialize()
{
	EffectTableManager.Reset();
	CachedWorld.Reset();
	Super::Deinitialize();
}

UDBAEffectPlayer* UDBAEffectPlayer::Get(UWorld* World)
{
	return World ? World->GetSubsystem<UDBAEffectPlayer>() : nullptr;
}

void UDBAEffectPlayer::PlayReleaseEffect(AActor* Caster, FName SkillID, FVector Location, FRotator Direction)
{
	FDBASkillEffectRow EffectData;
	if (GetSkillEffectData(SkillID, EffectData))
	{
		SafeSpawnNiagaraEffect(EffectData.ReleaseEffect.Get(), EffectData.NiagaraParameters, 0.0f, Location, Direction.Vector(), Direction);
	}
}

void UDBAEffectPlayer::PlayHitEffect(AActor* Target, FName SkillID, FVector ImpactPoint)
{
	FDBASkillEffectRow EffectData;
	if (GetSkillEffectData(SkillID, EffectData))
	{
		const FVector TargetDirection = Target ? (Target->GetActorLocation() - ImpactPoint).GetSafeNormal() : FVector::UpVector;
		SafeSpawnNiagaraEffect(EffectData.HitEffect.Get(), EffectData.NiagaraParameters, 0.0f, ImpactPoint, TargetDirection, FRotator::ZeroRotator);
	}
}

void UDBAEffectPlayer::PlaySound(AActor* Target, FName SkillID, bool bIsHit)
{
	FDBASkillEffectRow EffectData;
	if (!GetSkillEffectData(SkillID, EffectData))
	{
		return;
	}

	USoundBase* Sound = bIsHit ? EffectData.HitSound.Get() : EffectData.CastSound.Get();
	if (Sound)
	{
		SafePlaySound(Sound, Target ? Target->GetActorLocation() : FVector::ZeroVector, true);
	}
}

void UDBAEffectPlayer::TriggerScreenShake(AActor* Target, FName SkillID, float Scale)
{
	FDBASkillEffectRow EffectData;
	if (!GetSkillEffectData(SkillID, EffectData) || !EffectData.ScreenShakeClass)
	{
		return;
	}

	APlayerController* PC = nullptr;
	if (const ACharacter* Character = Cast<ACharacter>(Target))
	{
		PC = Character->GetController<APlayerController>();
	}
	else if (const APawn* Pawn = Cast<APawn>(Target))
	{
		PC = Pawn->GetController<APlayerController>();
	}
	if (!PC && GetWorld())
	{
		PC = GetWorld()->GetFirstPlayerController();
	}
	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraShake(EffectData.ScreenShakeClass, EffectData.ShakeScale * Scale);
	}
}

void UDBAEffectPlayer::SpawnDamageNumber(AActor* Target, float Damage, bool bIsCritical, uint8 ElementValue, FVector ImpactPoint)
{
	if (UDBAFloatingDamageComponent* DamageComponent = Target ? Target->FindComponentByClass<UDBAFloatingDamageComponent>() : nullptr)
	{
		DamageComponent->SpawnDamageNumber(Damage, bIsCritical, ElementValue, ImpactPoint);
	}
}

void UDBAEffectPlayer::SetEffectTableManager(UDBAEffectTableManager* Manager)
{
	EffectTableManager = Manager;
}

bool UDBAEffectPlayer::GetSkillEffectData(FName SkillID, FDBASkillEffectRow& OutEffectData) const
{
	if (!EffectTableManager.IsValid())
	{
		return false;
	}
	OutEffectData = EffectTableManager->GetSkillEffect(SkillID);
	return OutEffectData.IsLoaded() || OutEffectData.ScreenShakeClass != nullptr;
}

UNiagaraComponent* UDBAEffectPlayer::SafeSpawnNiagaraEffect(
	UNiagaraSystem* System,
	const FDBANiagaraSkillParameters& NiagaraParameters,
	float Damage,
	FVector Location,
	FVector Direction,
	FRotator Rotation)
{
	if (!System || !GetWorld())
	{
		return nullptr;
	}
	UNiagaraComponent* SpawnedComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), System, Location, Rotation, FVector(1.0f), true, true, ENCPoolMethod::AutoRelease, true);
	UDBANiagaraSkillParameterLibrary::ApplySkillParameters(SpawnedComponent, NiagaraParameters, Damage, Location, Direction, 0.0f, NiagaraParameters.EffectRadius);
	return SpawnedComponent;
}

void UDBAEffectPlayer::SafePlaySound(USoundBase* SoundBase, FVector Location, bool bIs3D)
{
	if (!SoundBase || !GetWorld())
	{
		return;
	}
	if (bIs3D)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), SoundBase, Location);
	}
	else
	{
		UGameplayStatics::SpawnSound2D(GetWorld(), SoundBase);
	}
}
