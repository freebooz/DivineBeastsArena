// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "UUDBAChainUltimatePanelWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAChainUltimatePanelWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAChainUltimatePanelWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ChainUltimatePanel")
	void UpdateChainCount(int32 Count);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ChainUltimatePanel")
	void ShowChainReady();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|ChainUltimatePanel", meta = (DisplayName = "On Chain Count Updated"))
	void BP_OnChainCountUpdated(int32 Count);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|ChainUltimatePanel", meta = (DisplayName = "On Chain Ready"))
	void BP_OnChainReady();
};
