// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBACharacterCreateElementStepWidgetBase.generated.h"

class UPanelWidget;
class UDBACharacterCreateWidgetController;
class UDBACharacterCreateElementViewModel;
class UDBACharacterCreateElementCardWidgetBase;

/** WBP_DBA_CharacterCreate_ElementStep 的 C++ 父类，负责动态卡片列表和 Flow 意图转发。 */
UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterCreateElementStepWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Element")
	void SetWidgetController(UDBACharacterCreateWidgetController* InController);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Element")
	bool NextStep();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Element")
	void BackStep();

protected:
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleViewModelChanged();

	UFUNCTION()
	void HandleElementCardClicked(EDBAElement Element);

	void RebuildElementCards();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate|Element")
	TObjectPtr<UPanelWidget> ElementCardContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|CharacterCreate|Element")
	TSubclassOf<UDBACharacterCreateElementCardWidgetBase> ElementCardClass;

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateWidgetController> Controller;

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateElementViewModel> ViewModel;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UDBACharacterCreateElementCardWidgetBase>> SpawnedCards;
};
