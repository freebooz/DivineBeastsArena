// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 泛化生肖VFX组件实现

#include "GameDBA/Presentation/VFX/Components/DBAZodiacVFXComponent_Generic.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameCore/Async/DBAAsyncAssetLoader.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UDBAZodiacVFXComponent_Generic::UDBAZodiacVFXComponent_Generic()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDBAZodiacVFXComponent_Generic::BeginPlay()
{
	Super::BeginPlay();
	PreloadPresentationAssets();
}

void UDBAZodiacVFXComponent_Generic::LoadDefaultAssets()
{
	UE_LOG(LogDBACombat, Log, TEXT("[DBAZodiacVFXComponent_Generic] 加载默认资源: ZodiacType=%d"), (uint8)ZodiacType);
}

void UDBAZodiacVFXComponent_Generic::PreloadPresentationAssets()
{
	TArray<FSoftObjectPath> Paths;
	DBAAsyncAssetLoader::AddPreloadPath(AttackVFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(HitVFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(MoveVFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(DeathVFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(RespawnVFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(AttackSFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(HitSFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(MoveSFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(DeathSFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(AttackMontage, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(HitMontage, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(DeathMontage, Paths);
	for (const TPair<FName, TSoftObjectPtr<UAnimMontage>>& Pair : SkillMontages)
	{
		DBAAsyncAssetLoader::AddPreloadPath(Pair.Value, Paths);
	}
	DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
}

void UDBAZodiacVFXComponent_Generic::PlayAttackVFX(AActor* Target)
{
	if (UParticleSystem* VFX = AttackVFX.Get())
	{
		FVector Location = Target ? Target->GetActorLocation() : GetOwner()->GetActorLocation();
		FRotator Rotation = GetOwner()->GetActorRotation();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}
}

void UDBAZodiacVFXComponent_Generic::PlayHitVFX(AActor* Attacker)
{
	if (UParticleSystem* VFX = HitVFX.Get())
	{
		FVector Location = GetOwner()->GetActorLocation();
		FRotator Rotation = FRotator::ZeroRotator;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}
}

void UDBAZodiacVFXComponent_Generic::PlayMoveVFX(const FVector& Direction)
{
	if (UParticleSystem* VFX = MoveVFX.Get())
	{
		FVector Location = GetOwner()->GetActorLocation();
		FRotator Rotation = Direction.Rotation();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}
}

void UDBAZodiacVFXComponent_Generic::PlayDeathVFX()
{
	if (UParticleSystem* VFX = DeathVFX.Get())
	{
		FVector Location = GetOwner()->GetActorLocation();
		FRotator Rotation = FRotator::ZeroRotator;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}
}

void UDBAZodiacVFXComponent_Generic::PlayRespawnVFX()
{
	if (UParticleSystem* VFX = RespawnVFX.Get())
	{
		FVector Location = GetOwner()->GetActorLocation();
		FRotator Rotation = FRotator::ZeroRotator;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}
}

void UDBAZodiacVFXComponent_Generic::PlayAttackSFX()
{
	if (USoundBase* SFX = AttackSFX.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}
}

void UDBAZodiacVFXComponent_Generic::PlayHitSFX()
{
	if (USoundBase* SFX = HitSFX.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}
}

void UDBAZodiacVFXComponent_Generic::PlayMoveSFX()
{
	if (USoundBase* SFX = MoveSFX.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}
}

void UDBAZodiacVFXComponent_Generic::PlayDeathSFX()
{
	if (USoundBase* SFX = DeathSFX.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}
}

void UDBAZodiacVFXComponent_Generic::PlaySkillSFX(FName SkillId)
{
	UE_LOG(LogDBACombat, Verbose, TEXT("[DBAZodiacVFXComponent_Generic] 播放技能音效: %s"), *SkillId.ToString());
}

void UDBAZodiacVFXComponent_Generic::PlayAttackAnimation()
{
	if (UAnimMontage* Montage = AttackMontage.Get())
	{
		if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				AnimInstance->Montage_Play(Montage);
			}
		}
	}
}

void UDBAZodiacVFXComponent_Generic::PlayHitAnimation()
{
	if (UAnimMontage* Montage = HitMontage.Get())
	{
		if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				AnimInstance->Montage_Play(Montage);
			}
		}
	}
}

void UDBAZodiacVFXComponent_Generic::PlayMoveAnimation(float Speed)
{
	if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{
		// 计算速度倍率并设置动画播放速率
		float SpeedMultiplier = Speed / DBAConstants::AnimationSpeedBase;
		Mesh->GlobalAnimRateScale = SpeedMultiplier;
	}
}

void UDBAZodiacVFXComponent_Generic::PlayDeathAnimation()
{
	if (UAnimMontage* Montage = DeathMontage.Get())
	{
		if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				AnimInstance->Montage_Play(Montage);
			}
		}
	}
}

void UDBAZodiacVFXComponent_Generic::PlaySkillAnimation(FName SkillId)
{
	if (TSoftObjectPtr<UAnimMontage>* MontagePtr = SkillMontages.Find(SkillId))
	{
		if (UAnimMontage* Montage = MontagePtr->Get())
		{
			if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
			{
				if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
				{
					AnimInstance->Montage_Play(Montage);
				}
			}
		}
	}
}

UAnimBlueprint* UDBAZodiacVFXComponent_Generic::GetAnimBlueprint() const
{
	if (UClass* BPClass = AnimBlueprintClass.Get())
	{
		return Cast<UAnimBlueprint>(BPClass->GetDefaultObject());
	}
	return nullptr;
}
