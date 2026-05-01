// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayCue - 虎Q技能

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "Data/DBASkillDataRow.h"
#include "DBACue_Tiger_Q.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API UDBACue_Tiger_Q : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	UDBACue_Tiger_Q();

	// 当 Cue 被触发时调用
	virtual bool OnExecuteGameplayCue(AActor* Target, FGameplayCueParameters& Parameters) override;

	// 当 Cue 生效时调用 (持续性 Cue)
	virtual void OnActiveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters) override;

	// 当 Cue 结束时调用
	virtual void OnRemoveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters) override;

protected:
	// 从技能数据表加载配置
	void LoadSkillData();

protected:
	// 特效峰值缩放
	UPROPERTY(EditDefaultsOnly, Category = "Cue")
	float CueScale = 1.0f;

	// 技能ID (用于查询数据)
	UPROPERTY(EditDefaultsOnly, Category = "Cue")
	FName SkillId = FName(TEXT("Tiger_Q"));
};
