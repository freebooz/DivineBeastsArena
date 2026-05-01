// Copyright Freebooz Games, Inc. All Rights Reserved.
// 动画配置

#pragma once

#include "CoreMinimal.h"
#include "DBAAnimConfig.generated.h"
#include "DBAAnimConfig.generated.h"

USTRUCT(BlueprintType)
struct FDBAAnimConfig
{{
	GENERATED_BODY()

	/** 待机动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> IdleMontage;

	/** 行走动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> WalkMontage;

	/** 奔跑动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> RunMontage;

	/** 攻击动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> AttackMontage;

	/** 技能动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> SkillMontage;

	/** 受击动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> HitMontage;

	/** 死亡动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> DeathMontage;
}};
