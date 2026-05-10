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

    virtual bool OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters);
    virtual void OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters);
    virtual void OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters);

protected:
    // Load skill data from DataTable
    void LoadSkillData();

    // Play VFX helper
    void PlayVFX(AActor* Target, float Scale);

    // Play SFX helper
    void PlaySFX(AActor* Target);

    /** 获取技能ID - 子类可重写 */
    virtual FName GetSkillId() const { return SkillId; }

    /** 技能ID (可在蓝图中配置) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cue")
    FName SkillId = NAME_None;

    UPROPERTY(EditDefaultsOnly, Category = "Cue")
    float CueScale = 1.0f;
};