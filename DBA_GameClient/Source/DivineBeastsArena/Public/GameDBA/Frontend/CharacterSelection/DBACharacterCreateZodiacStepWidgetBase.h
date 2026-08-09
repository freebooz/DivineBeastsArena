// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBACharacterCreateZodiacStepWidgetBase.generated.h"

class UPanelWidget;
class UDBACharacterCreateWidgetController;
class UDBACharacterCreateZodiacViewModel;
class UDBAZodiacItemWidgetBase;

/** WBP_DBA_CharacterCreate_ZodiacStep 的 C++ 父类，负责由 ViewModel 动态生成 Registry 驱动的生肖列表。 */
UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterCreateZodiacStepWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Zodiac")
	void SetWidgetController(UDBACharacterCreateWidgetController* InController);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Zodiac")
	void NextStep();

protected:
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleViewModelChanged();

	UFUNCTION()
	void HandleZodiacClicked(EDBAZodiac Zodiac);

	void RebuildZodiacItems();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate|Zodiac")
	TObjectPtr<UPanelWidget> ZodiacListContainer;

	/** 由 WBP_DBA_ZodiacItem 配置；未配置时不会创建占位 UI。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|CharacterCreate|Zodiac")
	TSubclassOf<UDBAZodiacItemWidgetBase> ZodiacItemWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateWidgetController> Controller;

	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateZodiacViewModel> ViewModel;
};
