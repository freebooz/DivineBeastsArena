// Copyright Freebooz Games, Inc. All Rights Reserved.
// 怪物模型基类

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DBAMonsterBase.generated.h"

/**
 * DBAMonsterBase
 * 怪物模型基类
 * 提供怪物公共功能
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAMonsterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ADBAMonsterBase();

protected:
	virtual void BeginPlay() override;

public:
	/** 播放受击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Monster")
	void PlayHitVFX(AActor* Attacker);

	/** 播放死亡特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Monster")
	void PlayDeathVFX();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	FName MonsterType = FName(TEXT("None"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	float MaxHealth = 100.0f;
};