// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能投射物 - 福岳灵猪技能Q

#pragma once

#include "CoreMinimal.h"
#include "DBA/Combat/DBASkillProjectileBase.h"
#include "DBAProjectile_Pig_Q.generated.h"

/**
 * ADBAProjectile_Pig_Q
 * 福岳灵猪技能Q投射物
 * 用于福岳灵猪的技能Q技能
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAProjectile_Pig_Q : public ADBASkillProjectileBase
{
	GENERATED_BODY()

public:
	ADBAProjectile_Pig_Q();

protected:
	virtual void BeginPlay() override;

public:
	/** 初始化投射物 */
	virtual void InitializeProjectile(
		FName InSkillId,
		AActor* InOwner,
		AActor* InTarget,
		float InDamage,
		float InSpeed,
		float InRadius) override;
};
