// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DBAStartupViewModel.generated.h"

UENUM(BlueprintType)
enum class EDBAStartupServiceState : uint8
{
	Checking,
	Online,
	OfflineRecoverable,
	FatalConfiguration
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDBAOnStartupViewModelChanged);

/** Read-only presentation state for the startup screen. It never stores credentials or performs network work. */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAStartupViewModel : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "DBA|Startup")
	FDBAOnStartupViewModelChanged OnChanged;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Startup")
	FText Title;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Startup")
	FText VersionText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Startup")
	FText ServiceStatusText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Startup")
	EDBAStartupServiceState ServiceState = EDBAStartupServiceState::Checking;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Startup")
	bool bCanContinue = false;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Startup")
	bool bCanOpenSettings = true;

	void SetPresentation(const FText& InTitle, const FText& InVersionText);
	void SetServiceStatus(EDBAStartupServiceState InState, const FText& InStatusText, bool bInCanContinue);
};
