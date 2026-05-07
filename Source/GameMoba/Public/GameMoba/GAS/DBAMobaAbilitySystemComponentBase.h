// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameMoba - 通用MOBA AbilitySystemComponent基类

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "DBAMobaAbilitySystemComponentBase.generated.h"

/**
 * UDBAMobaAbilitySystemComponentBase
 * MOBA游戏通用AbilitySystemComponent基类
 * 提供GAS通用接口
 */
UCLASS(ClassGroup=(GameMoba), meta=(BlueprintSpawnableComponent))
class GAMEMOBA_API UDBAMobaAbilitySystemComponentBase : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UDBAMobaAbilitySystemComponentBase(const FObjectInitializer& ObjectInitializer);

protected:
	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	//~ End UActorComponent Interface

public:
	/** 初始化技能系统 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ability|Base")
	virtual void InitializeAbilities(AActor* InOwnerActor, AActor* InAvatarActor);

	/** 清理所有已授予的技能 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Ability|Base")
	void RemoveAllGrantedAbilities();

protected:
	/** 技能初始化完成 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|Ability|Base")
	bool bAbilitiesInitialized = false;

	/** 已授予的技能Handle列表 */
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;
};