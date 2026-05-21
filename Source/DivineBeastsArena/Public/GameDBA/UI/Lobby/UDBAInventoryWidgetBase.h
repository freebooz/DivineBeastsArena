// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "UDBAInventoryWidgetBase.generated.h"

class UButton;
class UBorder;
class UDBAMainLobbyWidgetController;
class UImage;
class UTextBlock;
class UTexture2D;

USTRUCT(BlueprintType)
struct FDBAInventoryItemView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Inventory")
	FString ItemId;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Inventory")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Inventory")
	FString Category;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Inventory")
	FString Rarity;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Inventory")
	int32 Count = 1;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Inventory")
	bool bUsable = false;
};

UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAInventoryWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAInventoryWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|Inventory")
	void RefreshInventory();

	UFUNCTION(BlueprintCallable, Category = "DBA|Inventory")
	void SetInventoryDataFromJson(const FString& InventoryJson);

	UFUNCTION(BlueprintCallable, Category = "DBA|Inventory")
	void SortInventory();

	UFUNCTION(BlueprintCallable, Category = "DBA|Inventory")
	void SelectItemByIndex(int32 ItemIndex);

	UFUNCTION(BlueprintCallable, Category = "DBA|Inventory")
	void SelectNextItem();

	UFUNCTION(BlueprintCallable, Category = "DBA|Inventory")
	void SelectPreviousItem();

	UFUNCTION(BlueprintCallable, Category = "DBA|Inventory")
	void UseSelectedItem();

	UFUNCTION(BlueprintCallable, Category = "DBA|Inventory")
	void CloseInventory();

	UFUNCTION(BlueprintPure, Category = "DBA|Inventory")
	const TArray<FDBAInventoryItemView>& GetItems() const { return Items; }

	UFUNCTION(BlueprintPure, Category = "DBA|Inventory")
	int32 GetSelectedIndex() const { return SelectedIndex; }

protected:
	void EnsureNativeFallbackLayout();
	void ResolveBoundWidgetsFromWidgetTree();
	void ApplyPanelBackgroundTexture();
	void BindControls();
	void UnbindControls();
	void EnsureController();
	void RefreshItemText();
	void RefreshDetailsText();
	void SetStatus(const FText& Text);

	UFUNCTION()
	void HandleInventoryUpdated(const FString& DataJson);

	UFUNCTION()
	void HandleBackendError(const FString& ErrorMessage);

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleRefreshClicked();

	UFUNCTION()
	void HandleSortClicked();

	UFUNCTION()
	void HandleUseClicked();

	UFUNCTION()
	void HandleNextClicked();

	UFUNCTION()
	void HandlePreviousClicked();

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Inventory")
	void BP_OnInventoryUpdated(const TArray<FDBAInventoryItemView>& NewItems, int32 NewSelectedIndex);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Inventory")
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Inventory")
	TObjectPtr<UButton> RefreshButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Inventory")
	TObjectPtr<UButton> SortButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Inventory")
	TObjectPtr<UButton> UseButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Inventory")
	TObjectPtr<UButton> NextButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Inventory")
	TObjectPtr<UButton> PreviousButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Inventory")
	TObjectPtr<UTextBlock> ItemsText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Inventory")
	TObjectPtr<UTextBlock> DetailsText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Inventory")
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Inventory|Visual")
	TObjectPtr<UBorder> PanelBackgroundBorder;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Inventory|Visual")
	TObjectPtr<UImage> PanelBackgroundImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Inventory|Visual")
	TSoftObjectPtr<UTexture2D> PanelBackgroundTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Inventory|Visual")
	FVector2D NativePanelSize = FVector2D(720.0f, 480.0f);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DBA|Inventory")
	TArray<FDBAInventoryItemView> Items;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DBA|Inventory")
	int32 SelectedIndex = INDEX_NONE;

	UPROPERTY(Transient)
	TObjectPtr<UDBAMainLobbyWidgetController> InventoryController;
};
