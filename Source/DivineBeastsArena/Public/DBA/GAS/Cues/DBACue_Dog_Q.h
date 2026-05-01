// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayCue - 狗Q技能

#pragma once

#include "CoreMinimal.h"
#include "DBA/GAS/Cues/DBACue_Base.h"
#include "Data/DBASkillDataRow.h"
#include "DBACue_Dog_Q.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API ADBACue_Dog_Q : public ADBACue_Base
{
	GENERATED_BODY()

public:
	ADBACue_Dog_Q();

	// 当 Cue 被触发时调用
	virtual bool OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters);

	// 当 Cue 生效时调用 (持续性 Cue)
	virtual void OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters);

	// 当 Cue 结束时调用
	virtual void OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters);

protected:
	// 从技能数据表加载配置
	void LoadSkillData();

protected:
	// 特效峰值缩放
	UPROPERTY(EditDefaultsOnly, Category = "Cue")
	float CueScale = 1.0f;

	// 技能ID (用于查询数据)
	UPROPERTY(EditDefaultsOnly, Category = "Cue")
	FName SkillId = FName(TEXT("Dog_Q"));

protected:
	virtual FName GetSkillId() const override { return FName(TEXT("Dog_Q")); }
};
