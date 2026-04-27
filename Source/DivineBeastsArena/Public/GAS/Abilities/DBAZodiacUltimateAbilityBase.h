// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/DBAZodiacAbilityBase.h"
#include "DBAZodiacUltimateAbilityBase.generated.h"

/**
 * 生肖大招基类 (ZodiacUltimate)
 * 必须消耗 100 点 UltimateEnergy
 */
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBAZodiacUltimateAbilityBase : public UDBAZodiacAbilityBase
{
	GENERATED_BODY()

public:
	UDBAZodiacUltimateAbilityBase();

protected:
	/**
	 * 检查 UltimateEnergy 是否达到 100
	 * 服务端与客户端预测均执行
	 */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	/**
	 * 消耗 100 点 UltimateEnergy
	 */
	virtual bool CommitAbilityCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
};
