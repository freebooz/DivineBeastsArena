// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能投射物 - 月华灵兔技能E

#pragma once

#include "CoreMinimal.h"
#include "DBAProjectile_Rabbit_E.generated.h"
#include "DBA/Combat/DBASkillProjectileBase.h"

/**
 * ADBAProjectile_Rabbit_E
 * 月华灵兔技能E投射物
 * 用于月华灵兔的技能E技能
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAProjectile_Rabbit_E : public ADBASkillProjectileBase
{
	GENERATED_BODY()

public:
	ADBAProjectile_Rabbit_E();

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
