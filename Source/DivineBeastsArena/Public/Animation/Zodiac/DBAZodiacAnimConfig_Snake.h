// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖动画配置 - 玄花灵蛇

#pragma once

#include "CoreMinimal.h"
#include "DBAZodiacAnimConfig_Snake.generated.h"
#include "Animation/DBAAnimConfig.h"

/**
 * UDBAZodiacAnimConfig_Snake
 * 玄花灵蛇动画配置
 * 定义玄花灵蛇角色特有的动画资源
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAZodiacAnimConfig_Snake : public UDataAsset
{
	GENERATED_BODY()

public:
	UDBAZodiacAnimConfig_Snake();

	/** 待机动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|玄花灵蛇")
	TSoftObjectPtr<UAnimMontage> Idle_Montage;

	/** 行走动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|玄花灵蛇")
	TSoftObjectPtr<UAnimMontage> Walk_Montage;

	/** 奔跑动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|玄花灵蛇")
	TSoftObjectPtr<UAnimMontage> Run_Montage;

	/** 普通攻击动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|玄花灵蛇")
	TSoftObjectPtr<UAnimMontage> Attack_Montage;

	/** 被动技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|玄花灵蛇")
	TSoftObjectPtr<UAnimMontage> Passive_Montage;

	/** Q技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|玄花灵蛇")
	TSoftObjectPtr<UAnimMontage> Q_Montage;

	/** W技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|玄花灵蛇")
	TSoftObjectPtr<UAnimMontage> W_Montage;

	/** E技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|玄花灵蛇")
	TSoftObjectPtr<UAnimMontage> E_Montage;

	/** R终极技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|玄花灵蛇")
	TSoftObjectPtr<UAnimMontage> R_Montage;

	/** 受击动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|玄花灵蛇")
	TSoftObjectPtr<UAnimMontage> Hit_Montage;

	/** 死亡动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|玄花灵蛇")
	TSoftObjectPtr<UAnimMontage> Death_Montage;
};
