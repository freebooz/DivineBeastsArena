// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "GameDBA/Spectator/DBAObserverTypes.h"
#include "DBASpectatorComponent.generated.h"

class UDBASpectatorManager;

/**
 * UDBASpectatorComponent
 * 瑙傛垬缁勪欢
 * 鎸傚湪瑙傛垬鑰匬awn鎴栬鎴樿€匔ontroller涓? * 璐熻矗绠＄悊瑙傛垬鑰呯殑瑙嗚鐘舵€佸拰杈撳叆澶勭悊
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
	/** 杩炴帴鍒拌鎴樻ā寮?*/
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	void JoinSpectatorMode(FString MatchID);

	/** 鏂紑瑙傛垬妯″紡 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	void LeaveSpectatorMode();

	/** 鍒囨崲鍒颁笅涓€涓帺瀹惰瑙?*/
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|View")
	void CycleNextTarget();

	/** 鍒囨崲鍒颁笂涓€涓帺瀹惰瑙?*/
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|View")
	void CyclePreviousTarget();

	/** 鍒囨崲鍒版寚瀹氱储寮曠帺瀹?*/
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|View")
	bool JumpToTarget(int32 TargetIndex);

	/** 鍒囨崲瑙嗚妯″紡 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|View")
	void SetViewMode(EDBAObserverViewMode NewViewMode);

	/** 鑾峰彇褰撳墠瑙嗚鐩爣 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|View")
	FDBAObserverViewTarget GetCurrentViewTarget() const;

	/** 鑾峰彇鎵€鏈夊彲鐢ㄨ瑙掔洰鏍?*/
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|View")
	TArray<FDBAObserverViewTarget> GetAllViewTargets() const;

	/** 鑾峰彇褰撳墠瑙傛垬绠＄悊鍣?*/
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator")
	UDBASpectatorManager* GetSpectatorManager() const;

protected:
	/** 杈撳叆: 鍒囨崲鍒颁笅涓€涓?*/
	void Input_CycleNext();

	/** 杈撳叆: 鍒囨崲鍒颁笂涓€涓?*/
	void Input_CyclePrevious();

	/** 杈撳叆: 鍒囨崲鍒拌嚜鐢辫瑙?*/
	void Input_ToggleFreeView();

	/** 杈撳叆: 鏁板瓧閿垏鎹?*/
	void Input_NumericSwitch(int32 Index);
	void Input_NumericSwitch1();
	void Input_NumericSwitch2();
	void Input_NumericSwitch3();
	void Input_NumericSwitch4();
	void Input_NumericSwitch5();
	void Input_NumericSwitch6();
	void Input_NumericSwitch7();
	void Input_NumericSwitch8();
	void Input_NumericSwitch9();

	/** 杈撳叆: 鏆傚仠/鎭㈠ */
	void Input_TogglePause();

private:
	/** 鑾峰彇鎵€灞濸layerController */
	APlayerController* GetOwningPlayerController() const;

public:
	/** 褰撳墠瑙嗚妯″紡 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|Spectator|View")
	EDBAObserverViewMode CurrentViewMode;

	/** 褰撳墠瑙嗚鐩爣 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|Spectator|View")
	FDBAObserverViewTarget CurrentViewTarget;

	/** 鏄惁宸茶繛鎺?*/
	UPROPERTY(BlueprintReadOnly, Category = "DBA|Spectator")
	bool bIsConnected;

protected:
	/** 杈撳叆缁勪欢寮曠敤 */
	UPROPERTY(Transient)
	TWeakObjectPtr<UInputComponent> CachedInputComponent;

	/** 瑙傛垬绠＄悊鍣ㄥ紩鐢?*/
	UPROPERTY(Transient)
	TWeakObjectPtr<UDBASpectatorManager> SpectatorManager;

	/** 瑙傛垬鑰呮帶鍒跺櫒寮曠敤 */
	UPROPERTY(Transient)
	TWeakObjectPtr<APlayerController> OwningPlayerController;
};

