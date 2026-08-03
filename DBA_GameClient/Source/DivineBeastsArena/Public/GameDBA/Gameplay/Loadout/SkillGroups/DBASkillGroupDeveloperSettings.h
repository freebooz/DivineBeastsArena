// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：固定技能组数据表的项目级配置入口。
- 阅读重点：主表为必填软引用，摘要表为可选软引用；运行时由子系统异步加载。
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DBASkillGroupDeveloperSettings.generated.h"

class UDataTable;

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="DBA 固定技能组配置"))
class DIVINEBEASTSARENA_API UDBASkillGroupDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 生肖乘元素固定技能组主表，必须配置。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|SkillGroup")
	TSoftObjectPtr<UDataTable> DefaultSkillGroupDataTable;

	/** 生肖技能组摘要表，可选；未配置时由主表生成摘要。 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DBA|SkillGroup")
	TSoftObjectPtr<UDataTable> DefaultSkillGroupSummaryDataTable;
};
