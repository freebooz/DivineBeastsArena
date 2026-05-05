// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能投射物 - 曜鸣神鸡技能E

#pragma once

#include "CoreMinimal.h"
#include "DBA/Combat/DBASkillProjectileBase.h"
#include "DBAProjectile_Rooster_E.generated.h"

/**
 * ADBAProjectile_Rooster_E
 * 曜鸣神鸡技能E投射物
 * 用于曜鸣神鸡的技能E技能
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAProjectile_Rooster_E : public ADBASkillProjectileBase
{
	GENERATED_BODY()

public:
	ADBAProjectile_Rooster_E();

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
