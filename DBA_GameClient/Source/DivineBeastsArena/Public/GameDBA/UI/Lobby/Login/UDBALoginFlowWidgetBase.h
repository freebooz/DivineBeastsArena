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
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "UDBALoginFlowWidgetBase.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;
class UTexture2D;
class UWidget;
class USoundBase;
class UAudioComponent;

USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBALoginVisualLayoutSpec
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login")
	float PanelAnchorX = 0.66f;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login")
	FText TitleText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login")
	FText PrimaryButtonText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login")
	TArray<FText> LeftToolLabels;
};

UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBALoginFlowWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBALoginFlowWidgetBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "DBA|Login|Visual")
	static FDBALoginVisualLayoutSpec GetReferenceVisualLayoutSpec();

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
	virtual void SubmitDebugLogin(const FString& Username = TEXT("dba_dev_01"));

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
	void HandleDebugLoginClicked();

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
	UDBALoginFlowSubsystem* GetLoginFlow() const;
	void InitializeAudioAssets();
	void StartBackgroundMusic();
	void StopBackgroundMusic();
	void PlayButtonClickSfx() const;
	void InitializeVisualAssets();
	void BuildReferenceNativeLayout();
	void ApplyVisualStyle();
	void ApplyLoginBackgroundTexture();
	void ApplyButtonTextureStyle(UButton* Button) const;
	void ApplyGuestButtonStyle(UButton* Button) const;
	void UpdateLoadingStateByFlow(EDBALoginFlowState NewState);

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
	TObjectPtr<UButton> DebugLoginButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UTextBlock> ErrorText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Login")
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login")
	FString LastErrorMessage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Visual")
	bool bUseReferenceNativeLayout = false;

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
};
