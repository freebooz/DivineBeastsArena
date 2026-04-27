// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "DBAMobaGameplayCuePolicy.generated.h"

/**
 * GameplayCue 策略接口
 * 用于控制 GameplayCue 在不同环境下的行为
 *
 * Dedicated Server 不创建表现资源，但保留事件桥接接口
 * 客户端根据策略决定是否播放 Cue
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAMobaGameplayCuePolicy : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 判断是否应该执行 GameplayCue
	 * @param CueTag GameplayCue Tag
	 * @return 是否应该执行
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GameplayCue Policy")
	bool ShouldExecuteGameplayCue(FGameplayTag CueTag) const;
	virtual bool ShouldExecuteGameplayCue_Implementation(FGameplayTag CueTag) const;

	/**
	 * 判断是否为 Dedicated Server
	 * Dedicated Server 不创建表现资源
	 */
	UFUNCTION(BlueprintCallable, Category = "GameplayCue Policy")
	bool IsDedicatedServer() const;

	/**
	 * 判断是否为本地玩家
	 * 用于优化非本地玩家的 Cue 播放
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GameplayCue Policy")
	bool IsLocalPlayer() const;
	virtual bool IsLocalPlayer_Implementation() const;
};
