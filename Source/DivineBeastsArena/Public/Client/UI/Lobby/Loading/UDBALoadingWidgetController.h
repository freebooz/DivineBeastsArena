// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAWidgetController.h"
#include "DBALoadingWidgetController.generated.h"

UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBALoadingWidgetController : public UDBAWidgetController
{
	GENERATED_BODY()

public:
	UDBALoadingWidgetController(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Loading")
	void RequestLoadComplete();

	UFUNCTION(BlueprintPure, Category = "DBA|UI|Loading")
	float GetLoadingProgress() const;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadingProgressChanged, float, Progress);
	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|Loading")
	FOnLoadingProgressChanged OnLoadingProgressChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadingComplete);
	UPROPERTY(BlueprintAssignable, Category = "DBA|UI|Loading")
	FOnLoadingComplete OnLoadingComplete;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Loading")
	float LoadingProgress;
};
