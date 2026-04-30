// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 鼠W技能 技能类

#pragma once

#include "CoreMinimal.h"
#include "DBA/GAS/Abilities/DBAElementAbilityBase.h"
#include "DBAGameplayAbility_Rat_W.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API UDBAGameplayAbility_Rat_W : public UDBAElementAbilityBase
{
	GENERATED_BODY()

public:
	UDBAGameplayAbility_Rat_W()
	{
		// 技能配置
		AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Rat.W"), false);
		ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

		// 生肖/元素类型
		ZodiacType = EDBAZodiacType::Rat;
		// TODO: 根据英雄主元素设置
		ElementType = EDBAElement::None;
	}
};
