// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 兔Q技能 技能类

#pragma once

#include "CoreMinimal.h"
#include "DBA/GAS/Abilities/DBAElementAbilityBase.h"
#include "DBAGameplayAbility_Rabbit_Q.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API UDBAGameplayAbility_Rabbit_Q : public UDBAElementAbilityBase
{
	GENERATED_BODY()

public:
	UDBAGameplayAbility_Rabbit_Q()
	{
		// 技能配置
		AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Rabbit.Q"), false);
		ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

		// 生肖/元素类型
		ZodiacType = EDBAZodiacType::Rabbit;
		// TODO: 根据英雄主元素设置
		ElementType = EDBAElement::None;
	}
};
