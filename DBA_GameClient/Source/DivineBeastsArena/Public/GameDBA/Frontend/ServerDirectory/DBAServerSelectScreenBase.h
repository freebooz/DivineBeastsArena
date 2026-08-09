// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/UI/Framework/DBACommonScreenBase.h"
#include "DBAServerSelectScreenBase.generated.h"

class UButton;
class UScrollBox;
class UTextBlock;
class UDBAServerListItemWidgetBase;
class UDBAServerSelectViewModel;
class UDBAServerSelectWidgetController;

/** WBP_DBA_ServerSelect 的 C++ 父类。布局、动画与 SafeZone 样式可在 Blueprint 配置，业务由 Controller/Flow 处理。 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAServerSelectScreenBase : public UDBACommonScreenBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|ServerSelect")
	void ActivateScreen();

	UFUNCTION(BlueprintPure, Category = "DBA|ServerSelect")
	UDBAServerSelectViewModel* GetViewModel() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	void EnsureFallbackLayout();
	void RefreshPresentation();

	UFUNCTION()
	void HandleViewModelChanged();

	UFUNCTION()
	void HandleRefreshClicked();

	UFUNCTION()
	void HandleRetryClicked();

	UFUNCTION()
	void HandleConfirmClicked();

	UFUNCTION()
	void HandleServerChosen(const FString& ServerId);

	UFUNCTION()
	void HandleBackRequested();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|ServerSelect")
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|ServerSelect")
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|ServerSelect")
	TObjectPtr<UScrollBox> ServerList;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|ServerSelect")
	TObjectPtr<UButton> RefreshButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|ServerSelect")
	TObjectPtr<UButton> RetryButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|ServerSelect")
	TObjectPtr<UButton> ConfirmButton;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|ServerSelect")
	TSubclassOf<UDBAServerListItemWidgetBase> ServerListItemWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UDBAServerSelectWidgetController> WidgetController;
};
