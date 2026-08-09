// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "DBACharacterPreviewControlsWidgetBase.generated.h"

class UDBACharacterCreateWidgetController;

/** WBP_DBA_CharacterPreviewControls 的 C++ 父类，统一将拖动/缩放/重置意图交给 PreviewSubsystem。 */
UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterPreviewControlsWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Preview")
	void SetWidgetController(UDBACharacterCreateWidgetController* InController) { Controller = InController; }

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Preview")
	void Rotate(float DeltaYawDegrees);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Preview")
	void Zoom(float DeltaDistance);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate|Preview")
	void ResetCamera();

protected:
	UPROPERTY(Transient)
	TObjectPtr<UDBACharacterCreateWidgetController> Controller;
};
