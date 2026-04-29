// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DBA/GAS/Abilities/DBAZodiacAbilityBase.h"
#include "DBAResonanceAbilityBase.generated.h"

/**
 * 自然元素共鸣被动基类 (Resonance)
 * 自动授予，不绑定输入，根据 ResonanceLevel 提供 Buff
 */
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBAResonanceAbilityBase : public UDBAMobaGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UDBAResonanceAbilityBase();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/**
	 * 应用共鸣效果
	 * - Lv.1: 控制时间 +0.25 秒，护盾值 +5%
	 * - Lv.2: 控制时间 +0.50 秒，护盾值 +10%
	 * ...
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Resonance")
	void ApplyResonanceEffect(int32 CurrentResonanceLevel);
};
