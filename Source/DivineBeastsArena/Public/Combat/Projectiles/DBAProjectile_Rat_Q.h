// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能投射物 - 夜隐灵鼠技能Q

#pragma once

#include "CoreMinimal.h"
#include "DBA/Combat/DBASkillProjectileBase.h"
#include "DBAProjectile_Rat_Q.generated.h"

/**
 * ADBAProjectile_Rat_Q
 * 夜隐灵鼠技能Q投射物
 * 用于夜隐灵鼠的技能Q技能
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAProjectile_Rat_Q : public ADBASkillProjectileBase
{
	GENERATED_BODY()

public:
	ADBAProjectile_Rat_Q();

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
