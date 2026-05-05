// Copyright Freebooz Games, Inc. All Rights Reserved.
// 冷却效果

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "DBAGE_Cooldown.generated.h"

/**
 * UDBAGE_Cooldown
 * 冷却GameplayEffect
 * 用于应用技能冷却时间
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAGE_Cooldown : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDBAGE_Cooldown();

	/** 冷却持续时间 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown", meta = (UIMin = 0.1, UIMax = 60.0))
	float CooldownDuration = 1.0f;

protected:
	virtual void PostLoad() override;
};