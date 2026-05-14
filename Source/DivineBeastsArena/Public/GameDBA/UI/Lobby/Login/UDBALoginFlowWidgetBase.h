// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "UDBALoginFlowWidgetBase.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;

UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBALoginFlowWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBALoginFlowWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	virtual void SubmitLogin();

	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	virtual void SubmitGuestLogin();

	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	virtual void ShowError(const FString& ErrorMessage);

	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	virtual void ClearError();

protected:
	UFUNCTION()
	void HandleLoginClicked();

	UFUNCTION()
	void HandleGuestLoginClicked();

	UFUNCTION()
	void HandleFlowStateChanged(EDBALoginFlowState NewState);

	UFUNCTION()
	void HandleFlowError(const FString& ErrorMessage);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Login", meta = (DisplayName = "On Show Error"))
	void BP_OnShowError(const FString& ErrorMessage);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Login", meta = (DisplayName = "On Clear Error"))
	void BP_OnClearError();

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Login", meta = (DisplayName = "On Flow State Changed"))
	void BP_OnFlowStateChanged(EDBALoginFlowState NewState);

	void EnsureNativeFallbackLayout();
	void BindControls();
	void UnbindControls();
	void SetStatus(const FText& InStatusText);
	UDBALoginFlowSubsystem* GetLoginFlow() const;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UEditableTextBox> EmailInput;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UEditableTextBox> PasswordInput;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UButton> LoginButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UButton> GuestLoginButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UTextBlock> ErrorText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login")
	FString LastErrorMessage;
};
