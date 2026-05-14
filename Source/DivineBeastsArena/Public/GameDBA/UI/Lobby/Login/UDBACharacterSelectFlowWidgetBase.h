// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Account/DBAAccountTypes.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "UDBACharacterSelectFlowWidgetBase.generated.h"

class UButton;
class UTextBlock;
class UWidget;
class UViewport;
class UDBALoginFlowSubsystem;
class ADBACharacterPreviewActor;
class ADirectionalLight;
class ASkyLight;
class USoundBase;
class UAudioComponent;

UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBACharacterSelectFlowWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBACharacterSelectFlowWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	virtual void UpdateCharacters(const TArray<FDBACharacterSummary>& Characters);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	virtual void SelectCharacter(const FDBACharacterId& CharacterId);

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	virtual void ConfirmSelectedCharacter();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	virtual void EnterCharacterCreate();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	virtual void RefreshCharacterList();

protected:
	UFUNCTION()
	void HandleConfirmClicked();

	UFUNCTION()
	void HandleCreateClicked();

	UFUNCTION()
	void HandleRefreshClicked();

	UFUNCTION()
	void HandleCharactersLoaded(const TArray<FDBACharacterSummary>& Characters);

	UFUNCTION()
	void HandleFlowError(const FString& ErrorMessage);

	UFUNCTION()
	void HandleBackgroundMusicFinished();

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterSelect", meta = (DisplayName = "On Characters Updated"))
	void BP_OnCharactersUpdated(const TArray<FDBACharacterSummary>& Characters);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterSelect", meta = (DisplayName = "On Character Selected"))
	void BP_OnCharacterSelected(const FDBACharacterId& CharacterId);

	void EnsureNativeFallbackLayout();
	void BindControls();
	void UnbindControls();
	void RefreshCharacterText();
	void SetStatus(const FText& InStatusText);
	UDBALoginFlowSubsystem* GetLoginFlow() const;
	void InitializePreviewViewport();
	void DestroyPreviewViewport();
	void UpdateCharacterPreviewById(const FDBACharacterId& CharacterId);
	void InitializeAudioAssets();
	void StartBackgroundMusic();
	void StopBackgroundMusic();
	void PlayButtonClickSfx() const;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect")
	TObjectPtr<UTextBlock> CharacterListText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect")
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect")
	TObjectPtr<UButton> ConfirmButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect")
	TObjectPtr<UButton> CreateButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect")
	TObjectPtr<UButton> RefreshButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect")
	TObjectPtr<UViewport> CharacterPreviewViewport;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect")
	TObjectPtr<UWidget> CharacterPreviewHost;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterSelect")
	TArray<FDBACharacterSummary> CurrentCharacters;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterSelect")
	FDBACharacterId SelectedCharacterId;

	UPROPERTY(Transient)
	TObjectPtr<ADBACharacterPreviewActor> PreviewActor;

	UPROPERTY(Transient)
	TObjectPtr<ADirectionalLight> PreviewDirectionalLight;

	UPROPERTY(Transient)
	TObjectPtr<ASkyLight> PreviewSkyLight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Audio")
	TObjectPtr<USoundBase> BackgroundMusicSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Audio")
	TObjectPtr<USoundBase> ButtonClickSound;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BackgroundMusicComponent;
};
