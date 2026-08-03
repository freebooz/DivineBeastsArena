// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 文件职责：验证固定技能组数据行的最小完整性和数据表可用性。
- 注意：RowId 与玩法身份的关系以 Arena 数据表为权威，C++ 不拼接规则。
*/

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Data/Tables/DBAFixedSkillGroupData.h"
#include "GameDBA/Gameplay/Loadout/SkillGroups/DBASkillGroupDeveloperSettings.h"
#include "GameDBA/Gameplay/Loadout/SkillGroups/DBASkillGroupGeneratorSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAFixedSkillGroupDataRowIdentityTest,
	"DivineBeastsArena.GameDBA.Data.FixedSkillGroup.ValidatesRequiredIdentityFields",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAFixedSkillGroupDataRowIdentityTest::RunTest(const FString& Parameters)
{
	FDBAZodiacElementFixedSkillGroupRow ValidRow;
	ValidRow.RowId = FName(TEXT("由数据表配置"));
	ValidRow.ZodiacType = EDBAZodiac::Tiger;
	ValidRow.ElementType = EDBAElement::Fire;
	TestTrue(TEXT("完整数据行应被接受"), ValidRow.HasValidIdentity());

	ValidRow.RowId = NAME_None;
	TestFalse(TEXT("缺少行标识的数据行应被拒绝"), ValidRow.HasValidIdentity());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAFixedSkillGroupGeneratorRejectsInvalidIdentityTest,
	"DivineBeastsArena.GameDBA.Data.FixedSkillGroup.GeneratorRejectsMissingIdentity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAFixedSkillGroupGeneratorRejectsInvalidIdentityTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDBASkillGroupGeneratorSubsystem* Generator = NewObject<UDBASkillGroupGeneratorSubsystem>(GameInstance);
	TestNotNull(TEXT("技能组生成器应能被构造用于校验"), Generator);

	FDBAZodiacElementFixedSkillGroupRow SkillGroup;
	TestFalse(TEXT("缺少生肖不得被报告为已配置"), Generator->GetSkillGroup(EDBAZodiac::None, EDBAElement::Water, SkillGroup));
	TestFalse(TEXT("缺少元素不得被报告为已配置"), Generator->GetSkillGroup(EDBAZodiac::Rat, EDBAElement::None, SkillGroup));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAFixedSkillGroupAssetRowsTest,
	"DivineBeastsArena.GameDBA.Data.FixedSkillGroup.AssetRowsHaveCompleteIdentity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAFixedSkillGroupAssetRowsTest::RunTest(const FString& Parameters)
{
	const UDBASkillGroupDeveloperSettings* Settings = GetDefault<UDBASkillGroupDeveloperSettings>();
	if (!TestNotNull(TEXT("固定技能组开发者设置必须存在"), Settings)
		|| !TestFalse(TEXT("固定技能组主表软引用必须配置"), Settings->DefaultSkillGroupDataTable.IsNull()))
	{
		return false;
	}

	UDataTable* SkillGroupTable = Cast<UDataTable>(Settings->DefaultSkillGroupDataTable.ToSoftObjectPath().TryLoad());
	if (!TestNotNull(TEXT("固定技能组数据表应能加载"), SkillGroupTable))
	{
		return false;
	}

	TArray<FDBAZodiacElementFixedSkillGroupRow*> Rows;
	SkillGroupTable->GetAllRows(TEXT("固定技能组数据行完整性"), Rows);
	TestTrue(TEXT("固定技能组数据表至少应包含一行"), Rows.Num() > 0);
	for (const FDBAZodiacElementFixedSkillGroupRow* Row : Rows)
	{
		TestTrue(TEXT("每个固定技能组数据行必须包含完整身份字段"), Row && Row->HasValidIdentity());
	}

	return true;
}

#endif
