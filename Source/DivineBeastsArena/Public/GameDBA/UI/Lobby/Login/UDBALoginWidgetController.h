// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameMoba/UI/DBAMobaHUDWidgetControllerBase.h"
#include "UDBALoginWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBALoginUIError, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBALoginUIStateChanged, EDBALoginFlowState, State);

UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBALoginWidgetController : public UDBAMobaHUDWidgetControllerBase
{
	GENERATED_BODY()

public:
	UDBALoginWidgetController(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	void Start();

	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	void LoginWithEmail(const FString& Email, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	void LoginAsGuest();

	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	void LoginDebug(const FString& DisplayName = TEXT("frontend_debug"));

	UPROPERTY(BlueprintAssignable, Category = "DBA|Login")
	FDBALoginUIError OnLoginError;

	UPROPERTY(BlueprintAssignable, Category = "DBA|Login")
	FDBALoginUIStateChanged OnLoginStateChanged;

protected:
	UFUNCTION()
	void HandleFlowError(const FString& ErrorMessage);

	UFUNCTION()
	void HandleFlowStateChanged(EDBALoginFlowState State);

	UDBALoginFlowSubsystem* GetLoginFlow() const;
};
