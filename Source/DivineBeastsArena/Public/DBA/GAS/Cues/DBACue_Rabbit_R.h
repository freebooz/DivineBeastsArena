// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayCue - 兔终极技能

#pragma once

#include "CoreMinimal.h"
#include "DBA/GAS/Cues/DBACue_Base.h"
#include "Data/DBASkillDataRow.h"
#include "DBACue_Rabbit_R.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API ADBACue_Rabbit_R : public ADBACue_Base
{
	GENERATED_BODY()

public:
	ADBACue_Rabbit_R();

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

	// 技能ID (用于查询数据)
	UPROPERTY(EditDefaultsOnly, Category = "Cue")
	FName SkillId = FName(TEXT("Rabbit_R"));

protected:
	virtual FName GetSkillId() const override { return FName(TEXT("Rabbit_R")); }
};
