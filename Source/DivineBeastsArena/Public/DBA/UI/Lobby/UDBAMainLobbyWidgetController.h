// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAWidgetController.h"
#include "Common/Account/DBAAccountTypes.h"
#include "Common/Party/DBAPartyTypes.h"
#include "UDBAMainLobbyWidgetController.generated.h"

UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAMainLobbyWidgetController : public UDBAWidgetController
{
	GENERATED_BODY()

public:
	UDBAMainLobbyWidgetController(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void RequestPartyInfo();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void RequestSwitchFiveCampTheme(uint8 FiveCamp);

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void RequestNavigateToNewbieVillage();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void RequestNavigateToPractice();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void RequestExitGame();

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyInfoReady, const FDBAPartyInfo&, PartyInfo);
	UPROPERTY(BlueprintAssignable, Category = "DBA|MainLobby")
	FOnPartyInfoReady OnPartyInfoReady;
};
