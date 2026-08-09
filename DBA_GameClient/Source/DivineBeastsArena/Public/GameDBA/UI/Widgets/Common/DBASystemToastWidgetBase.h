// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBASystemToastWidgetBase.generated.h"

/** Blueprint base for WBP_DBA_SystemToast. The subsystem serializes presentation ownership. */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBASystemToastWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="DBA|UI|Toast")
	void ShowMessage(const FText& InMessage);

	UPROPERTY(BlueprintReadOnly, Category="DBA|UI|Toast")
	FText Message;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="DBA|UI|Toast", meta=(DisplayName="On Toast Message Changed"))
	void BP_OnToastMessageChanged(const FText& InMessage);
};
