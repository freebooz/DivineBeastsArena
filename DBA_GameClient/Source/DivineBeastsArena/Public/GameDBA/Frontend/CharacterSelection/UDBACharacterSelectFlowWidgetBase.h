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
#include "UDBACharacterSelectFlowWidgetBase.generated.h"

class UButton;
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
class DIVINEBEASTSARENA_API UDBACharacterSelectFlowWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBACharacterSelectFlowWidgetBase(const FObjectInitializer& ObjectInitializer);

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

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	virtual void SelectPreviousCharacter();

	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	virtual void SelectNextCharacter();

	/** 将选角界面铺满视口，供 UIManager 在 AddToViewport 后调用 */
	UFUNCTION(BlueprintCallable, Category = "DBA|CharacterSelect")
	void ApplyCharacterFlowViewportPresentation();

protected:
	UFUNCTION()
	void HandleConfirmClicked();

	UFUNCTION()
	void HandleCreateClicked();

	UFUNCTION()
	void HandleRefreshClicked();

	UFUNCTION()
	void HandlePrevClicked();

	UFUNCTION()
	void HandleNextClicked();

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
	void ResolveBoundWidgetsFromWidgetTree();
	void ApplyBlueprintLayoutOverrides();
	void ApplyLocalizedText();
	void BindControls();
	void UnbindControls();
	void RefreshCharacterText();
	void InitializeZodiacPresentationData();
	void HandleZodiacPresentationDataTableLoaded(UDataTable* LoadedTable, const FSoftObjectPath& AssetPath);
	void SetStatus(const FText& InStatusText);
	UDBAFrontendFlowSubsystem* GetLoginFlow() const;
	UDBAFrontendFlowController* GetFrontendFlowController() const;
	void InitializePresentationLevel();
	void ReleasePresentationLevel();
	void UpdatePresentedCharacterById(const FDBACharacterId& CharacterId);
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
	TObjectPtr<UButton> PrevCharacterButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|CharacterSelect")
	TObjectPtr<UButton> NextCharacterButton;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterSelect")
	TArray<FDBACharacterSummary> CurrentCharacters;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|CharacterSelect")
	FDBACharacterId SelectedCharacterId;

	UPROPERTY(Transient)
	bool bIsSubmittingSelection = false;

	UPROPERTY(Transient)
	TObjectPtr<ADBACharacterPresentationActor> PresentationStage;

	UPROPERTY(Transient)
	TObjectPtr<UDBAZodiacHeroDataAsset> ZodiacPresentationData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|CharacterSelect|Preview")
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
