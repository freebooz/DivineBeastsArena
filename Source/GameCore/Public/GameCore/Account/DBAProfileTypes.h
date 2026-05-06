// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "DBAProfileTypes.generated.h"

/**
 * 图形质量等级
 */
UENUM(BlueprintType)
enum class EDBAGraphicsQuality : uint8
{
	/** 低 */
	Low = 0 UMETA(DisplayName = "低"),

	/** 中 */
	Medium UMETA(DisplayName = "中"),

	/** 高 */
	High UMETA(DisplayName = "高"),

	/** 极高 */
	Epic UMETA(DisplayName = "极高"),

	/** 自定义 */
	Custom UMETA(DisplayName = "自定义")
};

/**
 * 按键绑定
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAKeyBinding
{
	GENERATED_BODY()

	/** 动作名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FName ActionName;

	/** 主按键 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FKey PrimaryKey;

	/** 备用按键 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	FKey SecondaryKey;

	FDBAKeyBinding()
		: ActionName(NAME_None)
	{
	}

	FDBAKeyBinding(FName InActionName, FKey InPrimaryKey, FKey InSecondaryKey = EKeys::Invalid)
		: ActionName(InActionName)
		, PrimaryKey(InPrimaryKey)
		, SecondaryKey(InSecondaryKey)
	{
	}
};

/**
 * 图形设置
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAGraphicsSettings
{
	GENERATED_BODY()

	/** 图形质量等级 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
	EDBAGraphicsQuality QualityLevel = EDBAGraphicsQuality::High;

	/** 分辨率宽度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
	int32 ResolutionWidth = 1920;

	/** 分辨率高度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
	int32 ResolutionHeight = 1080;

	/** 全屏模式 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
	bool bFullscreen = true;

	/** 垂直同步 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
	bool bVSync = false;

	/** 帧率限制 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
	int32 FrameRateLimit = 144;

	FDBAGraphicsSettings()
	{
	}
};

/**
 * 音频设置
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAAudioSettings
{
	GENERATED_BODY()

	/** 主音量 (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MasterVolume = 1.0f;

	/** 音乐音量 (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MusicVolume = 0.8f;

	/** 音效音量 (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SFXVolume = 1.0f;

	/** 语音音量 (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VoiceVolume = 1.0f;

	/** 静音 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bMuted = false;

	FDBAAudioSettings()
	{
	}
};

/**
 * 游戏设置
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAGameplaySettings
{
	GENERATED_BODY()

	/** 鼠标灵敏度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float MouseSensitivity = 1.0f;

	/** 摄像机距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (ClampMin = "500.0", ClampMax = "3000.0"))
	float CameraDistance = 1500.0f;

	/** 显示伤害数字 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bShowDamageNumbers = true;

	/** 显示血条 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bShowHealthBars = true;

	/** 自动攻击 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bAutoAttack = true;

	/** 快速施法 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bQuickCast = false;

	FDBAGameplaySettings()
	{
	}
};

/**
 * UI 布局数据
 * 存储 UI 元素的位置和大小
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAUILayoutData
{
	GENERATED_BODY()

	/** 布局名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FString LayoutName;

	/** 布局数据（JSON 格式） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FString LayoutData;

	FDBAUILayoutData()
		: LayoutName(TEXT("Default"))
	{
	}
};

/**
 * 玩家 Profile
 * 包含玩家的所有设置和偏好
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAPlayerProfile
{
	GENERATED_BODY()

	/** 图形设置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FDBAGraphicsSettings GraphicsSettings;

	/** 音频设置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FDBAAudioSettings AudioSettings;

	/** 游戏设置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FDBAGameplaySettings GameplaySettings;

	/** 按键绑定 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	TArray<FDBAKeyBinding> KeyBindings;

	/** UI 布局 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	TArray<FDBAUILayoutData> UILayouts;

	/** 当前 UI 布局索引 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	int32 CurrentUILayoutIndex = 0;

	FDBAPlayerProfile()
	{
	}
};
