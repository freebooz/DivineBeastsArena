// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Account/DBAAccountTypes.h"
#include "GameMoba/UI/DBAMobaHUDWidgetControllerBase.h"
#include "UDBACharacterSelectWidgetController.generated.h"

class UDBALoginFlowSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBACharactersChanged, const TArray<FDBACharacterSummary>&, Characters);

UCLASS(BlueprintType, Blueprintable)
class DIVINEBEASTSARENA_API UDBACharacterSelectWidgetController : public UDBAMobaHUDWidgetControllerBase
{
	GENERATED_BODY()

public:
	UDBACharacterSelectWidgetController(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	void BindLoginFlow();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	void SelectCharacter(const FDBACharacterId& CharacterId);

	UPROPERTY(BlueprintAssignable, Category = "DBA|CharacterSelect")
	FDBACharactersChanged OnCharactersChanged;

protected:
	UFUNCTION()
	void HandleCharactersLoaded(const TArray<FDBACharacterSummary>& Characters);

	UDBALoginFlowSubsystem* GetLoginFlow() const;
};
