// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖动画配置 - 曜鸣神鸡

#pragma once

#include "CoreMinimal.h"
#include "DBAZodiacAnimConfig_Rooster.generated.h"
#include "DBA/Animation/DBAAnimConfig.h"

/**
 * UDBAZodiacAnimConfig_Rooster
 * 曜鸣神鸡动画配置
 * 定义曜鸣神鸡角色特有的动画资源
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAZodiacAnimConfig_Rooster : public UDataAsset
{
	GENERATED_BODY()

public:
	UDBAZodiacAnimConfig_Rooster();

	/** 待机动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|曜鸣神鸡")
	TSoftObjectPtr<UAnimMontage> Idle_Montage;

	/** 行走动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|曜鸣神鸡")
	TSoftObjectPtr<UAnimMontage> Walk_Montage;

	/** 奔跑动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|曜鸣神鸡")
	TSoftObjectPtr<UAnimMontage> Run_Montage;

	/** 普通攻击动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|曜鸣神鸡")
	TSoftObjectPtr<UAnimMontage> Attack_Montage;

	/** 被动技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|曜鸣神鸡")
	TSoftObjectPtr<UAnimMontage> Passive_Montage;

	/** Q技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|曜鸣神鸡")
	TSoftObjectPtr<UAnimMontage> Q_Montage;

	/** W技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|曜鸣神鸡")
	TSoftObjectPtr<UAnimMontage> W_Montage;

	/** E技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|曜鸣神鸡")
	TSoftObjectPtr<UAnimMontage> E_Montage;

	/** R终极技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|曜鸣神鸡")
	TSoftObjectPtr<UAnimMontage> R_Montage;

	/** 受击动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|曜鸣神鸡")
	TSoftObjectPtr<UAnimMontage> Hit_Montage;

	/** 死亡动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|曜鸣神鸡")
	TSoftObjectPtr<UAnimMontage> Death_Montage;
};
