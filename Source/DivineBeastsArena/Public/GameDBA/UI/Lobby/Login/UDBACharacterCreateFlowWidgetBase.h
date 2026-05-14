// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Account/DBAAccountTypes.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "UDBACharacterCreateFlowWidgetBase.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;
class UWidget;
class UViewport;
class UDBALoginFlowSubsystem;
class ADBACharacterPreviewActor;
class ADBACharacterPresentationActor;
class ADirectionalLight;
class ASkyLight;
class USoundBase;
class UAudioComponent;

UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBACharacterCreateFlowWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBACharacterCreateFlowWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	virtual FReply NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;
	virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent) override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	virtual void SetCharacterName(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	virtual void SetZodiac(EDBAZodiac Zodiac);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	virtual void SetElement(EDBAElement Element);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	virtual void SetFiveCamp(EDBAFiveCamp FiveCamp);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	virtual void Submit();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	virtual void BackToCharacterSelect();

protected:
	UFUNCTION()
	void HandleCreateClicked();

	UFUNCTION()
	void HandleBackClicked();

	UFUNCTION()
	void HandleZodiacClicked();

	UFUNCTION()
	void HandleElementClicked();

	UFUNCTION()
	void HandleFiveCampClicked();

	UFUNCTION()
	void HandleFlowError(const FString& ErrorMessage);

	UFUNCTION()
	void HandleBackgroundMusicFinished();

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterCreate", meta = (DisplayName = "On Validation Changed"))
	void BP_OnValidationChanged(bool bInIsValid, const FText& ValidationMessage);

	void EnsureNativeFallbackLayout();
	void BindControls();
	void UnbindControls();
	bool Validate();
	void RefreshChoiceText();
	void ShowValidationMessage(bool bValid, const FText& Message);
	UDBALoginFlowSubsystem* GetLoginFlow() const;
	void InitializePreviewViewport();
	void DestroyPreviewViewport();
	void RefreshPreviewCharacter();
	bool IsPointerOverPreviewHost(const FVector2D& ScreenPosition) const;
	void BeginPreviewRotationDrag(const FVector2D& ScreenPosition);
	void UpdatePreviewRotationDrag(const FVector2D& ScreenPosition);
	void EndPreviewRotationDrag();
	void InitializeAudioAssets();
	void StartBackgroundMusic();
	void StopBackgroundMusic();
	void PlayButtonClickSfx() const;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate")
	TObjectPtr<UEditableTextBox> CharacterNameInput;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate")
	TObjectPtr<UButton> ZodiacButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate")
	TObjectPtr<UButton> ElementButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate")
	TObjectPtr<UButton> FiveCampButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate")
	TObjectPtr<UButton> CreateButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate")
	TObjectPtr<UButton> BackButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate")
	TObjectPtr<UTextBlock> ZodiacText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate")
	TObjectPtr<UTextBlock> ElementText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate")
	TObjectPtr<UTextBlock> FiveCampText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate")
	TObjectPtr<UTextBlock> ValidationText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate")
	TObjectPtr<UViewport> CharacterPreviewViewport;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate")
	TObjectPtr<UWidget> CharacterPreviewHost;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	FString CharacterName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	EDBAZodiac SelectedZodiac = EDBAZodiac::Rat;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	EDBAElement SelectedElement = EDBAElement::Water;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	EDBAFiveCamp SelectedFiveCamp = EDBAFiveCamp::None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterCreate")
	bool bIsCreateValid = false;

	UPROPERTY(Transient)
	TObjectPtr<ADBACharacterPresentationActor> PreviewActor;

	UPROPERTY(Transient)
	TObjectPtr<ADirectionalLight> PreviewDirectionalLight;

	UPROPERTY(Transient)
	TObjectPtr<ADirectionalLight> PreviewFillLight;

	UPROPERTY(Transient)
	TObjectPtr<ASkyLight> PreviewSkyLight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|CharacterCreate|Preview")
	float PreviewDragRotationDegreesPerPixel = 0.28f;

	UPROPERTY(Transient)
	bool bIsPreviewRotationDragging = false;

	UPROPERTY(Transient)
	FVector2D LastPreviewDragScreenPosition = FVector2D::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Audio")
	TObjectPtr<USoundBase> BackgroundMusicSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Audio")
	TObjectPtr<USoundBase> ButtonClickSound;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BackgroundMusicComponent;
};
