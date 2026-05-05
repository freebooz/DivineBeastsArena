// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "GameDBA/Spectator/DBAObserverTypes.h"
#include "DBASpectatorComponent.generated.h"

class UDBASpectatorManager;

/**
 * UDBASpectatorComponent
 * 观战组件
 * 挂在观战者Pawn或观战者Controller上
 * 负责管理观战者的视角状态和输入处理
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "DBA Spectator Component"))
class DIVINEBEASTSARENA_API UDBASpectatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDBASpectatorComponent();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent(UInputComponent* InputComponent);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/** 连接到观战模式 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	void JoinSpectatorMode(FString MatchID);

	/** 断开观战模式 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	void LeaveSpectatorMode();

	/** 切换到下一个玩家视角 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|View")
	void CycleNextTarget();

	/** 切换到上一个玩家视角 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|View")
	void CyclePreviousTarget();

	/** 切换到指定索引玩家 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|View")
	bool JumpToTarget(int32 TargetIndex);

	/** 切换视角模式 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|View")
	void SetViewMode(EDBAObserverViewMode NewViewMode);

	/** 获取当前视角目标 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|View")
	FDBAObserverViewTarget GetCurrentViewTarget() const;

	/** 获取所有可用视角目标 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|View")
	TArray<FDBAObserverViewTarget> GetAllViewTargets() const;

	/** 获取当前观战管理器 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	UDBASpectatorManager* GetSpectatorManager() const;

protected:
	/** 输入: 切换到下一个 */
	void Input_CycleNext();

	/** 输入: 切换到上一个 */
	void Input_CyclePrevious();

	/** 输入: 切换到自由视角 */
	void Input_ToggleFreeView();

	/** 输入: 数字键切换 */
	void Input_NumericSwitch(int32 Index);

	/** 输入: 暂停/恢复 */
	void Input_TogglePause();

private:
	/** 获取所属PlayerController */
	APlayerController* GetOwningPlayerController() const;

public:
	/** 当前视角模式 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|Spectator|View")
	EDBAObserverViewMode CurrentViewMode;

	/** 当前视角目标 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|Spectator|View")
	FDBAObserverViewTarget CurrentViewTarget;

	/** 是否已连接 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|Spectator")
	bool bIsConnected;

protected:
	/** 输入组件引用 */
	UPROPERTY()
	TWeakObjectPtr<UInputComponent> CachedInputComponent;

	/** 观战管理器引用 */
	UPROPERTY()
	TWeakObjectPtr<UDBASpectatorManager> SpectatorManager;

	/** 观战者控制器引用 */
	UPROPERTY()
	TWeakObjectPtr<APlayerController> OwningPlayerController;
};
