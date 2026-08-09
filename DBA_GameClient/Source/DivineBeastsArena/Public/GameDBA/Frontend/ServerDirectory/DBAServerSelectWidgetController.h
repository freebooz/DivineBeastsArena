// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Frontend/Core/DBAFrontendContracts.h"
#include "GameMoba/UI/DBAMobaHUDWidgetControllerBase.h"
#include "DBAServerSelectWidgetController.generated.h"

class UDBAFrontendFlowSubsystem;
class UDBAServerDirectorySubsystem;
class UDBAServerSelectViewModel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAServerSelectUIError, const FDBAApiError&, Error);

/** 将选服 UI 意图转发给 Flow 与目录 Subsystem；不创建页面、不发送 HTTP。 */
UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBAServerSelectWidgetController : public UDBAMobaHUDWidgetControllerBase
{
	GENERATED_BODY()

public:
	UDBAServerSelectWidgetController(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "DBA|ServerSelect")
	void Start();

	UFUNCTION(BlueprintCallable, Category = "DBA|ServerSelect")
	void Refresh();

	UFUNCTION(BlueprintCallable, Category = "DBA|ServerSelect")
	void Retry();

	UFUNCTION(BlueprintCallable, Category = "DBA|ServerSelect")
	void SelectServer(const FString& ServerId);

	UFUNCTION(BlueprintCallable, Category = "DBA|ServerSelect")
	void ConfirmSelection();

	UFUNCTION(BlueprintCallable, Category = "DBA|ServerSelect")
	void RequestBackToLogin();

	UFUNCTION(BlueprintPure, Category = "DBA|ServerSelect")
	UDBAServerSelectViewModel* GetViewModel() const { return ViewModel; }

	UPROPERTY(BlueprintAssignable, Category = "DBA|ServerSelect")
	FDBAServerSelectUIError OnServerDirectoryError;

protected:
	UFUNCTION()
	void HandleDirectoryChanged(bool bSuccess, const FDBAApiError& Error);

	UFUNCTION()
	void HandleFlowStateChanged(EDBAFrontendState PreviousState, EDBAFrontendState NewState);

	void DeinitializeController();
	UDBAServerDirectorySubsystem* GetServerDirectory() const;
	UDBAFrontendFlowSubsystem* GetFlow() const;

	UPROPERTY(Transient)
	TObjectPtr<UDBAServerSelectViewModel> ViewModel;

	TWeakObjectPtr<UDBAServerDirectorySubsystem> ServerDirectory;
	TWeakObjectPtr<UDBAFrontendFlowSubsystem> FrontendFlow;
};
