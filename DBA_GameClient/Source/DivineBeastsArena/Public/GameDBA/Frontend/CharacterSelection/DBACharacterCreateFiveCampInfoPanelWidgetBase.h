// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateFiveCampViewModel.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBACharacterCreateFiveCampInfoPanelWidgetBase.generated.h"

class UDBACharacterCreateWidgetController;
class UDBACharacterCreateFiveCampViewModel;

/** WBP_DBA_FiveCampInfoPanel 的 C++ 父类；监听 ViewModel 并将当前卡片投影交给蓝图布局显示。 */
UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterCreateFiveCampInfoPanelWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|FiveCamp")
	void SetWidgetController(UDBACharacterCreateWidgetController* InController);

protected:
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleViewModelChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterCreate|FiveCamp")
	void BP_OnFiveCampInfoChanged(const FDBACharacterCreateFiveCampCardModel& SelectedCard, const FText& ValidationMessage);

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateWidgetController> Controller;

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateFiveCampViewModel> ViewModel;
};
