// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameCore/Networking/Account/DBAAccountTypes.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "UDBACharacterCreateFlowWidgetBase.generated.h"

class UButton;
class UEditableText;
class UEditableTextBox;
class UTextBlock;
class UWidget;
class UDBAFrontendFlowSubsystem;
class UDBAFrontendFlowController;
class UDBAZodiacHeroDataAsset;
class UDataTable;
class ADBACharacterPresentationActor;
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
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
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

	/** 将创建界面铺满视口，供 UIManager 在 AddToViewport 后调用 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterCreate")
	void ApplyCharacterFlowViewportPresentation();

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
	void HandleCharacterNameChanged(const FText& NewText);

	UFUNCTION()
	void HandleFlowError(const FString& ErrorMessage);

	UFUNCTION()
	void HandleBackgroundMusicFinished();

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|CharacterCreate", meta = (DisplayName = "On Validation Changed"))
	void BP_OnValidationChanged(bool bInIsValid, const FText& ValidationMessage);

	void EnsureNativeFallbackLayout();
	void ResolveBoundWidgetsFromWidgetTree();
	void ApplyBlueprintLayoutOverrides();
	void ApplyLocalizedText();
	void BindControls();
	void UnbindControls();
	bool Validate();
	bool ValidateCharacterName(FText& OutMessage) const;
	void RefreshChoiceText();
	void InitializeZodiacPresentationData();
	void HandleZodiacPresentationDataTableLoaded(UDataTable* LoadedTable, const FSoftObjectPath& AssetPath);
	void ShowValidationMessage(bool bValid, const FText& Message);
	UDBAFrontendFlowSubsystem* GetLoginFlow() const;
	UDBAFrontendFlowController* GetFrontendFlowController() const;
	void InitializePresentationLevel();
	void ReleasePresentationLevel();
	void RefreshPresentedCharacter();
	void BindPlacedPresentationStage();

	UFUNCTION()
	void HandleDeferredPresentationStageBinding();
	bool IsPointerOverInteractiveControl(const FVector2D& ScreenPosition) const;
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

	/** 兼容旧版 Widget Blueprint 中使用的 UEditableText 名称控件。 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterCreate")
	TObjectPtr<UEditableText> CharacterNameEditableText;

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
	bool bIsSubmittingCreate = false;

	UPROPERTY(Transient)
	TObjectPtr<ADBACharacterPresentationActor> PresentationStage;

	UPROPERTY(Transient)
	TObjectPtr<UDBAZodiacHeroDataAsset> ZodiacPresentationData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|CharacterCreate|Preview")
	float PreviewDragRotationDegreesPerPixel = 0.55f;

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

	UPROPERTY(Transient)
	FTimerHandle DeferredPresentationActivateTimerHandle;

	UPROPERTY(Transient)
	int32 DeferredPresentationActivateRetryCount = 0;
};
