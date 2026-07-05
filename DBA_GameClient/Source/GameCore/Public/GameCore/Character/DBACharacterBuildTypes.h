// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明角色创建冻结摘要的基础层数据契约。
- 阅读重点：Zodiac / Element 决定 FixedSkillGroupId；FiveCamp 仅作为表现包选择保留。
- 修改提示：本文件属于 GameCore，不得引入具体生肖技能、元素克制表、UI 资源或 DivineBeastsArena 项目层类型。
*/

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "DBACharacterBuildTypes.generated.h"

/**
 * 角色创建构建摘要
 *
 * 该摘要只承载跨流程、跨服务可传递的身份维度结果。
 * 具体技能、属性、资源和表现包加载仍由 ArenaGame / Dedicated Server 按数据表验证。
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBACharacterBuildSummary
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Character Build")
	EDBAZodiac Zodiac = EDBAZodiac::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Character Build")
	EDBAElement PrimaryElement = EDBAElement::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Character Build")
	EDBAFiveCamp FiveCamp = EDBAFiveCamp::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Character Build")
	FName FixedSkillGroupId = NAME_None;

	bool IsValid() const
	{
		return Zodiac != EDBAZodiac::None
			&& PrimaryElement != EDBAElement::None
			&& FiveCamp != EDBAFiveCamp::None
			&& !FixedSkillGroupId.IsNone();
	}
};

namespace DBACharacterBuild
{
	GAMECORE_API const TCHAR* ToStableZodiacName(EDBAZodiac Zodiac);
	GAMECORE_API const TCHAR* ToStableElementName(EDBAElement Element);
	GAMECORE_API FName MakeFixedSkillGroupId(EDBAZodiac Zodiac, EDBAElement Element);
	GAMECORE_API EDBAFiveCamp ResolveFiveCamp(EDBAFiveCamp RequestedFiveCamp, EDBAElement Element);
	GAMECORE_API FDBACharacterBuildSummary MakeBuildSummary(
		EDBAZodiac Zodiac,
		EDBAElement PrimaryElement,
		EDBAFiveCamp RequestedFiveCamp);
}
