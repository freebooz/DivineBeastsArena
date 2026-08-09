// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/UI/Framework/DBACommonModalBase.h"
#include "DBAModalDialogWidgetBase.generated.h"

/** Blueprint base for WBP_DBA_ModalDialog. The controller supplies localized title, body, and actions. */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAModalDialogWidgetBase : public UDBACommonModalBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="DBA|UI|Modal")
	void SetDialogContent(const FText& InTitle, const FText& InBody);

	UPROPERTY(BlueprintReadOnly, Category="DBA|UI|Modal")
	FText Title;

	UPROPERTY(BlueprintReadOnly, Category="DBA|UI|Modal")
	FText Body;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="DBA|UI|Modal", meta=(DisplayName="On Dialog Content Changed"))
	void BP_OnDialogContentChanged(const FText& InTitle, const FText& InBody);
};
