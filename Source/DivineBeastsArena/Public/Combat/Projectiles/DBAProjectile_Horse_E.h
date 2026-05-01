// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能投射物 - 踏风天驹技能E

#pragma once

#include "CoreMinimal.h"
#include "DBA/Combat/DBASkillProjectileBase.h"
#include "DBAProjectile_Horse_E.generated.h"

/**
 * ADBAProjectile_Horse_E
 * 踏风天驹技能E投射物
 * 用于踏风天驹的技能E技能
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAProjectile_Horse_E : public ADBASkillProjectileBase
{
	GENERATED_BODY()

public:
	ADBAProjectile_Horse_E();

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
