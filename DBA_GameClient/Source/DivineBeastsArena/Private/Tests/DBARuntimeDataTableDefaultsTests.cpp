// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Data/Tables/Settings/DBASkillNameDeveloperSettings.h"
#include "GameDBA/Data/Tables/DBASkillNameRow.h"
#include "GameDBA/Data/Tables/DBAFixedSkillGroupData.h"
#include "GameDBA/Gameplay/Progression/Balance/DBAHeroBalanceDeveloperSettings.h"
#include "GameDBA/Gameplay/Progression/Balance/DBAHeroBalanceRow.h"
#include "GameDBA/Gameplay/Loadout/SkillGroups/DBASkillGroupDeveloperSettings.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"

namespace
{
	UDataTable* LoadConfiguredDataTable(const TSoftObjectPtr<UDataTable>& TableReference)
	{
		return Cast<UDataTable>(TableReference.ToSoftObjectPath().TryLoad());
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBARuntimeDataTableDefaultsTest,
	"DivineBeastsArena.Data.RuntimeDefaults.HeroBalanceAndSkillNames",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBARuntimeDataTableDefaultsTest::RunTest(const FString& Parameters)
{
	const UDBAHeroBalanceDeveloperSettings* HeroBalanceSettings = GetDefault<UDBAHeroBalanceDeveloperSettings>();
	TestNotNull(TEXT("英雄平衡开发者设置必须存在"), HeroBalanceSettings);
	if (!HeroBalanceSettings)
	{
		return false;
	}

	TestFalse(TEXT("英雄平衡默认表软引用必须配置"), HeroBalanceSettings->DefaultHeroBalanceTable.IsNull());
	UDataTable* HeroBalanceTable = LoadConfiguredDataTable(HeroBalanceSettings->DefaultHeroBalanceTable);
	TestNotNull(TEXT("英雄平衡默认表资产必须可加载"), HeroBalanceTable);
	if (!HeroBalanceTable)
	{
		return false;
	}

	TestEqual(TEXT("英雄平衡表必须包含十二条生肖记录"), HeroBalanceTable->GetRowNames().Num(), 12);
	TestNotNull(TEXT("英雄平衡表必须包含鼠生肖记录"), HeroBalanceTable->FindRow<FDBAHeroBalanceRow>(TEXT("Rat"), TEXT("运行默认数据表测试")));

	const UDBASkillNameDeveloperSettings* SkillNameSettings = GetDefault<UDBASkillNameDeveloperSettings>();
	TestNotNull(TEXT("技能名称开发者设置必须存在"), SkillNameSettings);
	if (!SkillNameSettings)
	{
		return false;
	}

	TestFalse(TEXT("技能名称默认表软引用必须配置"), SkillNameSettings->DefaultSkillNameTable.IsNull());
	UDataTable* SkillNameTable = LoadConfiguredDataTable(SkillNameSettings->DefaultSkillNameTable);
	TestNotNull(TEXT("技能名称默认表资产必须可加载"), SkillNameTable);
	if (!SkillNameTable)
	{
		return false;
	}

	TestEqual(TEXT("技能名称表必须包含十二生肖乘六槽位记录"), SkillNameTable->GetRowNames().Num(), 72);
	TestNotNull(TEXT("技能名称表必须包含鼠生肖被动技能记录"), SkillNameTable->FindRow<FDBASkillNameRow>(TEXT("Rat_Passive"), TEXT("运行默认数据表测试")));
	TestNotNull(TEXT("技能名称表必须包含猪生肖终极技能记录"), SkillNameTable->FindRow<FDBASkillNameRow>(TEXT("Pig_Ultimate"), TEXT("运行默认数据表测试")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBASkillGroupDataTableDefaultTest,
	"DivineBeastsArena.Data.RuntimeDefaults.FixedSkillGroups",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBASkillGroupDataTableDefaultTest::RunTest(const FString& Parameters)
{
	const UDBASkillGroupDeveloperSettings* SkillGroupSettings = GetDefault<UDBASkillGroupDeveloperSettings>();
	TestNotNull(TEXT("固定技能组开发者设置必须存在"), SkillGroupSettings);
	if (!SkillGroupSettings)
	{
		return false;
	}

	TestFalse(TEXT("固定技能组主表软引用必须配置"), SkillGroupSettings->DefaultSkillGroupDataTable.IsNull());
	UDataTable* SkillGroupTable = LoadConfiguredDataTable(SkillGroupSettings->DefaultSkillGroupDataTable);
	TestNotNull(TEXT("固定技能组主表资产必须可加载"), SkillGroupTable);
	if (!SkillGroupTable)
	{
		return false;
	}

	TestEqual(TEXT("固定技能组主表必须包含六十条生肖乘元素记录"), SkillGroupTable->GetRowNames().Num(), 60);
	TestNotNull(TEXT("固定技能组主表必须包含鼠水组合"), SkillGroupTable->FindRow<FDBAZodiacElementFixedSkillGroupRow>(TEXT("Rat_Water"), TEXT("运行默认数据表测试")));
	TestNotNull(TEXT("固定技能组主表必须包含猪土组合"), SkillGroupTable->FindRow<FDBAZodiacElementFixedSkillGroupRow>(TEXT("Pig_Earth"), TEXT("运行默认数据表测试")));

	return true;
}

#endif
