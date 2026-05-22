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
#include "Blueprint/UserWidget.h"
#include "UObject/SoftObjectPtr.h"
#include "DBAUserWidgetBase.generated.h"

class UButton;
class USoundBase;

/**
 * DBAUserWidgetBase
 *
 * UMG UserWidget 基类
 * 用于所有可在屏幕上显示的 UI 控件
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class GAMECORE_API UDBAUserWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UDBAUserWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	void HandleAnyButtonClicked();

	void BindButtonClickAudio();

public:
	/**
	 * Widget 被激活时调用（显示时）
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI")
	virtual void Activate();

	/**
	 * Widget 被取消激活时调用（隐藏时）
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI")
	virtual void Deactivate();

	/**
	 * 检查 Widget 是否处于激活状态
	 */
	UFUNCTION(BlueprintPure, Category = "DBA|UI")
	bool IsActive() const { return bIsActive; }

protected:
	/** Widget 是否处于激活状态 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI")
	bool bIsActive = false;

	/** Widget 的优先级（数值越高优先级越高） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI")
	int32 WidgetPriority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Audio")
	bool bAutoBindClickSound = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Audio")
	TSoftObjectPtr<USoundBase> DefaultClickSound;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> BoundButtons;
};
