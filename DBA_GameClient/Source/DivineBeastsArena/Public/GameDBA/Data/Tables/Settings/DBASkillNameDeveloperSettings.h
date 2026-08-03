// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明技能名称 DeveloperSettings，通过 DefaultGame.ini 暴露 DataTable 软引用配置入口。
- 阅读重点：TSoftObjectPtr 配置字段，设计师可在 Project Settings 中配置。
- 修改提示：新增配置项时同步更新 DefaultGame.ini 与 Subsystem 初始化逻辑。
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DBASkillNameDeveloperSettings.generated.h"

class UDataTable;

/**
 * 技能名称 DeveloperSettings
 *
 * 用途：
 * - 通过 DefaultGame.ini 配置技能名称 DataTable 软引用
 * - 设计师可在 Project Settings > DBA 技能名称配置 中指定 DataTable
 *
 * 配置示例（DefaultGame.ini）：
 * [/Script/DivineBeastsArena.DBASkillNameDeveloperSettings]
 * DefaultSkillNameTable=/Game/DBA/Data/Tables/DT_SkillNames.DT_SkillNames
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="DBA 技能名称配置"))
class DIVINEBEASTSARENA_API UDBASkillNameDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 默认技能名称 DataTable 软引用（指向 DT_SkillNames 资产） */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "SkillName", meta = (AllowedClasses = "/Script/Engine.DataTable"))
	TSoftObjectPtr<UDataTable> DefaultSkillNameTable;
};
