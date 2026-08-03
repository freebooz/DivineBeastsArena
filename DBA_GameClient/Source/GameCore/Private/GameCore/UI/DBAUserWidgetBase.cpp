// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameCore/UI/DBAUserWidgetBase.h"
#include "GameCore/Core/DBALogChannels.h"
#include "GameCore/UI/DBAUserWidgetDeveloperSettings.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/SoftObjectPath.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

/**
 * 构造函数
 * 初始化用户 Widget 基类
 */
UDBAUserWidgetBase::UDBAUserWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (const UDBAUserWidgetDeveloperSettings* Settings = GetDefault<UDBAUserWidgetDeveloperSettings>())
	{
		DefaultClickSound = Settings->DefaultClickSound;
	}
	else
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[DBAUserWidgetBase] 未找到通用 UI 配置，默认点击音效软引用为空。"));
	}
}

/**
 * 原生初始化回调
 * 当 Widget 控制器和绑定Widget 完成初始化时调用
 */
void UDBAUserWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 异步预加载默认点击音效，避免按钮点击时同步加载阻塞 GameThread。
	PreloadDefaultClickSound();
}

/**
 * 原生构建回调
 * 当 Widget 被构建到屏幕上时调用
 * 适合进行事件绑定、数据初始化等操作
 */
void UDBAUserWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (bAutoBindClickSound)
	{
		BindButtonClickAudio();
	}
}

/**
 * 原生销毁回调
 * 当 Widget 从屏幕上移除时调用
 * 适合进行事件解绑、计时器清理等操作
 */
void UDBAUserWidgetBase::NativeDestruct()
{
	for (UButton* Button : BoundButtons)
	{
		if (Button)
		{
			Button->OnClicked.RemoveDynamic(this, &UDBAUserWidgetBase::HandleAnyButtonClicked);
		}
	}
	BoundButtons.Reset();

	// 取消未完成的异步加载句柄，避免回调悬挂。
	if (ClickSoundStreamableHandle.IsValid())
	{
		ClickSoundStreamableHandle->CancelHandle();
		ClickSoundStreamableHandle.Reset();
	}

	Super::NativeDestruct();
}

/**
 * 原生 Tick 回调
 * 每帧调用，用于 Widget 动画和状态更新
 * @param MyGeometry 当前 Widget 的几何信息
 * @param InDeltaTime 距离上一帧的时间
 */
void UDBAUserWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

/**
 * 激活 Widget
 * 设置 bIsActive 为 true，表示 Widget 正在显示
 * 子类可重写此方法添加激活时的逻辑
 */
void UDBAUserWidgetBase::Activate()
{
	bIsActive = true;
}

/**
 * 停用 Widget
 * 设置 bIsActive 为 false，表示 Widget 已隐藏
 * 子类可重写此方法添加停用时的逻辑
 */
void UDBAUserWidgetBase::Deactivate()
{
	bIsActive = false;
}

void UDBAUserWidgetBase::HandleAnyButtonClicked()
{
	USoundBase* ClickSound = CachedClickSound.Get();
	if (!ClickSound)
	{
		// 音效尚未异步加载完成，输出一次中文警告日志，避免日志刷屏。
		if (!bHasLoggedClickSoundNotReady)
		{
			UE_LOG(LogDBAUI, Warning, TEXT("[DBAUserWidgetBase] 默认点击音效尚未异步加载完成，本次点击跳过音效播放。软引用路径：%s"),
				DefaultClickSound.IsValid() ? TEXT("已加载但弱指针失效") : *DefaultClickSound.ToSoftObjectPath().ToString());
			bHasLoggedClickSoundNotReady = true;
		}
		return;
	}

	// 重置警告标记，便于下次未就绪时再次提示。
	bHasLoggedClickSoundNotReady = false;
	UGameplayStatics::PlaySound2D(this, ClickSound, 0.85f, 1.0f, 0.0f, nullptr, nullptr, true);
}

void UDBAUserWidgetBase::PreloadDefaultClickSound()
{
	// 若已缓存或软引用无效，直接返回。
	if (CachedClickSound.IsValid())
	{
		return;
	}

	if (!DefaultClickSound.IsValid())
	{
		// 软引用尚未加载，发起异步加载。
		const FSoftObjectPath& SoundPath = DefaultClickSound.ToSoftObjectPath();
		if (!SoundPath.IsValid())
		{
		UE_LOG(LogDBAUI, Warning, TEXT("[DBAUserWidgetBase] DefaultClickSound 软引用路径无效，无法异步加载。"));
			return;
		}

		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		ClickSoundStreamableHandle = Streamable.RequestAsyncLoad(
			SoundPath,
			FStreamableDelegate::CreateUObject(this, &UDBAUserWidgetBase::HandleDefaultClickSoundLoaded));
	}
	else
	{
		// 软引用已加载，直接缓存。
		HandleDefaultClickSoundLoaded();
	}
}

void UDBAUserWidgetBase::HandleDefaultClickSoundLoaded()
{
	ClickSoundStreamableHandle.Reset();

	USoundBase* LoadedSound = DefaultClickSound.Get();
	if (!LoadedSound)
	{
		UE_LOG(LogDBAUI, Warning, TEXT("[DBAUserWidgetBase] 默认点击音效异步加载完成但解析为空，请检查软引用路径：%s"),
			*DefaultClickSound.ToSoftObjectPath().ToString());
		return;
	}

	CachedClickSound = LoadedSound;
	UE_LOG(LogDBAUI, Log, TEXT("[DBAUserWidgetBase] 默认点击音效异步加载完成并已缓存：%s"),
		*LoadedSound->GetName());
}

void UDBAUserWidgetBase::BindButtonClickAudio()
{
	if (!WidgetTree)
	{
		return;
	}

	TArray<UWidget*> AllWidgets;
	WidgetTree->GetAllWidgets(AllWidgets);
	for (UWidget* Widget : AllWidgets)
	{
		UButton* Button = Cast<UButton>(Widget);
		if (!Button || BoundButtons.Contains(Button))
		{
			continue;
		}

		Button->OnClicked.RemoveDynamic(this, &UDBAUserWidgetBase::HandleAnyButtonClicked);
		Button->OnClicked.AddDynamic(this, &UDBAUserWidgetBase::HandleAnyButtonClicked);
		BoundButtons.Add(Button);
	}
}
