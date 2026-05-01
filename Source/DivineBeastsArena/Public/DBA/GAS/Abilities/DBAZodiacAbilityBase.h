// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MobaBase/GAS/DBAMobaGameplayAbilityBase.h"
#include "Common/DBAEnumsCore.h"
#include "RPC/DBARpcServer.h"
#include "DBAZodiacAbilityBase.generated.h"

/**
 * 生肖技能基类 (Passive 等)
 * 决定英雄身份、外观剪影、动画基调
 */
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBAZodiacAbilityBase : public UDBAMobaGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UDBAZodiacAbilityBase();

	/** 所属生肖标识 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Zodiac")
	EDBAZodiacType ZodiacType;

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	/** 服务端激活回调 - RPC集成 */
	virtual void OnServerActivate_Implementation() override;
};
