// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayCue - 虎E技能

#include "DBA/GAS/Cues/DBACue_Tiger_E.h"
#include "DBA/GAS/DBAAbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

ADBACue_Tiger_E::ADBACue_Tiger_E()
{
	// 加载技能数据
	LoadSkillData();
}

void ADBACue_Tiger_E::LoadSkillData()
{
	// 从DataTable加载技能数据
	UDataTable* SkillTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/Skills/SkillDataTable.SkillDataTable'"));
	if (SkillTable)
	{
		static const FString ContextString = TEXT("DBACue_Tiger_E");
		FDBASkillDataRow* SkillRow = SkillTable->FindRow<FDBASkillDataRow>(FName(TEXT("Tiger_E")), ContextString, false);
		if (SkillRow)
		{
			CueScale = SkillRow->BaseDamage > 0 ? 1.0f + SkillRow->DamageScaling * 0.1f : 1.0f;
		}
	}
}

bool ADBACue_Tiger_E::OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	// 获取技能数据
	FDBASkillDataRow* SkillData = nullptr;
	UDataTable* SkillTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/Skills/SkillDataTable.SkillDataTable'"));
	if (SkillTable)
	{
		static const FString ContextString = TEXT("OnExecuteGameplayCue");
		SkillData = SkillTable->FindRow<FDBASkillDataRow>(SkillId, ContextString, false);
	}

	// 播放视觉效果
	if (SkillData && SkillData->VFXAsset.IsValid())
	{
		UParticleSystem* VFX = SkillData->VFXAsset.LoadSynchronous();
		if (VFX)
		{
			FVector Location = Target ? Target->GetActorLocation() : FVector::ZeroVector;
			FRotator Rotation = Target ? Target->GetActorRotation() : FRotator::ZeroRotator;
			UGameplayStatics::SpawnEmitterAtLocation(Target, VFX, Location, Rotation, true);
		}
	}

	// 播放音效
	if (SkillData && SkillData->SFXAsset.IsValid())
	{
		USoundBase* SFX = Cast<USoundBase>(SkillData->SFXAsset.LoadSynchronous());
		if (SFX)
		{
			UGameplayStatics::PlaySoundAtLocation(Target, SFX, Target ? Target->GetActorLocation() : FVector::ZeroVector);
		}
	}

	// 通过ASC广播技能事件（已注释，因为OnSkillCueExecuted不存在）
	AActor* OwnerActor = GetOwner();
	if (OwnerActor)
	{
		UDBAAbilitySystemComponent* ASC = OwnerActor->FindComponentByClass<UDBAAbilitySystemComponent>();
		if (ASC)
		{
			// ASC->OnSkillCueExecuted.Broadcast(SkillId, Target);
		}
	}

	return true;
}

void ADBACue_Tiger_E::OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	// 获取技能数据，检查是否有引导特效
	FDBASkillDataRow* SkillData = nullptr;
	UDataTable* SkillTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/Skills/SkillDataTable.SkillDataTable'"));
	if (SkillTable)
	{
		static const FString ContextString = TEXT("OnActiveGameplayCue");
		SkillData = SkillTable->FindRow<FDBASkillDataRow>(SkillId, ContextString, false);
	}

	// 播放持续性特效（如引导、充能）
	if (SkillData && SkillData->VFXAsset.IsValid())
	{
		UParticleSystem* VFX = SkillData->VFXAsset.LoadSynchronous();
		if (VFX)
		{
			FVector Location = Target ? Target->GetActorLocation() : FVector::ZeroVector;
			FRotator Rotation = Target ? Target->GetActorRotation() : FRotator::ZeroRotator;
			UGameplayStatics::SpawnEmitterAtLocation(Target, VFX, Location, Rotation, true);
		}
	}
}

void ADBACue_Tiger_E::OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	// 清理特效 - 在目标位置停止所有相关粒子系统
	if (Target)
	{
		TArray<UParticleSystemComponent*> ParticleComponents;
		Target->GetComponents<UParticleSystemComponent>(ParticleComponents);
		for (UParticleSystemComponent* Particle : ParticleComponents)
		{
			if (Particle && Particle->bIsActive)
			{
				// 检查是否是此Cue创建的特效（通过Tag标记）
				if (Particle->ComponentHasTag(FName(TEXT("Cue_Tiger_E"))))
				{
					Particle->Deactivate();
				}
			}
		}
	}
}
