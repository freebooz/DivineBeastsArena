// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "Components/ActorComponent.h"
#include "GameDBA/Spectator/DBAObserverTypes.h"
#include "DBASpectatorComponent.generated.h"

class UDBASpectatorManager;
class UInputMappingContext;
struct FInputActionValue;

/**
 * UDBASpectatorComponent
 * 观战组件
 * 挂在观战者 Pawn 或战斗 Controller 上
 * 负责管理观战者的视角状态和输入处理
 *
 * P1-4 改造：输入绑定从旧版 BindAction/BindKey 迁移到 Enhanced Input 系统。
 * UInputAction 和 UInputMappingContext 通过 DBASpectatorInputConfigDataAsset 数据资产配置，
 * 避免硬编码（符合 DBA.DataAsset.NoHardcoding 策略）。
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
	/** 连接到战斗模式 */
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
	/**
	 * 输入: 切换到下一个观战目标
	 * P1-4 改造：回调签名增加 FInputActionValue 参数以兼容 Enhanced Input。
	 */
	void Input_CycleNext(const FInputActionValue& Value);

	/**
	 * 输入: 切换到上一个观战目标
	 * P1-4 改造：回调签名增加 FInputActionValue 参数以兼容 Enhanced Input。
	 */
	void Input_CyclePrevious(const FInputActionValue& Value);

	/**
	 * 输入: 切换自由视角/跟随视角
	 * P1-4 改造：回调签名增加 FInputActionValue 参数以兼容 Enhanced Input。
	 */
	void Input_ToggleFreeView(const FInputActionValue& Value);

	/** 输入: 数字键切换 */
	void Input_NumericSwitch(int32 Index);
	void Input_NumericSwitch1(const FInputActionValue& Value);
	void Input_NumericSwitch2(const FInputActionValue& Value);
	void Input_NumericSwitch3(const FInputActionValue& Value);
	void Input_NumericSwitch4(const FInputActionValue& Value);
	void Input_NumericSwitch5(const FInputActionValue& Value);
	void Input_NumericSwitch6(const FInputActionValue& Value);
	void Input_NumericSwitch7(const FInputActionValue& Value);
	void Input_NumericSwitch8(const FInputActionValue& Value);
	void Input_NumericSwitch9(const FInputActionValue& Value);

	/**
	 * 输入: 暂停/恢复观战
	 * P1-4 改造：回调签名增加 FInputActionValue 参数以兼容 Enhanced Input。
	 */
	void Input_TogglePause(const FInputActionValue& Value);

private:
	/** 获取所属 PlayerController */
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
	UPROPERTY(Transient)
	TWeakObjectPtr<UInputComponent> CachedInputComponent;

	/** 观战管理器引用 */
	UPROPERTY(Transient)
	TWeakObjectPtr<UDBASpectatorManager> SpectatorManager;

	/** 观战控制器引用 */
	UPROPERTY(Transient)
	TWeakObjectPtr<APlayerController> OwningPlayerController;
};
