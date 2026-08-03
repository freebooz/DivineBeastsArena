// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明英雄成长属性默认值的数据资产软引用配置入口。
- 修改提示：这里只保存软引用，不承载成长逻辑或运行数值。
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBAHeroGrowthDefaultsDataAsset.h"
#include "DBAHeroGrowthDeveloperSettings.generated.h"

/**
 * 英雄成长属性开发者设置
 *
 * 在 DefaultGame.ini 中配置：
 *   [/Script/DivineBeastsArena.DBAHeroGrowthDeveloperSettings]
 *   DefaultHeroGrowthDefaults=/Game/DBA/Gameplay/Progression/DA_DBA_HeroGrowthDefaults.DA_DBA_HeroGrowthDefaults
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="DBA 英雄成长属性设置"))
class DIVINEBEASTSARENA_API UDBAHeroGrowthDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 项目级默认英雄成长属性数据资产。PlayerState 创建 AttributeSet 后会异步加载并应用。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|HeroGrowth Attribute")
	TSoftObjectPtr<UDBAHeroGrowthDefaultsDataAsset> DefaultHeroGrowthDefaults;
};
