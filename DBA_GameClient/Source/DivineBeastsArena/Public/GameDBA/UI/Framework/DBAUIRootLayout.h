// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DBAUIRootLayout.generated.h"

class UOverlay;

/** Root layout used by the client UI layer manager. Each named overlay owns one UI lifetime category. */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAUIRootLayout : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	bool MountBackground(UUserWidget* Widget);
	bool MountScreen(UUserWidget* Widget);
	bool MountModal(UUserWidget* Widget);
	bool MountToast(UUserWidget* Widget);
	bool MountTooltip(UUserWidget* Widget);
	bool MountDebug(UUserWidget* Widget);
	bool RemoveManagedWidget(UUserWidget* Widget);
	UUserWidget* GetTopModal() const;

	UOverlay* GetBackgroundLayer() const { return BackgroundLayer; }
	UOverlay* GetScreenLayer() const { return ScreenLayer; }
	UOverlay* GetModalLayer() const { return ModalLayer; }
	UOverlay* GetToastLayer() const { return ToastLayer; }
	UOverlay* GetTooltipLayer() const { return TooltipLayer; }
	UOverlay* GetDebugLayer() const { return DebugLayer; }

private:
	void EnsureLayerTree();
	bool MountSingle(UOverlay* Layer, UUserWidget* Widget);
	bool MountStacked(UOverlay* Layer, UUserWidget* Widget);

	UPROPERTY(Transient)
	TObjectPtr<UOverlay> RootOverlay;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UOverlay> BackgroundLayer;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UOverlay> ScreenLayer;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UOverlay> ModalLayer;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UOverlay> ToastLayer;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UOverlay> TooltipLayer;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	TObjectPtr<UOverlay> DebugLayer;
};
