// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayCue - 蛇被动技能

#include "DBA/GAS/Cues/DBACue_Snake_Passive.h"
#include "DBA/GAS/DBAAbilitySystemComponent.h"
#include "Data/DBADataAsset.h"
#include "Kismet/GameplayStatics.h"

UDBACue_Snake_Passive::UDBACue_Snake_Passive()
{
	// 默认配置
	bAutoDestroyOnOwnerRemoved = true;
	bOnlyRelevantToOwner = false;
	RollbackPolicy = ECueRollback::CanRollback;

	// 加载技能数据
	LoadSkillData();
}

void UDBACue_Snake_Passive::LoadSkillData()
{
	// 从 DataTable 加载技能数据
	UDataTable* SkillTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/Skills/SkillDataTable.SkillDataTable'"));
	if (SkillTable)
	{
		static const FString ContextString = TEXT("DBACue_Snake_Passive");
		FDBASkillDataRow* SkillRow = SkillTable->FindRow<FDBASkillDataRow>(FName(TEXT("Snake_Passive")), ContextString, false);
		if (SkillRow)
		{
			CueScale = SkillRow->BaseDamage > 0 ? 1.0f + SkillRow->DamageScaling * 0.1f : 1.0f;
		}
	}
}

bool UDBACue_Snake_Passive::OnExecuteGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
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
			UGameplayStatics::SpawnEmitterAtLocation(Target, VFX, Location, Rotation, CueScale);
		}
	}

	// 播放音效
	if (SkillData && SkillData->SFXAsset.IsValid())
	{
		USoundCue* SFX = SkillData->SFXAsset.LoadSynchronous();
		if (SFX)
		{
			UGameplayStatics::PlaySoundAtLocation(Target, SFX, Target ? Target->GetActorLocation() : FVector::ZeroVector);
		}
	}

	// 通过 ASC 广播技能事件
	if (AActor* Owner = GetOwningActor())
	{
		if (UDBAAbilitySystemComponent* ASC = Owner->FindComponentByClass<UDBAAbilitySystemComponent>())
		{
			ASC->OnSkillCueExecuted.Broadcast(SkillId, Target);
		}
	}

	return true;
}

void UDBACue_Snake_Passive::OnActiveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
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
			UGameplayStatics::SpawnEmitterAtLocation(Target, VFX, Location, Rotation, CueScale * 0.7f);
		}
	}
}

void UDBACue_Snake_Passive::OnRemoveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
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
				// 检查是否是此 Cue 创建的特效（通过 Tag 标记）
				if (Particle->ComponentHasTag(FName(TEXT("Cue_Snake_Passive"))))
				{
					Particle->Deactivate();
				}
			}
		}
	}
}
