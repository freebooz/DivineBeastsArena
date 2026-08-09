// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/UI/Framework/DBACommonModalBase.h"
#include "DBAGlobalLoadingWidgetBase.generated.h"

/** Blueprint base for WBP_DBA_GlobalLoading. Visibility is owned by request tokens in UDBAUILayerManagerSubsystem. */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAGlobalLoadingWidgetBase : public UDBACommonModalBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="DBA|UI|Loading")
	void SetLoadingMessage(const FText& InMessage);

	UPROPERTY(BlueprintReadOnly, Category="DBA|UI|Loading")
	FText LoadingMessage;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="DBA|UI|Loading", meta=(DisplayName="On Loading Message Changed"))
	void BP_OnLoadingMessageChanged(const FText& InMessage);
};
