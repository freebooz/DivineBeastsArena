// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/GAS/DBAMobaGameplayAbilityBase.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "GameMoba/RPC/DBARpcServer.h"
#include "DBAZodiacAbilityBase.generated.h"

/**
 * 鐢熻倴鎶€鑳藉熀绫?(Passive 绛?
 * 鍐冲畾鑻遍泟韬唤銆佸瑙傚壀褰便€佸姩鐢诲熀璋? */
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBAZodiacAbilityBase : public UDBAMobaGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UDBAZodiacAbilityBase();

	/** 鎵€灞炵敓鑲栨爣璇?*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Zodiac")
	EDBAZodiacType ZodiacType;

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
};

