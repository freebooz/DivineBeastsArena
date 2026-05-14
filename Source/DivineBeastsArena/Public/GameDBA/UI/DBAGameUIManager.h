// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameCore/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "GameDBA/UI/Lobby/UDBAMainLobbyWidgetBase.h"
#include "GameDBA/UI/Lobby/Login/UDBALoginFlowWidgetBase.h"
#include "GameDBA/UI/Lobby/Login/UDBACharacterSelectFlowWidgetBase.h"
#include "GameDBA/UI/Lobby/Login/UDBACharacterCreateFlowWidgetBase.h"
#include "DBAGameUIManager.generated.h"

class UDBAArenaHUDRootWidgetBase;
class UDBAMainLobbyWidgetBase;
class UAudioComponent;
class USoundBase;

/**
 * EDBAUIState
 * UI状态枚举
 */
UENUM(BlueprintType)
enum class EDBAUIState : uint8
{
	None,
	MainMenu,
	Lobby,
	HeroSelect,
	Loading,
	InGame,
	Pause
};

/**
 * FOnUIStateChanged
 * UI状态改变委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIStateChanged, EDBAUIState, NewState);

/**
 * DBAGameUIManager
 *
 * 游戏UI状态管理器
 * 统一管理所有游戏界面的显示/隐藏和切换
 */
UCLASS(Blueprintable)
class DIVINEBEASTSARENA_API UDBAGameUIManager : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	UDBAGameUIManager();

protected:
	virtual void OnSubsystemInitialize() override;
	virtual void OnSubsystemDeinitialize() override;

public:
	/** 获取当前状态 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	EDBAUIState GetCurrentState() const { return CurrentState; }

	/** 切换UI状态 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void TransitionTo(EDBAUIState NewState);

	/** 注册状态改变回调 (C++内部使用) */
	void RegisterStateChangeCallback(const FOnUIStateChanged& Delegate);

	/** 显示主大厅 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void ShowMainLobby();

	/** 隐藏主大厅 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void HideMainLobby();

	/** 显示战斗HUD */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void ShowArenaHUD();

	/** 隐藏战斗HUD */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void HideArenaHUD();

	/** 清理所有UI */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void ClearAllUI();

protected:
	/** 创建Lobby Widget */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreateMainLobbyWidget();

	/** 创建Arena HUD Widget */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreateArenaHUDWidget();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void ShowLoginFlowWidget();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void HideLoginFlowWidget();

protected:
	/** 当前UI状态 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	EDBAUIState CurrentState = EDBAUIState::None;

	/** 状态改变回调 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	FOnUIStateChanged OnStateChanged;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBAMainLobbyWidgetBase> MainLobbyWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBAArenaHUDRootWidgetBase> ArenaHUDWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBALoginFlowWidgetBase> LoginWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBACharacterSelectFlowWidgetBase> CharacterSelectWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBACharacterCreateFlowWidgetBase> CharacterCreateWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBAMainLobbyWidgetBase> MainLobbyWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBAArenaHUDRootWidgetBase> ArenaHUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBALoginFlowWidgetBase> LoginWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBACharacterSelectFlowWidgetBase> CharacterSelectWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBACharacterCreateFlowWidgetBase> CharacterCreateWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<class UDBASplashVideoWidget> SplashVideoWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<class UDBASplashVideoWidget> SplashVideoWidgetClass;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void ShowSplashVideo();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void HideSplashVideo();

private:
	UPROPERTY(Transient)
	EDBALoginFlowState CachedLoginFlowState = EDBALoginFlowState::Startup;

	UFUNCTION()
	void HandleLoginFlowStateChanged(EDBALoginFlowState NewState);

	void RefreshLoginFlowWidgetVisibility();
	void HideAllFlowWidgets();
	void EnsureLoginFlowBackgroundMusic();
	void StopLoginFlowBackgroundMusic();

	UFUNCTION()
	void HandleLoginFlowBackgroundMusicFinished();

	template<typename WidgetType>
	WidgetType* EnsureFlowWidgetCreated(TSubclassOf<WidgetType> WidgetClass, TObjectPtr<WidgetType>& WidgetInstance);
	void SetFlowWidgetVisible(UUserWidget* WidgetToShow);

	void TryShowSplashVideo();
	void EnsureLoginFlowStartedFromManager();

	bool bMainLobbyVisible = false;
	bool bArenaHUDVisible = false;
	bool bFlowWidgetVisible = false;
	bool bLoginFlowStartRequested = false;

	FTimerHandle SplashVideoTimerHandle;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> LoginFlowBackgroundMusicComponent;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|UI|Audio")
	TObjectPtr<USoundBase> LoginFlowBackgroundMusicSound;
};
