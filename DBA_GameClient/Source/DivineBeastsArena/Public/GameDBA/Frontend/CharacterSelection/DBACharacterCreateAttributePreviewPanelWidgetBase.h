// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBACharacterCreateAttributePreviewPanelWidgetBase.generated.h"

class UDBACharacterCreateWidgetController;
class UDBACharacterCreateElementViewModel;

/** WBP_DBA_AttributePreviewPanel 的父类，显示固定构筑给出的非权威属性摘要。 */
UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterCreateAttributePreviewPanelWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|AttributePreview")
	void SetWidgetController(UDBACharacterCreateWidgetController* InController);

protected:
	virtual void NativeDestruct() override;
	UFUNCTION()
	void HandleViewModelChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterCreate|AttributePreview")
	void BP_OnAttributePreviewChanged(const FDBACharacterCreateAttributePreviewModel& Preview);

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateWidgetController> Controller;
	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateElementViewModel> ViewModel;
};
