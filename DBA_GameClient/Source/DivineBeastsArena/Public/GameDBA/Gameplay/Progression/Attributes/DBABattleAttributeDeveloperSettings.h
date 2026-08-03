// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明战斗属性默认值的数据资产软引用配置入口。
- 修改提示：这里只保存软引用，不承载战斗逻辑或运行数值。
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBABattleAttributeDefaultsDataAsset.h"
#include "DBABattleAttributeDeveloperSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="DBA 战斗属性设置"))
class DIVINEBEASTSARENA_API UDBABattleAttributeDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 项目级默认战斗属性数据资产。PlayerState 创建 AttributeSet 后会异步加载并应用。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="DBA|Battle Attribute")
	TSoftObjectPtr<UDBABattleAttributeDefaultsDataAsset> DefaultBattleAttributeDefaults;
};
