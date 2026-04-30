// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 蛇E技能 技能类

#pragma once

#include "CoreMinimal.h"
#include "DBA/GAS/Abilities/DBAElementAbilityBase.h"
#include "DBAGameplayAbility_Snake_E.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API UDBAGameplayAbility_Snake_E : public UDBAElementAbilityBase
{
	GENERATED_BODY()

public:
	UDBAGameplayAbility_Snake_E()
	{
		// 技能配置
		AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Snake.E"), false);
		ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

		// 生肖/元素类型
		ZodiacType = EDBAZodiacType::Snake;
		// TODO: 根据英雄主元素设置
		ElementType = EDBAElement::None;
	}
};
