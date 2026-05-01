// Copyright Freebooz Games, Inc. All Rights Reserved.
#include "DBA/GAS/Cues/DBACue_Base.h"
#include "Data/DBASkillDataRow.h"
#include "Kismet/GameplayStatics.h"

ADBACue_Base::ADBACue_Base()
{
    bAutoDestroyOnOwnerRemoved = true;
    bOnlyRelevantToOwner = false;
}

bool ADBACue_Base::OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
    LoadSkillData();

    FDBASkillDataRow* SkillData = nullptr;
    UDataTable* SkillTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/Skills/SkillDataTable.SkillDataTable'"));
    if (SkillTable)
    {
        SkillData = SkillTable->FindRow<FDBASkillDataRow>(GetSkillId(), TEXT("OnExecute"), false);
    }

    if (SkillData && SkillData->VFXAsset.IsValid())
    {
        if (UParticleSystem* VFX = SkillData->VFXAsset.LoadSynchronous())
        {
            FVector Location = Target ? Target->GetActorLocation() : FVector::ZeroVector;
            FRotator Rotation = Target ? Target->GetActorRotation() : FRotator::ZeroRotator;
            UGameplayStatics::SpawnEmitterAtLocation(Target, VFX, Location, Rotation, CueScale);
        }
    }

    if (SkillData && SkillData->SFXAsset.IsValid())
    {
        if (USoundCue* SFX = SkillData->SFXAsset.LoadSynchronous())
        {
            UGameplayStatics::PlaySoundAtLocation(Target, SFX, Target ? Target->GetActorLocation() : FVector::ZeroVector);
        }
    }

    return true;
}

void ADBACue_Base::OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
    LoadSkillData();
    PlayVFX(Target, CueScale * 0.7f);
}

void ADBACue_Base::OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
    // Cleanup logic
}

void ADBACue_Base::LoadSkillData()
{
    // Implemented in subclasses
}

void ADBACue_Base::PlayVFX(AActor* Target, float Scale)
{
    // Get VFX from skill data and play
}

void ADBACue_Base::PlaySFX(AActor* Target)
{
    // Get SFX from skill data and play
}