// Copyright Freebooz Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "DBACue_Base.generated.h"

class UDataTable;
struct FDBASkillDataRow;

UCLASS(Abstract)
class DIVINEBEASTSARENA_API ADBACue_Base : public AGameplayCueNotify_Actor
{
    GENERATED_BODY()
public:
    ADBACue_Base();

    virtual bool OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters) override;
    virtual void OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters) override;
    virtual void OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters) override;

protected:
    // Load skill data from DataTable
    void LoadSkillData();

    // Play VFX helper
    void PlayVFX(AActor* Target, float Scale);

    // Play SFX helper
    void PlaySFX(AActor* Target);

    // Get skill ID for this cue (override in subclasses)
    virtual FName GetSkillId() const { return NAME_None; }

    UPROPERTY(EditDefaultsOnly, Category = "Cue")
    float CueScale = 1.0f;
};