// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameMoba/UI/UDBAMobaWidgetControllerBase.h"
#include "UDBALoginWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBALoginUIError, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBALoginUIStateChanged, EDBALoginFlowState, State);

UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBALoginWidgetController : public UDBAMobaWidgetControllerBase
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
