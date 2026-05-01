// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能投射物 - 镇岳神牛技能Q

#pragma once

#include "CoreMinimal.h"
#include "DBAProjectile_Ox_Q.generated.h"
#include "DBA/Combat/DBASkillProjectileBase.h"

/**
 * ADBAProjectile_Ox_Q
 * 镇岳神牛技能Q投射物
 * 用于镇岳神牛的技能Q技能
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAProjectile_Ox_Q : public ADBASkillProjectileBase
{
	GENERATED_BODY()

public:
	ADBAProjectile_Ox_Q();

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
