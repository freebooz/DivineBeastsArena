// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayCue - 马终极技能

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "DBACue_Horse_R.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API UDBACue_Horse_R : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	UDBACue_Horse_R();

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
