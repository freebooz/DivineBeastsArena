// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Account/DBAAccountTypes.h"
#include "GameMoba/UI/DBAMobaHUDWidgetControllerBase.h"
#include "UDBACharacterCreateWidgetController.generated.h"

class UDBALoginFlowSubsystem;

UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterCreateWidgetController : public UDBAMobaHUDWidgetControllerBase
{
	GENERATED_BODY()

public:
	UDBACharacterCreateWidgetController(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void SetCharacterName(const FString& InName);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void SetZodiac(EDBAZodiac InZodiac);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void SetElement(EDBAElement InElement);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void SetFiveCamp(EDBAFiveCamp InFiveCamp);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void Submit();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	FDBACharacterCreateRequest PendingRequest;

	UDBALoginFlowSubsystem* GetLoginFlow() const;
};
