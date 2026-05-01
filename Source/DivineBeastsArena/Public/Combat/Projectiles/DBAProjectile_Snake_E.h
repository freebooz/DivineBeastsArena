// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能投射物 - 玄花灵蛇技能E

#pragma once

#include "CoreMinimal.h"
#include "DBAProjectile_Snake_E.generated.h"
#include "DBA/Combat/DBASkillProjectileBase.h"

/**
 * ADBAProjectile_Snake_E
 * 玄花灵蛇技能E投射物
 * 用于玄花灵蛇的技能E技能
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAProjectile_Snake_E : public ADBASkillProjectileBase
{
	GENERATED_BODY()

public:
	ADBAProjectile_Snake_E();

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
