// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBANetworkStatusWidgetBase.generated.h"

/** Blueprint base for WBP_DBA_NetworkStatus. Network services update it through the UI layer manager only. */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBANetworkStatusWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="DBA|UI|Network")
	void SetNetworkStatus(bool bInAvailable, const FText& InStatusText);

	UPROPERTY(BlueprintReadOnly, Category="DBA|UI|Network")
	bool bIsNetworkAvailable = true;

	UPROPERTY(BlueprintReadOnly, Category="DBA|UI|Network")
	FText StatusText;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="DBA|UI|Network", meta=(DisplayName="On Network Status Changed"))
	void BP_OnNetworkStatusChanged(bool bInAvailable, const FText& InStatusText);
};
