// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterCreateElementViewModel.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBACharacterCreateElementCardWidgetBase.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnCharacterCreateElementCardClicked, EDBAElement, Element);

/** WBP_DBA_ElementCard 的轻量父类，仅展示 Controller 投影并发出选择意图。 */
UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterCreateElementCardWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	void ApplyCard(const FDBACharacterCreateElementCardModel& InCard);

	UPROPERTY(BlueprintAssignable, Category = "DBA|CharacterCreate|Element")
	FDBAOnCharacterCreateElementCardClicked OnElementClicked;

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void HandleClicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterCreate|Element")
	void BP_OnCardApplied(const FDBACharacterCreateElementCardModel& InCard);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate|Element")
	TObjectPtr<UButton> SelectButton;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate|Element")
	FDBACharacterCreateElementCardModel Card;
};
