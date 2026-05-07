// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameMoba - 通用MOBA GameplayAbility基类

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "DBAMobaGameplayAbilityBase.generated.h"

/**
 * UDBAMobaGameplayAbilityBase
 * MOBA游戏通用GameplayAbility基类
 * 提供技能通用接口
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class GAMEMOBA_API UDBAMobaGameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UDBAMobaGameplayAbilityBase(const FObjectInitializer& ObjectInitializer);

protected:
	//~ Begin UGameplayAbility Interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~ End UGameplayAbility Interface

public:
	/** 获取技能图标 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ability|Base")
	UTexture2D* GetAbilityIcon() const { return AbilityIcon; }

	/** 获取技能名称 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ability|Base")
	FText GetAbilityName() const { return AbilityName; }

	/** 获取冷却时间 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ability|Base")
	float GetCooldownDuration() const { return CooldownDuration; }

protected:
	/** 技能图标 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Ability|Base")
	TObjectPtr<UTexture2D> AbilityIcon;

	/** 技能显示名称 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Ability|Base")
	FText AbilityName;

	/** 冷却时间(秒) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Ability|Base")
	float CooldownDuration = 0.0f;

	/** 能量消耗 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Ability|Base")
	float EnergyCost = 0.0f;

	/** 是否是终极技能 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Ability|Base")
	bool bIsUltimate = false;

	/** 技能所属元素 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Ability|Base")
	FName ElementType = NAME_None;
};