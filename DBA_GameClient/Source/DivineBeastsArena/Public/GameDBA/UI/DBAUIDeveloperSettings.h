// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明 UI 相关的 DeveloperSettings 配置入口，承载 UI 音效软引用和文案。
- 阅读重点：先看 UCLASS 配置和 UPROPERTY 字段，理解 UI 音效和文案的数据驱动入口。
- 修改提示：新增 UI 配置项时优先在此类中添加；避免在 Widget 中硬编码路径或文案。
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DBAUIDeveloperSettings.generated.h"

class USoundBase;
class UDBAUIFlowRegistry;
class UDBAZodiacHeroDataAsset;

/**
 * UDBAUIDeveloperSettings
 * UI 配置入口（P0-5/P0-6 修复：替代 Widget 中硬编码的音效路径和文案）
 * 通过 Project Settings -> Game -> DBA UI 配置修改
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="DBA UI 配置"))
class DIVINEBEASTSARENA_API UDBAUIDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// ==================== UI 流程资产配置 ====================

	/** 前端、选创角、大厅与竞技场 Widget 类注册表。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Flow")
	TSoftObjectPtr<UDBAUIFlowRegistry> DefaultUIFlowRegistry;

	// ==================== UI 音效配置 ====================

	/** UI 按钮点击音效（替代原硬编码 /Game/DBA/Audio/UI/SFX/SFX_UI_ButtonClick） */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Audio|SFX")
	TSoftObjectPtr<USoundBase> UIButtonClickSFX;

	/** 角色选择界面 BGM（替代原硬编码 /Game/DBA/Audio/UI/BGM/BGM_CharacterSelect_Loop） */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Audio|BGM")
	TSoftObjectPtr<USoundBase> CharacterSelectBGM;

	/** 角色创建界面 BGM（替代原硬编码 /Game/DBA/Audio/UI/BGM/BGM_CharacterCreate_Loop） */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Audio|BGM")
	TSoftObjectPtr<USoundBase> CharacterCreateBGM;

	/** 登录流程 BGM；角色选择与创建会优先使用各自的专用音乐。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Audio|BGM")
	TSoftObjectPtr<USoundBase> LoginFlowBGM;

	// ==================== UI 文案配置 ====================

	/** 启动视频跳过提示文案（替代原硬编码 TEXT("按 ESC 跳过")） */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Text")
	FText SplashSkipHintText;

	/** 选角与创建角色共用的生肖角色数据资产，界面通过异步事件读取描述、属性与技能。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|CharacterSelection")
	TSoftObjectPtr<UDBAZodiacHeroDataAsset> ZodiacCharacterSelectionData;
};
