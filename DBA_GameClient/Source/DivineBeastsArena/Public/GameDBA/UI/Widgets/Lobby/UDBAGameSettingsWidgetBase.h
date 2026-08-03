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
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "UDBAGameSettingsWidgetBase.generated.h"

class UButton;
class UBorder;
class UCheckBox;
class UImage;
class USlider;
class UTextBlock;
class UTexture2D;

UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAGameSettingsWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAGameSettingsWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|Settings")
	void RefreshFromRuntime();

	UFUNCTION(BlueprintCallable, Category = "DBA|Settings")
	void ApplySettings();

	UFUNCTION(BlueprintCallable, Category = "DBA|Settings")
	void ResetDefaults();

	UFUNCTION(BlueprintCallable, Category = "DBA|Settings")
	void CloseSettings();

protected:
	void EnsureNativeFallbackLayout();
	void ResolveBoundWidgetsFromWidgetTree();
	void ApplyPanelBackgroundTexture();
	void BindControls();
	void UnbindControls();
	void UpdateStatus(const FText& Text);
	void ApplyMasterVolume(float Volume) const;
	float SliderToMouseSensitivity(float SliderValue) const;
	float MouseSensitivityToSlider(float Sensitivity) const;
	float SliderToCameraDistance(float SliderValue) const;
	float CameraDistanceToSlider(float Distance) const;

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleApplyClicked();

	UFUNCTION()
	void HandleResetClicked();

	UFUNCTION()
	void HandleMasterVolumeChanged(float Value);

	UFUNCTION()
	void HandleMouseSensitivityChanged(float Value);

	UFUNCTION()
	void HandleCameraDistanceChanged(float Value);

	UFUNCTION()
	void HandleFullscreenChanged(bool bIsChecked);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Settings")
	void BP_OnSettingsApplied(float MasterVolume, float MouseSensitivity, float CameraDistance, bool bFullscreen);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Settings")
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Settings")
	TObjectPtr<UButton> ApplyButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Settings")
	TObjectPtr<UButton> ResetButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Settings")
	TObjectPtr<USlider> MasterVolumeSlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Settings")
	TObjectPtr<USlider> MouseSensitivitySlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Settings")
	TObjectPtr<USlider> CameraDistanceSlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Settings")
	TObjectPtr<UCheckBox> FullscreenCheckBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Settings")
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Settings|Visual")
	TObjectPtr<UBorder> PanelBackgroundBorder;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "DBA|Settings|Visual")
	TObjectPtr<UImage> PanelBackgroundImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Settings|Visual")
	TSoftObjectPtr<UTexture2D> PanelBackgroundTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Settings|Visual")
	FVector2D NativePanelSize = FVector2D(640.0f, 460.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Settings")
	float DefaultMasterVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Settings")
	float DefaultMouseSensitivity = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Settings")
	float DefaultCameraDistance = 520.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Settings")
	float MinMouseSensitivity = 0.04f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Settings")
	float MaxMouseSensitivity = 0.80f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DBA|Settings")
	float CurrentMasterVolume = 1.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DBA|Settings")
	float CurrentMouseSensitivity = 0.25f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DBA|Settings")
	float CurrentCameraDistance = 520.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DBA|Settings")
	bool bCurrentFullscreen = false;
};
