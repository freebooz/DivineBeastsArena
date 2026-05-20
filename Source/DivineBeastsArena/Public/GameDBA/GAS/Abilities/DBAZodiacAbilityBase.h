// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/GAS/DBAMobaGameplayAbilityBase.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "GameMoba/RPC/DBARpcServer.h"
#include "DBAZodiacAbilityBase.generated.h"

/**
 * 生肖技能基类(Passive 技能)
 * 决定英雄身份、外观剑影、动画基类 */
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBAZodiacAbilityBase : public UDBAMobaGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UDBAZodiacAbilityBase();

	/** 所属生肖标志 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Zodiac")
	EDBAZodiacType ZodiacType;

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
};

