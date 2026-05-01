// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayCue - 猴被动技能

#pragma once

#include "CoreMinimal.h"
#include "DBA/GAS/Cues/DBACue_Base.h"
#include "Data/DBASkillDataRow.h"
#include "DBACue_Monkey_Passive.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API ADBACue_Monkey_Passive : public ADBACue_Base
{
	GENERATED_BODY()

public:
	ADBACue_Monkey_Passive();

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
	FName SkillId = FName(TEXT("Monkey_Passive"));

protected:
	virtual FName GetSkillId() const override { return FName(TEXT("Monkey_Passive")); }
};
