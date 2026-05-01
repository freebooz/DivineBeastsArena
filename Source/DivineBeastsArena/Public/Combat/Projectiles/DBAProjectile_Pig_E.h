// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能投射物 - 福岳灵猪技能E

#pragma once

#include "CoreMinimal.h"
#include "DBAProjectile_Pig_E.generated.h"
#include "DBA/Combat/DBASkillProjectileBase.h"

/**
 * ADBAProjectile_Pig_E
 * 福岳灵猪技能E投射物
 * 用于福岳灵猪的技能E技能
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAProjectile_Pig_E : public ADBASkillProjectileBase
{
	GENERATED_BODY()

public:
	ADBAProjectile_Pig_E();

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
