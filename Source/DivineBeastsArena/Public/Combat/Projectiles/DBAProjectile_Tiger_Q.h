// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能投射物 - 裂风虎君技能Q

#pragma once

#include "CoreMinimal.h"
#include "DBAProjectile_Tiger_Q.generated.h"
#include "DBA/Combat/DBASkillProjectileBase.h"

/**
 * ADBAProjectile_Tiger_Q
 * 裂风虎君技能Q投射物
 * 用于裂风虎君的技能Q技能
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAProjectile_Tiger_Q : public ADBASkillProjectileBase
{
	GENERATED_BODY()

public:
	ADBAProjectile_Tiger_Q();

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
