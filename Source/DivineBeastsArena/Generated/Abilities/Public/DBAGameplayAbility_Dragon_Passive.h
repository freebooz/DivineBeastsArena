// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 龙被动技能 技能类

#pragma once

#include "CoreMinimal.h"
#include "DBA/GAS/Abilities/DBAZodiacAbilityBase.h"
#include "DBAGameplayAbility_Dragon_Passive.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API UDBAGameplayAbility_Dragon_Passive : public UDBAZodiacAbilityBase
{
	GENERATED_BODY()

public:
	UDBAGameplayAbility_Dragon_Passive()
	{
		// 技能配置
		AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Dragon.Passive"), false);
		ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

		// 生肖/元素类型
		ZodiacType = EDBAZodiacType::Dragon;
		// Passive 技能不绑定特定元素
		ElementType = EDBAElement::None;
	}
};
