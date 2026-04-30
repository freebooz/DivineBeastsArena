// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayCue - 猪被动技能

#pragma once

#include "CoreMinimal.h"
#include "DBACue_Pig_Passive.generated.h"
#include "GameplayCueNotify_Actor.h"

UCLASS()
class DIVINEBEASTSARENA_API UDBACue_Pig_Passive : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	UDBACue_Pig_Passive();

	// 当 Cue 被触发时调用
	virtual bool OnExecuteGameplayCue(AActor* Target, FGameplayCueParameters& Parameters) override;

	// 当 Cue 生效时调用 (持续性 Cue)
	virtual void OnActiveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters) override;

	// 当 Cue 结束时调用
	virtual void OnRemoveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters) override;

protected:
	// 特效峰值缩放
	UPROPERTY(EditDefaultsOnly, Category = "Cue")
	float CueScale = 1.0f;
};
