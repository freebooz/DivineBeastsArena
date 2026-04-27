// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "Common/DBAEnumsCore.h"
#include "DBAMainLobbyWidgetBase.generated.h"

class UDBAMainLobbyWidgetController;
class UDBAPartyPanelWidgetBase;
class UDBAQueueModeSelectWidgetBase;

/**
 * DBAMainLobbyWidgetBase
 *
 * 主大厅界面 Widget 基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAMainLobbyWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAMainLobbyWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void SetWidgetController(UDBAMainLobbyWidgetController* InController);

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void RefreshPartyInfo();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void SwitchFiveCampTheme(EDBAFiveCamp FiveCamp);

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void NavigateToNewbieVillage();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void NavigateToPractice();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void OpenQueueModeSelect();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void OpenFriendList();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void OpenSettings();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void ExitGame();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|MainLobby", meta = (DisplayName = "On Party Info Refreshed"))
	void BP_OnPartyInfoRefreshed();

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|MainLobby", meta = (DisplayName = "On FiveCamp Theme Switched"))
	void BP_OnFiveCampThemeSwitched(EDBAFiveCamp NewTheme);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby")
	TObjectPtr<UDBAMainLobbyWidgetController> WidgetController;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidget))
	TObjectPtr<UDBAPartyPanelWidgetBase> PartyPanel;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidget))
	TObjectPtr<UDBAQueueModeSelectWidgetBase> QueueModeSelect;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby")
	EDBAFiveCamp CurrentFiveCampTheme;
};
