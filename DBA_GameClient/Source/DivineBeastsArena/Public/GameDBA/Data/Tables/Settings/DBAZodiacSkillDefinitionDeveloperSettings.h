// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明万象灵庭 V2 技能定义 DataTable 的项目级开发配置入口。
- 修改提示：仅保存软引用，不承载战斗逻辑；运行时 Subsystem 接入见后续 P1 任务。
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DBAZodiacSkillDefinitionDeveloperSettings.generated.h"

class UDataTable;

/**
 * 万象灵庭 V2 生肖战术技能定义 DataTable 配置
 *
 * DefaultGame.ini 示例：
 * [/Script/DivineBeastsArena.DBAZodiacSkillDefinitionDeveloperSettings]
 * DefaultZodiacSkillDefinitionTable=/Game/DBA/Data/Tables/DT_ZodiacSkillDefinitions.DT_ZodiacSkillDefinitions
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="DBA 生肖战术技能定义 (V2)"))
class DIVINEBEASTSARENA_API UDBAZodiacSkillDefinitionDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** V2 生肖战术技能定义 DataTable 软引用（DT_ZodiacSkillDefinitions） */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|V2", meta = (AllowedClasses = "/Script/Engine.DataTable"))
	TSoftObjectPtr<UDataTable> DefaultZodiacSkillDefinitionTable;
};
