// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：GameCore 通用 Widget 默认资源的 DeveloperSettings 配置入口。
- 阅读重点：默认点击音效只通过项目配置提供软引用，运行时由 Widget 异步加载。
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DBAUserWidgetDeveloperSettings.generated.h"

class USoundBase;

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="DBA 通用 UI 配置"))
class GAMECORE_API UDBAUserWidgetDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 通用 Widget 默认点击音效；由 DefaultGame.ini 配置。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|UI|Audio")
	TSoftObjectPtr<USoundBase> DefaultClickSound;
};
