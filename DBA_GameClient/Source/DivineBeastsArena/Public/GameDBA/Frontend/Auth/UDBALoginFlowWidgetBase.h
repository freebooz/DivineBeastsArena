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
#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "UDBALoginFlowWidgetBase.generated.h"

class UButton;
class UEditableTextBox;
class FViewport;
class UTextBlock;
class UTexture2D;
class UWidget;
class USoundBase;
class UAudioComponent;
class UDBAFrontendFlowController;

USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBALoginVisualLayoutSpec
{
	GENERATED_BODY()

	/** 登录面板水平锚点（0.5 = 居中） */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login")
	float PanelAnchorX = 0.50f;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login")
	FText TitleText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login")
	FText PrimaryButtonText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login")
	TArray<FText> LeftToolLabels;

	/** 原生兜底布局参考设计分辨率（与 DBAUIFonts 参考视口一致） */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login|Layout")
	float ReferenceDesignWidth = 1920.0f;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login|Layout")
	float ReferenceDesignHeight = 1080.0f;

	/** 登录表单面板基准尺寸（1920x1080 设计坐标） */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login|Layout")
	float PanelWidth = 900.0f;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login|Layout")
	float PanelHeight = 640.0f;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login|Layout")
	float PanelTopOffset = 286.0f;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login|Layout")
	float PanelPadding = 56.0f;

	/** 输入行背景高度与可编辑框最小高度（1920x1080 设计坐标） */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login|Layout")
	float InputRowHeight = 72.0f;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login|Layout")
	float InputEditableHeight = 58.0f;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login|Layout")
	float InputMinDesiredWidth = 580.0f;

	/** 小分辨率下可选 viewport 缩放下限（仅当 fit 缩放低于此值时生效，不得导致溢出） */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login|Layout")
	float MinViewportUIScale = 0.72f;

	/** 紧凑视口判定宽度阈值 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login|Layout")
	float CompactViewportWidthThreshold = 1280.0f;

	/** 紧凑视口判定高度阈值 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login|Layout")
	float CompactViewportHeightThreshold = 720.0f;
};

UCLASS(Blueprintable, BlueprintType, meta = (DisableNativeTick = "true"))
class DIVINEBEASTSARENA_API UDBALoginFlowWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBALoginFlowWidgetBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "DBA|Login|Visual")
	static FDBALoginVisualLayoutSpec GetReferenceVisualLayoutSpec();

	/** 登录流程专用视口缩放（按 fit 比例缩放，避免小分辨率溢出） */
	UFUNCTION(BlueprintPure, Category = "DBA|Login|Visual")
	float GetLoginFlowViewportScale() const;

	/** 将登录控件按参考设计分辨率缩放并居中适配视口 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Login|Visual")
	void ApplyLoginViewportPresentation();

	/** 下一帧重新计算登录布局（视口尺寸尚未就绪时） */
	UFUNCTION(BlueprintCallable, Category = "DBA|Login|Visual")
	void ScheduleLoginLayoutRefresh();

	/** 聚焦默认账号输入框（含延迟重试） */
	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	void FocusDefaultInput();

	/** 供 GameUIManager 设置输入模式时使用的默认焦点控件 */
	UFUNCTION(BlueprintPure, Category = "DBA|Login")
	UEditableTextBox* GetDefaultInputFocusWidget() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	void HandleViewportResized(FViewport* Viewport, uint32 Unused);

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
	void HandleRememberToggleClicked();

	UFUNCTION()
	void HandleAgreementToggleClicked();

	UFUNCTION()
	void HandlePasswordVisibilityClicked();

	UFUNCTION()
	void HandleServerSelectClicked();

	UFUNCTION()
	void HandleForgotPasswordClicked();

	UFUNCTION()
	void HandleRegisterAccountClicked();

	UFUNCTION()
	void HandleAnnouncementClicked();

	UFUNCTION()
	void HandleSupportClicked();

	UFUNCTION()
	void HandleRepairClicked();

	UFUNCTION()
	void HandleUserAgreementClicked();

	UFUNCTION()
	void HandlePrivacyPolicyClicked();

	UFUNCTION()
	void HandleFlowStateChanged(EDBALoginFlowState NewState);

	UFUNCTION()
	void HandleFlowError(const FString& ErrorMessage);

	UFUNCTION()
	void HandleBackgroundMusicFinished();

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
	UDBAFrontendFlowSubsystem* GetLoginFlow() const;
	UDBAFrontendFlowController* GetFrontendFlowController() const;
	void InitializeAudioAssets();
	void StartBackgroundMusic();
	void StopBackgroundMusic();
	void PlayButtonClickSfx() const;
	void InitializeVisualAssets();
	void BuildReferenceNativeLayout();
	void ApplyVisualStyle();
	void ApplyComposedInputStyle(UEditableTextBox* TextBox) const;
	void ApplyLoginBackgroundTexture();
	void ApplyButtonTextureStyle(UButton* Button) const;
	void ApplyGuestButtonStyle(UButton* Button) const;
	void UpdateLoadingStateByFlow(EDBALoginFlowState NewState);
	bool CanSubmitLoginAction();
	void UpdateReferenceToggleVisuals();
	void ApplyLoginResponsiveLayout();
	void EnsureReferenceDesignRootSizeBox();
	void HandleDeferredLoginLayoutRefresh();

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UWidget> LoginPanel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UEditableTextBox> EmailInput;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UEditableTextBox> PasswordInput;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UButton> LoginButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UButton> GuestLoginButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UButton> RememberToggleButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UButton> AgreementToggleButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UButton> PasswordVisibilityButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UButton> ServerSelectButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UButton> ForgotPasswordButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UButton> RegisterAccountButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UButton> AnnouncementButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UButton> SupportButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UButton> RepairButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UButton> UserAgreementButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UButton> PrivacyPolicyButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UTextBlock> ErrorText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UTextBlock> RememberCheckText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UTextBlock> AgreementCheckText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UTextBlock> PasswordEyeText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UTextBlock> ServerNameText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login")
	FString LastErrorMessage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Visual")
	bool bUseReferenceNativeLayout = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Visual")
	bool bUseComposedImageLayout = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Login")
	bool bRememberAccount = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Login")
	bool bAgreementAccepted = true;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login")
	bool bPasswordVisible = false;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login")
	int32 SelectedServerIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Login")
	TArray<FText> AvailableServers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Audio")
	TObjectPtr<USoundBase> BackgroundMusicSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Audio")
	TObjectPtr<USoundBase> ButtonClickSound;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BackgroundMusicComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Visual")
	TObjectPtr<UTexture2D> LoginPanelTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Visual")
	TObjectPtr<UTexture2D> LoginButtonTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Visual")
	TObjectPtr<UTexture2D> LoginBackgroundTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Visual")
	TObjectPtr<UTexture2D> ReferenceInputTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Visual")
	TObjectPtr<UTexture2D> ReferenceFrameTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Visual")
	TObjectPtr<UTexture2D> ReferenceGuestButtonTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Visual")
	TObjectPtr<UTexture2D> ReferenceSideToolTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Visual")
	TObjectPtr<UTexture2D> ReferenceAgeBadgeTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Visual")
	TObjectPtr<UTexture2D> ReferenceCheckboxOnTexture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Visual")
	TObjectPtr<UTexture2D> ReferenceCheckboxOffTexture;

	FTimerHandle DelayedFocusTimerHandle;
	FTimerHandle DelayedLayoutRefreshTimerHandle;

	int32 LoginLayoutRefreshRetryCount = 0;
	bool bLoggedViewportMismatch = false;
};
