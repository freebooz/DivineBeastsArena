// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明角色构筑身份的跨层传输契约。
- 阅读重点：本类型只传递稳定标识符，不推导生肖、元素、阵营或技能组规则。
- 修改提示：本文件属于 GameCore，不得引入具体玩法枚举、技能规则、UI 资源或 DivineBeastsArena 项目层类型。
*/

#pragma once

#include "CoreMinimal.h"
#include "DBACharacterBuildTypes.generated.h"

/**
 * 角色构筑身份摘要。
 *
 * 该摘要只承载跨流程、跨服务可传递的稳定标识符。所有标识符与固定技能组的
 * 对应关系必须由 Arena 数据资产验证；GameCore 不得拼接、推导或维护玩法规则。
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBACharacterBuildSummary
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Character Build")
	FName ZodiacId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Character Build")
	FName PrimaryElementId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Character Build")
	FName FiveCampId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Character Build")
	FName FixedSkillGroupId = NAME_None;

	bool IsValid() const
	{
		return !ZodiacId.IsNone()
			&& !PrimaryElementId.IsNone()
			&& !FiveCampId.IsNone()
			&& !FixedSkillGroupId.IsNone();
	}
};

namespace DBACharacterBuild
{
	/** 仅组装中性传输契约，不校验玩法规则。 */
	GAMECORE_API FDBACharacterBuildSummary MakeBuildSummary(
		FName ZodiacId,
		FName PrimaryElementId,
		FName FiveCampId,
		FName FixedSkillGroupId);
}
