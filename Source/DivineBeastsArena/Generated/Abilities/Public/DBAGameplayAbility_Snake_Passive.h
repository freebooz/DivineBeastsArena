// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 蛇被动技能 技能类

#pragma once

#include "CoreMinimal.h"
#include "DBA/GAS/Abilities/DBAZodiacAbilityBase.h"
#include "DBAGameplayAbility_Snake_Passive.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API UDBAGameplayAbility_Snake_Passive : public UDBAZodiacAbilityBase
{
	GENERATED_BODY()

public:
	UDBAGameplayAbility_Snake_Passive()
	{
		// 技能配置
		AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Snake.Passive"), false);
		ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

		// 生肖/元素类型
		ZodiacType = EDBAZodiacType::Snake;
		// Passive 技能不绑定特定元素
		ElementType = EDBAElement::None;
	}
};
