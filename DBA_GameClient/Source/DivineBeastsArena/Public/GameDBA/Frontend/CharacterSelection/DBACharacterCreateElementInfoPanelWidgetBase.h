// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBACharacterCreateElementInfoPanelWidgetBase.generated.h"

class UDBACharacterCreateWidgetController;
class UDBACharacterCreateElementViewModel;

/** WBP_DBA_ElementInfoPanel 的父类；展示所选元素的规则配置说明。 */
UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterCreateElementInfoPanelWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Element")
	void SetWidgetController(UDBACharacterCreateWidgetController* InController);

protected:
	virtual void NativeDestruct() override;
	UFUNCTION()
	void HandleViewModelChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterCreate|Element")
	void BP_OnElementInfoChanged(EDBAElement Element, const FText& Name, const FText& Description, bool bIsAvailable);

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateWidgetController> Controller;
	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateElementViewModel> ViewModel;
};
