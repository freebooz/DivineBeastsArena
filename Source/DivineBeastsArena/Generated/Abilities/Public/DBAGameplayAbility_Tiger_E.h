// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 虎E技能 技能类

#pragma once

#include "CoreMinimal.h"
#include "DBA/GAS/Abilities/DBAElementAbilityBase.h"
#include "DBAGameplayAbility_Tiger_E.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API UDBAGameplayAbility_Tiger_E : public UDBAElementAbilityBase
{
	GENERATED_BODY()

public:
	UDBAGameplayAbility_Tiger_E()
	{
		// 技能配置
		AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Tiger.E"), false);
		ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

		// 生肖/元素类型
		ZodiacType = EDBAZodiacType::Tiger;
		// TODO: 根据英雄主元素设置
		ElementType = EDBAElement::None;
	}
};
