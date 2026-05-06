// Copyright Freebooz Games, Inc. All Rights Reserved.
// 怪物仇恨信息结构

#pragma once

#include "CoreMinimal.h"
#include "FAggroInfo.generated.h"

/**
 * FAggroInfo
 * 怪物仇恨信息结构
 * 用于记录每个目标的仇恨值
 */
USTRUCT(BlueprintType)
struct FAggroInfo
{
	GENERATED_BODY()

public:
	/** 目标Actor */
	UPROPERTY()
	TWeakObjectPtr<AActor> Target;

	/** 仇恨值 */
	UPROPERTY()
	float Threat = 0.0f;

	/** 最后仇恨时间戳 */
	UPROPERTY()
	float LastThreatTime = 0.0f;

public:
	FAggroInfo() {}

	FAggroInfo(AActor* InTarget, float InThreat)
		: Target(InTarget)
		, Threat(InThreat)
		, LastThreatTime(GetWorldTime(InTarget))
	{}

	bool IsValid() const
	{
		return Target.IsValid() && Threat > 0.0f;
	}

	void UpdateThreat(float NewThreat)
	{
		Threat = NewThreat;
		LastThreatTime = GetWorldTime(Target.Get());
	}

	void AddThreat(float Delta)
	{
		Threat += Delta;
		LastThreatTime = GetWorldTime(Target.Get());
	}

private:
	static float GetWorldTime(const AActor* Actor)
	{
		return Actor && Actor->GetWorld() ? Actor->GetWorld()->GetTimeSeconds() : 0.0f;
	}
};
