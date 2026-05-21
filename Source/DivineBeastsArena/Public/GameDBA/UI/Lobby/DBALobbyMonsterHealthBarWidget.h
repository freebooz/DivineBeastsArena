// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DBALobbyMonsterHealthBarWidget.generated.h"

class UProgressBar;

UCLASS()
class DIVINEBEASTSARENA_API UDBALobbyMonsterHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealthPercent(float Percent);
	void SetSelected(bool bSelected);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	FLinearColor ResolveFillColor() const;

	TSharedPtr<class SProgressBar> HealthProgressBar;
	float CachedPercent = 1.0f;
	bool bCachedSelected = false;
};
