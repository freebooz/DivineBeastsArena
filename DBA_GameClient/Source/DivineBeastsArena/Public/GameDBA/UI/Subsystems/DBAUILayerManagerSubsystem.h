// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Core/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "GameDBA/Frontend/Core/DBAFrontendContracts.h"
#include "DBAUILayerManagerSubsystem.generated.h"

class UDBAUIRootLayout;
class UDBAGlobalLoadingWidgetBase;
class UDBASystemToastWidgetBase;
class UDBAErrorBannerWidgetBase;
class UDBANetworkStatusWidgetBase;
class UDBAFrontendFlowSubsystem;
class UDBAServerSelectScreenBase;
class UUserWidget;

UENUM(BlueprintType)
enum class EDBAUILayer : uint8
{
	Background,
	Screen,
	Modal,
	Toast,
	Tooltip,
	Debug
};

/**
 * Client-only UI lifetime owner. It is the sole class allowed to attach the root layout to the viewport.
 * Game UI managers mount widgets into a named child layer instead of assigning viewport Z orders.
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAUILayerManagerSubsystem : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	/** Dedicated Server 不创建任何 UMG/CommonUI 根布局管理对象。 */
	virtual bool IsSupportedInCurrentEnvironment() const override;
	virtual void OnSubsystemInitialize() override;
	virtual void OnSubsystemDeinitialize() override;

	UFUNCTION(BlueprintCallable, Category="DBA|UI|Layers")
	bool EnsureRootLayout();

	UFUNCTION(BlueprintPure, Category="DBA|UI|Layers")
	UDBAUIRootLayout* GetRootLayout() const { return RootLayout; }

	bool MountWidget(UUserWidget* Widget, EDBAUILayer Layer);
	bool RemoveWidget(UUserWidget* Widget);

	UFUNCTION(BlueprintCallable, Category="DBA|UI|Loading")
	FName BeginGlobalLoading(const FText& Message);

	UFUNCTION(BlueprintCallable, Category="DBA|UI|Loading")
	bool EndGlobalLoading(FName RequestToken);

	UFUNCTION(BlueprintPure, Category="DBA|UI|Loading")
	bool IsGlobalLoadingVisible() const { return ActiveLoadingRequestTokens.Num() > 0; }

	UFUNCTION(BlueprintCallable, Category="DBA|UI|Feedback")
	void ShowToast(const FText& Message);

	UFUNCTION(BlueprintCallable, Category="DBA|UI|Feedback")
	void ShowErrorBanner(const FText& Message);

	UFUNCTION(BlueprintCallable, Category="DBA|UI|Feedback")
	void SetNetworkAvailable(bool bAvailable, const FText& StatusText);

	/** Routes Escape, gamepad back, and Android back (mapped by CommonInput) to the highest modal. */
	UFUNCTION(BlueprintCallable, Category="DBA|UI|Input")
	bool HandleBackAction();

private:
	void BindFrontendFlow();
	bool ActivateServerSelectScreen();

	UFUNCTION()
	void HandleFrontendStateChanged(EDBAFrontendState PreviousState, EDBAFrontendState NewState);

	bool IsClientUIRuntime() const;
	UUserWidget* CreateWidgetForLocalPlayer(TSubclassOf<UUserWidget> WidgetClass) const;
	void ApplyUIInputMode(UUserWidget* FocusWidget) const;
	void RestoreGameInputMode() const;

	UPROPERTY(Transient)
	TObjectPtr<UDBAUIRootLayout> RootLayout;

	UPROPERTY(Transient)
	TObjectPtr<UDBAGlobalLoadingWidgetBase> GlobalLoadingWidget;

	UPROPERTY(Transient)
	TObjectPtr<UDBASystemToastWidgetBase> SystemToastWidget;

	UPROPERTY(Transient)
	TObjectPtr<UDBAErrorBannerWidgetBase> ErrorBannerWidget;

	UPROPERTY(Transient)
	TObjectPtr<UDBANetworkStatusWidgetBase> NetworkStatusWidget;

	UPROPERTY(Transient)
	TObjectPtr<UDBAServerSelectScreenBase> ServerSelectScreen;

	TWeakObjectPtr<UDBAFrontendFlowSubsystem> FrontendFlow;

	UPROPERTY(Transient)
	TSet<FName> ActiveLoadingRequestTokens;
};
