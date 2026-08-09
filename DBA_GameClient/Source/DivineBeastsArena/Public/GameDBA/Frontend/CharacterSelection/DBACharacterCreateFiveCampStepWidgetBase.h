// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBACharacterCreateFiveCampStepWidgetBase.generated.h"

class UPanelWidget;
class UDBACharacterCreateWidgetController;
class UDBACharacterCreateFiveCampCardWidgetBase;
class UDBACharacterCreateFiveCampViewModel;

/**
 * WBP_DBA_CharacterCreate_FiveCampStep 的 C++ 父类。它动态创建数据表定义的卡片，
 * 并将 Next/Back 意图统一交给 Controller -> Flow；不直接 AddToViewport 或 OpenLevel。
 */
UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterCreateFiveCampStepWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|FiveCamp")
	void SetWidgetController(UDBACharacterCreateWidgetController* InController);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|FiveCamp")
	bool NextStep();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|FiveCamp")
	void BackStep();

protected:
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleViewModelChanged();

	UFUNCTION()
	void HandleFiveCampCardClicked(EDBAFiveCamp FiveCamp);

	void RebuildFiveCampCards();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate|FiveCamp")
	TObjectPtr<UPanelWidget> FiveCampCardContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|CharacterCreate|FiveCamp")
	TSubclassOf<UDBACharacterCreateFiveCampCardWidgetBase> FiveCampCardClass;

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateWidgetController> Controller;

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateFiveCampViewModel> ViewModel;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UDBACharacterCreateFiveCampCardWidgetBase>> SpawnedCards;
};
