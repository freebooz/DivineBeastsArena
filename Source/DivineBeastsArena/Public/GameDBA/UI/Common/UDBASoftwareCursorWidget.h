// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UDBASoftwareCursorWidget.generated.h"

class SImage;
class UTexture2D;

UCLASS()
class DIVINEBEASTSARENA_API UDBASoftwareCursorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;

protected:
	virtual void NativeConstruct() override;

private:
	UTexture2D* LoadCursorTexture();
	void ConfigureCursorBrush();

private:
	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> CursorTexture;

	FSlateBrush CursorBrush;
	TSharedPtr<SImage> CursorImage;
};
