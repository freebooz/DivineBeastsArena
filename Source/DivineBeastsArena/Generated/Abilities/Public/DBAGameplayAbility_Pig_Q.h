// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 猪Q技能 技能类

#pragma once

#include "CoreMinimal.h"
#include "DBA/GAS/Abilities/DBAElementAbilityBase.h"
#include "DBAGameplayAbility_Pig_Q.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API UDBAGameplayAbility_Pig_Q : public UDBAElementAbilityBase
{
	GENERATED_BODY()

public:
	UDBAGameplayAbility_Pig_Q()
	{
		// 技能配置
		AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Pig.Q"), false);
		ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

		// 生肖/元素类型
		ZodiacType = EDBAZodiacType::Pig;
		// TODO: 根据英雄主元素设置
		ElementType = EDBAElement::None;
	}
};
