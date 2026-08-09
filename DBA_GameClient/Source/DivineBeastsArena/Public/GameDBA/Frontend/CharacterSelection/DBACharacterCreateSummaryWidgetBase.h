// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBACharacterCreateSummaryWidgetBase.generated.h"

class UDBACharacterCreateWidgetController;
class UDBACharacterCreateConfirmViewModel;

/** WBP_DBA_CharacterCreateSummary 的 C++ 父类，只展示 Confirm ViewModel 的摘要投影。 */
UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterCreateSummaryWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Confirm")
	void SetWidgetController(UDBACharacterCreateWidgetController* InController);

protected:
	virtual void NativeDestruct() override;
	UFUNCTION()
	void HandleViewModelChanged();
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterCreate|Confirm")
	void BP_OnSummaryChanged(UDBACharacterCreateConfirmViewModel* InViewModel);

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateWidgetController> Controller;
	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateConfirmViewModel> ViewModel;
};
