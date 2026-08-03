// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：实现角色构筑身份的中性传输契约。
- 阅读重点：本文件不包含生肖、元素、阵营或技能组推导规则。
- 修改提示：玩法身份的合法性与资源映射由 Arena 数据资产负责。
*/

#include "GameCore/Types/DBACharacterBuildTypes.h"

namespace DBACharacterBuild
{
FDBACharacterBuildSummary MakeBuildSummary(
	const FName ZodiacId,
	const FName PrimaryElementId,
	const FName FiveCampId,
	const FName FixedSkillGroupId)
{
	FDBACharacterBuildSummary Summary;
	Summary.ZodiacId = ZodiacId;
	Summary.PrimaryElementId = PrimaryElementId;
	Summary.FiveCampId = FiveCampId;
	Summary.FixedSkillGroupId = FixedSkillGroupId;
	return Summary;
}
}
