// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/DBAZodiacAbilityBase.h"
#include "Common/Types/DBACommonEnums.h"
#include "DBAElementAbilityBase.generated.h"

/**
 * 自然元素之力技能基类 (Active Skill01~04)
 * 决定克制、属性加成、技能元素
 */
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBAElementAbilityBase : public UDBAMobaGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UDBAElementAbilityBase();

	/**
	 * 获取技能所属的自然元素类型
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Element")
	EDBAElement GetElementType() const { return ElementType; }

	/** 所属自然元素之力 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Element")
	EDBAElement ElementType = EDBAElement::None;

	/** 是否消耗 CurrentEnergy */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Element")
	float EnergyCost;

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual bool CommitAbilityCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;
};
