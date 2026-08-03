// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：GameMoba UI 基类默认资源的 DeveloperSettings 配置入口。
- 阅读重点：DefaultClickSound / DefaultBackgroundTexture 软引用，供 UDBAMobaUserWidgetBase 读取。
- 修改提示：新增 MOBA UI 默认资源时优先在此类配置，避免在 Widget 构造函数硬编码路径。
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DBAMobaUIDeveloperSettings.generated.h"

class USoundBase;
class UTexture2D;

/**
 * UDBAMobaUIDeveloperSettings
 * GameMoba UI 默认资源配置（P0-6：替代 UDBAMobaUserWidgetBase 构造函数硬编码软路径）
 * 通过 Project Settings -> Game -> DBA Moba UI 配置修改
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="DBA Moba UI 配置"))
class GAMEMOBA_API UDBAMobaUIDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** UI 默认点击音效（异步预加载，避免构造函数同步阻塞） */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|MobaUI|Audio")
	TSoftObjectPtr<USoundBase> DefaultClickSound;

	/** UI 默认背景纹理（引擎内置或项目资源，按需加载） */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|MobaUI|Visual")
	TSoftObjectPtr<UTexture2D> DefaultBackgroundTexture;
};
