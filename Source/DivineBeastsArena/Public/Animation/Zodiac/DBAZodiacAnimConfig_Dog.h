// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖动画配置 - 镇魄灵犬

#pragma once

#include "CoreMinimal.h"
#include "DBAZodiacAnimConfig_Dog.generated.h"
#include "DBA/Animation/DBAAnimConfig.h"

/**
 * UDBAZodiacAnimConfig_Dog
 * 镇魄灵犬动画配置
 * 定义镇魄灵犬角色特有的动画资源
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAZodiacAnimConfig_Dog : public UDataAsset
{
	GENERATED_BODY()

public:
	UDBAZodiacAnimConfig_Dog();

	/** 待机动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|镇魄灵犬")
	TSoftObjectPtr<UAnimMontage> Idle_Montage;

	/** 行走动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|镇魄灵犬")
	TSoftObjectPtr<UAnimMontage> Walk_Montage;

	/** 奔跑动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|镇魄灵犬")
	TSoftObjectPtr<UAnimMontage> Run_Montage;

	/** 普通攻击动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|镇魄灵犬")
	TSoftObjectPtr<UAnimMontage> Attack_Montage;

	/** 被动技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|镇魄灵犬")
	TSoftObjectPtr<UAnimMontage> Passive_Montage;

	/** Q技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|镇魄灵犬")
	TSoftObjectPtr<UAnimMontage> Q_Montage;

	/** W技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|镇魄灵犬")
	TSoftObjectPtr<UAnimMontage> W_Montage;

	/** E技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|镇魄灵犬")
	TSoftObjectPtr<UAnimMontage> E_Montage;

	/** R终极技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|镇魄灵犬")
	TSoftObjectPtr<UAnimMontage> R_Montage;

	/** 受击动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|镇魄灵犬")
	TSoftObjectPtr<UAnimMontage> Hit_Montage;

	/** 死亡动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|镇魄灵犬")
	TSoftObjectPtr<UAnimMontage> Death_Montage;
};
