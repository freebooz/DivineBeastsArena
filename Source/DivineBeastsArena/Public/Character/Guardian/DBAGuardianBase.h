// Copyright Freebooz Games, Inc. All Rights Reserved.
// 守卫模型基类

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DBAGuardianBase.generated.h"

class UDBAZodiacVFXComponent;

/**
 * DBAGuardianBase
 * 守卫模型基类
 * 提供守卫公共功能
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAGuardianBase : public AActor
{
	GENERATED_BODY()

public:
	ADBAGuardianBase();

protected:
	virtual void BeginPlay() override;

public:
	/** 获取VFX组件 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Guardian")
	UDBAZodiacVFXComponent* GetVFXComponent() const { return VFXComponent; }

	/** 播放攻击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Guardian")
	void PlayAttackVFX(AActor* Target);

	/** 播放受击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Guardian")
	void PlayHitVFX(AActor* Attacker);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|Components")
	TObjectPtr<UDBAZodiacVFXComponent> VFXComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	FName GuardianType = FName(TEXT("None"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	float MaxHealth = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	float AttackDamage = 25.0f;
};