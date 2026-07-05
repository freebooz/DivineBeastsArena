// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明可玩技能目录的项目级开发配置入口。
- 修改提示：这里只保存软引用配置，不承载技能逻辑或运行数值。
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameDBA/Combat/DBAPlayableSkillCatalogDataAsset.h"
#include "DBAPlayableSkillDeveloperSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="DBA 可玩技能设置"))
class DIVINEBEASTSARENA_API UDBAPlayableSkillDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 项目级默认可玩技能目录。组件未单独配置时会异步加载该数据资产。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|Playable Skill")
	TSoftObjectPtr<UDBAPlayableSkillCatalogDataAsset> DefaultSkillCatalog;
};
