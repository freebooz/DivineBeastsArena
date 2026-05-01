// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayCue - 龙W技能

#include "DBA/GAS/Cues/DBACue_Dragon_W.h"
#include "DBA/GAS/DBAAbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"

ADBACue_Dragon_W::ADBACue_Dragon_W()
{
	// 默认配置
	// bAutoDestroyOnOwnerRemoved = true;
	// bOnlyRelevantToOwner = false;
	// RollbackPolicy = ECueRollback::CanRollback;

	// 加载技能数据
	LoadSkillData();
}

void ADBACue_Dragon_W::LoadSkillData()
{
	// 从 DataTable 加载技能数据
	UDataTable* SkillTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/Skills/SkillDataTable.SkillDataTable'"));
	if (SkillTable)
	{
		static const FString ContextString = TEXT("DBACue_Dragon_W");
		FDBASkillDataRow* SkillRow = SkillTable->FindRow<FDBASkillDataRow>(FName(TEXT("Dragon_W")), ContextString, false);
		if (SkillRow)
		{
			CueScale = SkillRow->BaseDamage > 0 ? 1.0f + SkillRow->DamageScaling * 0.1f : 1.0f;
		}
	}
}

bool ADBACue_Dragon_W::OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
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
		USoundBase* SFX = Cast<USoundBase>(SkillData->SFXAsset.LoadSynchronous());
		if (SFX)
		{
			UGameplayStatics::PlaySoundAtLocation(Target, SFX, Target ? Target->GetActorLocation() : FVector::ZeroVector);
		}
	}

	// 通过 ASC 广播技能事件
	AActor* OwnerActor = GetOwner();
	{
		if (UDBAAbilitySystemComponent* ASC = OwnerActor ? OwnerActor->FindComponentByClass<UDBAAbilitySystemComponent>())
		{
			ASC->OnSkillCueExecuted.Broadcast(SkillId, Target);
		}
	}

	return true;
}

void ADBACue_Dragon_W::OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
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

void ADBACue_Dragon_W::OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
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
				if (Particle->ComponentHasTag(FName(TEXT("Cue_Dragon_W"))))
				{
					Particle->Deactivate();
				}
			}
		}
	}
}
