// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// GameMoba - 通用MOBA用户界面基类

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UObject/SoftObjectPtr.h"
#include "UDBAMobaUserWidgetBase.generated.h"

class UButton;
class UImage;
class USoundBase;
class UTexture2D;
struct FStreamableHandle;

/**
 * UDBAMobaUserWidgetBase
 * MOBA游戏通用用户界面基类
 * 提供UI通用接口和事件
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class GAMEMOBA_API UDBAMobaUserWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UDBAMobaUserWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	//~ Begin UUserWidget Interface
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//~ End UUserWidget Interface

	UFUNCTION()
	void HandleAnyButtonClicked();

	void BindButtonClickAudio();
	void ApplyDefaultBackgroundTexture();

	/** 异步预加载默认点击音效，避免在按钮点击回调中同步加载阻塞 GameThread。 */
	void PreloadDefaultClickSound();

	/** 音效异步加载完成回调。 */
	void HandleDefaultClickSoundLoaded();

protected:
	/** 显示UI时的动画 */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|Base", meta = (DisplayName = "On Show"))
	void BP_OnShow();

	/** 隐藏UI时的动画 */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|Base", meta = (DisplayName = "On Hide"))
	void BP_OnHide();

public:
	/** 是否可见 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Base")
	bool bIsVisible = true;

	/** UI层级 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Base")
	int32 UILevel = 0;

protected:
	/** 所属PlayerController */
	UPROPERTY()
	TWeakObjectPtr<class APlayerController> OwnerPlayerController;

	/** 自动给该Widget树内按钮绑定点击音效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Audio")
	bool bAutoBindClickSound = true;

	/** 自动给根节点插入背景图（如果根容器支持） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Visual")
	bool bAutoInjectBackground = true;

	/** UI点击音效资产 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Audio")
	TSoftObjectPtr<USoundBase> DefaultClickSound;

	/** UI默认背景纹理 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Visual")
	TSoftObjectPtr<UTexture2D> DefaultBackgroundTexture;

	/** 背景透明度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Visual", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BackgroundOpacity = 0.18f;

	/** 已绑定过点击事件的按钮集合 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UButton>> BoundButtons;

	/** 自动注入的背景图引用 */
	UPROPERTY(Transient)
	TObjectPtr<UImage> InjectedBackgroundImage;

	/** 异步加载完成后的音效缓存（弱指针，避免阻止音效 GC）。 */
	UPROPERTY(Transient)
	TWeakObjectPtr<USoundBase> CachedClickSound;

	/** 异步加载句柄，用于取消或生命周期管理。 */
	TSharedPtr<FStreamableHandle> ClickSoundStreamableHandle;

	/** 标记是否已输出过"音效未就绪"警告日志，避免日志刷屏。 */
	bool bHasLoggedClickSoundNotReady = false;
};
