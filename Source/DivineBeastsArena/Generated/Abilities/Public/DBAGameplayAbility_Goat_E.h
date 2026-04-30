// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 羊E技能 技能类

#pragma once

#include "CoreMinimal.h"
#include "DBA/GAS/Abilities/DBAElementAbilityBase.h"
#include "DBAGameplayAbility_Goat_E.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API UDBAGameplayAbility_Goat_E : public UDBAElementAbilityBase
{
	GENERATED_BODY()

public:
	UDBAGameplayAbility_Goat_E()
	{
		// 技能配置
		AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Goat.E"), false);
		ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

		// 生肖/元素类型
		ZodiacType = EDBAZodiacType::Goat;
		// TODO: 根据英雄主元素设置
		ElementType = EDBAElement::None;
	}
};
