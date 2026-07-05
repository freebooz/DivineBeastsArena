// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：验证 ArenaGame 固定技能组数据表行名与身份字段遵守 Zodiac + Element -> FixedSkillGroupId 契约。
- 阅读重点：这些测试不加载 .uasset，只校验源码层数据行和查询行名规则。
- 修改提示：如调整固定技能组命名，请同步后端 CharacterBuildRules、GameCore DBACharacterBuild 与 Dedicated Server URL admission。
*/

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/Data/DBAFixedSkillGroupData.h"
#include "GameDBA/Data/DBAZodiacHeroDataAsset.h"
#include "GameDBA/Services/DBASkillGroupGeneratorSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAFixedSkillGroupDataCanonicalRowNameTest,
	"DivineBeastsArena.GameDBA.Data.FixedSkillGroup.UsesCanonicalBuildSummaryRowName",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAFixedSkillGroupDataCanonicalRowNameTest::RunTest(const FString& Parameters)
{
	TestEqual(
		TEXT("数据资产固定技能组行名应匹配后端和运行时 FixedSkillGroupId"),
		UDBAZodiacHeroDataAsset::BuildFixedSkillGroupRowName(EDBAZodiac::Rat, EDBAElement::Water),
		FName(TEXT("Rat_Water")));

	TestEqual(
		TEXT("蛇与金元素应使用专用服务器准入一致的规范身份"),
		UDBAZodiacHeroDataAsset::BuildFixedSkillGroupRowName(EDBAZodiac::Snake, EDBAElement::Gold),
		FName(TEXT("Snake_Gold")));

	TestEqual(
		TEXT("缺失身份维度不应生成有效数据表行名"),
		UDBAZodiacHeroDataAsset::BuildFixedSkillGroupRowName(EDBAZodiac::None, EDBAElement::Water),
		NAME_None);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAFixedSkillGroupDataRowIdentityTest,
	"DivineBeastsArena.GameDBA.Data.FixedSkillGroup.ValidatesRowIdentity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAFixedSkillGroupDataRowIdentityTest::RunTest(const FString& Parameters)
{
	FDBAZodiacElementFixedSkillGroupRow ValidRow;
	ValidRow.RowId = FName(TEXT("Tiger_Fire"));
	ValidRow.ZodiacType = EDBAZodiac::Tiger;
	ValidRow.ElementType = EDBAElement::Fire;

	TestTrue(TEXT("匹配的行身份应被接受"), ValidRow.HasValidIdentity());

	FDBAZodiacElementFixedSkillGroupRow TamperedRow = ValidRow;
	TamperedRow.RowId = FName(TEXT("Tiger_Water"));
	TestFalse(TEXT("不匹配的行标识应被拒绝"), TamperedRow.HasValidIdentity());

	FDBAZodiacElementFixedSkillGroupRow MissingElementRow = ValidRow;
	MissingElementRow.ElementType = EDBAElement::None;
	TestFalse(TEXT("缺失元素应被拒绝"), MissingElementRow.HasValidIdentity());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAFixedSkillGroupGeneratorRejectsInvalidIdentityTest,
	"DivineBeastsArena.GameDBA.Data.FixedSkillGroup.GeneratorRejectsInvalidIdentityDimensions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAFixedSkillGroupGeneratorRejectsInvalidIdentityTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDBASkillGroupGeneratorSubsystem* Generator = NewObject<UDBASkillGroupGeneratorSubsystem>(GameInstance);
	TestNotNull(TEXT("技能组生成器应能被构造用于校验"), Generator);

	FDBAZodiacElementFixedSkillGroupRow SkillGroup;
	TestFalse(
		TEXT("缺失生肖不得生成兜底固定技能组"),
		Generator->GetSkillGroup(EDBAZodiac::None, EDBAElement::Water, SkillGroup));

	TestFalse(
		TEXT("缺失元素不得生成兜底固定技能组"),
		Generator->GetSkillGroup(EDBAZodiac::Rat, EDBAElement::None, SkillGroup));

	TestFalse(
		TEXT("非法身份维度不得被报告为已配置"),
		Generator->IsSkillGroupConfigured(EDBAZodiac::None, EDBAElement::Water));

	FDBAZodiacHeroAbilitySetSummaryRow Summary;
	TestFalse(
		TEXT("缺失生肖不得生成技能组摘要"),
		Generator->GetSkillGroupSummary(EDBAZodiac::None, Summary));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAFixedSkillGroupGeneratorFallbackIdentityTest,
	"DivineBeastsArena.GameDBA.Data.FixedSkillGroup.GeneratorFallbackUsesCanonicalIdentity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAFixedSkillGroupGeneratorFallbackIdentityTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UDBASkillGroupGeneratorSubsystem* Generator = NewObject<UDBASkillGroupGeneratorSubsystem>(GameInstance);
	TestNotNull(TEXT("技能组生成器应能被构造用于校验"), Generator);

	FDBAZodiacElementFixedSkillGroupRow SkillGroup;
	TestTrue(
		TEXT("数据表不可用时有效身份维度应生成兜底固定技能组"),
		Generator->GetSkillGroup(EDBAZodiac::Rat, EDBAElement::Water, SkillGroup));
	TestEqual(TEXT("兜底行标识应使用规范 FixedSkillGroupId"), SkillGroup.RowId, FName(TEXT("Rat_Water")));
	TestTrue(TEXT("兜底固定技能组应满足行身份校验"), SkillGroup.HasValidIdentity());

	FDBAZodiacHeroAbilitySetSummaryRow Summary;
	TestTrue(TEXT("有效生肖应生成兜底摘要行标识"), Generator->GetSkillGroupSummary(EDBAZodiac::Rat, Summary));
	TestEqual(TEXT("摘要水元素行标识应使用规范 FixedSkillGroupId"), Summary.WaterSkillGroupRowId, FName(TEXT("Rat_Water")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAFixedSkillGroupAssetRowsTest,
	"DivineBeastsArena.GameDBA.Data.FixedSkillGroup.AssetRows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAFixedSkillGroupAssetRowsTest::RunTest(const FString& Parameters)
{
	static constexpr const TCHAR* PackagePath = TEXT("/Game/DBA/Data/Tables/DT_FixedSkillGroups");
	static constexpr const TCHAR* ObjectPath = TEXT("/Game/DBA/Data/Tables/DT_FixedSkillGroups.DT_FixedSkillGroups");

	if (!TestTrue(TEXT("固定技能组数据表包应存在用于发布校验"), FPackageName::DoesPackageExist(PackagePath)))
	{
		AddError(FString::Printf(TEXT("缺少必需的固定技能组数据表资产：%s"), PackagePath));
		return false;
	}

	UDataTable* SkillGroupTable = LoadObject<UDataTable>(nullptr, ObjectPath);
	if (!TestNotNull(TEXT("固定技能组数据表应能加载"), SkillGroupTable))
	{
		return false;
	}

	TestTrue(
		TEXT("固定技能组数据表应使用固定技能组行结构"),
		SkillGroupTable->GetRowStruct() == FDBAZodiacElementFixedSkillGroupRow::StaticStruct());

	const TArray<EDBAZodiac> Zodiacs = {
		EDBAZodiac::Rat,
		EDBAZodiac::Ox,
		EDBAZodiac::Tiger,
		EDBAZodiac::Rabbit,
		EDBAZodiac::Dragon,
		EDBAZodiac::Snake,
		EDBAZodiac::Horse,
		EDBAZodiac::Goat,
		EDBAZodiac::Monkey,
		EDBAZodiac::Rooster,
		EDBAZodiac::Dog,
		EDBAZodiac::Pig
	};

	const TArray<EDBAElement> Elements = {
		EDBAElement::Gold,
		EDBAElement::Wood,
		EDBAElement::Water,
		EDBAElement::Fire,
		EDBAElement::Earth
	};

	TestEqual(TEXT("固定技能组测试应覆盖全部生肖维度"), Zodiacs.Num(), DBAConstants::ZodiacCount);
	TestEqual(TEXT("固定技能组测试应覆盖全部元素维度"), Elements.Num(), DBAConstants::ElementCount);
	TestEqual(TEXT("固定技能组数据表应包含 60 条生肖乘元素行"), SkillGroupTable->GetRowNames().Num(), DBAConstants::FixedSkillGroupRowCount);

	for (const EDBAZodiac Zodiac : Zodiacs)
	{
		for (const EDBAElement Element : Elements)
		{
			const FName ExpectedRowId = UDBAZodiacHeroDataAsset::BuildFixedSkillGroupRowName(Zodiac, Element);
			const FDBAZodiacElementFixedSkillGroupRow* Row = SkillGroupTable->FindRow<FDBAZodiacElementFixedSkillGroupRow>(ExpectedRowId, TEXT("FixedSkillGroupAssetRows"));
			if (!TestNotNull(FString::Printf(TEXT("数据表行应存在：%s"), *ExpectedRowId.ToString()), Row))
			{
				continue;
			}

			TestEqual(FString::Printf(TEXT("行标识应匹配行名：%s"), *ExpectedRowId.ToString()), Row->RowId, ExpectedRowId);
			TestTrue(FString::Printf(TEXT("行身份应有效：%s"), *ExpectedRowId.ToString()), Row->HasValidIdentity());
		}
	}

	return true;
}

#endif
