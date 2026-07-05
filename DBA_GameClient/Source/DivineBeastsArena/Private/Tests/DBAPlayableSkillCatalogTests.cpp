// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Combat/DBAPlayableSkillComponent.h"
#include "GameDBA/Combat/DBAPlayableSkillCatalogDataAsset.h"
#include "GameDBA/Combat/DBAPlayableSkillTypes.h"
#include "GameDBA/Combat/DBABloomHealingSpell.h"
#include "GameDBA/Combat/DBAChainLightningSpell.h"
#include "GameDBA/Combat/DBAFireballProjectile.h"
#include "GameDBA/Combat/DBAHolyShieldSpell.h"
#include "GameDBA/Combat/DBASkillProjectileBase.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAPlayableSkillCatalogDefaultsTest,
	"DivineBeastsArena.Combat.PlayableSkillCatalog.Defaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAPlayableSkillCatalogDefaultsTest::RunTest(const FString& Parameters)
{
	UDBAPlayableSkillComponent* SkillComponent = NewObject<UDBAPlayableSkillComponent>();
	TestNotNull(TEXT("技能组件应能创建"), SkillComponent);
	if (!SkillComponent)
	{
		return false;
	}

	const TArray<FDBAPlayableSkillRuntimeSpec> SkillSpecs = SkillComponent->GetAllSkillSpecs();
	TestEqual(TEXT("未配置默认技能目录时不应生成 C++ 内置技能"), SkillSpecs.Num(), 0);

	TArray<FString> ValidationErrors;
	TestFalse(TEXT("未配置默认技能目录时应报告无可用技能"), SkillComponent->ValidateEffectiveSkillSpecs(ValidationErrors));
	TestTrue(TEXT("未配置默认技能目录时应输出中文校验错误"), ValidationErrors.Contains(TEXT("SkillSpecs 为空")));

	const FDBAPlayableSkillCatalogSummary DefaultSummary = SkillComponent->GetSkillCatalogSummary();
	TestEqual(TEXT("未配置摘要来源应为内置默认占位"), DefaultSummary.Source, EDBAPlayableSkillCatalogSource::BuiltInDefaults);
	TestEqual(TEXT("未配置摘要目录标识应为空"), DefaultSummary.CatalogId, NAME_None);
	TestEqual(TEXT("未配置摘要技能数量应为零"), DefaultSummary.SkillCount, 0);
	TestEqual(TEXT("默认摘要已配置目录技能数量应为零"), DefaultSummary.ConfiguredCatalogSkillCount, 0);
	TestFalse(TEXT("未配置摘要不应有效"), DefaultSummary.bIsValid);
	TestFalse(TEXT("未配置摘要不应追加默认技能"), DefaultSummary.bAppendsBuiltInDefaults);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAPlayableSkillCatalogValidationTest,
	"DivineBeastsArena.Combat.PlayableSkillCatalog.Validation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAPlayableSkillCatalogValidationTest::RunTest(const FString& Parameters)
{
	UDBAPlayableSkillCatalogDataAsset* Catalog = NewObject<UDBAPlayableSkillCatalogDataAsset>();
	TestNotNull(TEXT("技能目录数据资产应能创建"), Catalog);
	if (!Catalog)
	{
		return false;
	}

	Catalog->CatalogId = NAME_None;

	FDBAPlayableSkillRuntimeSpec BrokenProjectile;
	BrokenProjectile.SkillSlot = 1;
	BrokenProjectile.EffectShape = EDBAPlayableSkillEffectShape::Projectile;

	FDBAPlayableSkillRuntimeSpec DuplicateSlot;
	DuplicateSlot.SkillSlot = 1;
	DuplicateSlot.SkillId = TEXT("Test.DuplicateSlot");
	DuplicateSlot.DisplayName = FText::FromString(TEXT("重复槽位"));
	DuplicateSlot.EffectShape = EDBAPlayableSkillEffectShape::HolyShield;
	DuplicateSlot.Magnitude = 10.0f;
	DuplicateSlot.Cooldown = 1.0f;
	DuplicateSlot.CastVFXScale = 1.0f;
	DuplicateSlot.HolyShieldClass = ADBAHolyShieldSpell::StaticClass();

	Catalog->SkillSpecs.Add(BrokenProjectile);
	Catalog->SkillSpecs.Add(DuplicateSlot);

	TArray<FString> ValidationErrors;
	TestFalse(TEXT("损坏技能目录应无法通过校验"), Catalog->ValidateDataIntegrity(ValidationErrors));
	TestTrue(TEXT("损坏技能目录应报告错误"), ValidationErrors.Num() > 0);

	const auto HasErrorContaining = [&ValidationErrors](const TCHAR* ExpectedText)
	{
		return ValidationErrors.ContainsByPredicate([ExpectedText](const FString& Error)
		{
			return Error.Contains(ExpectedText);
		});
	};

	TestTrue(TEXT("缺少目录标识应被报告"), HasErrorContaining(TEXT("CatalogId")));
	TestTrue(TEXT("缺少投射物类应被报告"), HasErrorContaining(TEXT("ProjectileClass")));
	TestTrue(TEXT("重复技能槽应被报告"), HasErrorContaining(TEXT("SkillSlot 重复")));
	TestTrue(TEXT("缺少施放特效应被报告"), HasErrorContaining(TEXT("CastNiagaraVFXAsset")));
	TestTrue(TEXT("缺少施放音效应被报告"), HasErrorContaining(TEXT("CastSFXAsset")));
	TestTrue(TEXT("缺少命中特效应被报告"), HasErrorContaining(TEXT("ImpactNiagaraVFXAsset")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAPlayableSkillCatalogDataAssetOverrideTest,
	"DivineBeastsArena.Combat.PlayableSkillCatalog.DataAssetOverride",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAPlayableSkillCatalogDataAssetOverrideTest::RunTest(const FString& Parameters)
{
	UDBAPlayableSkillCatalogDataAsset* Catalog = NewObject<UDBAPlayableSkillCatalogDataAsset>();
	TestNotNull(TEXT("技能目录数据资产应能创建"), Catalog);
	if (!Catalog)
	{
		return false;
	}

	FDBAPlayableSkillRuntimeSpec InvalidSpec;
	InvalidSpec.SkillSlot = 0;
	InvalidSpec.SkillId = TEXT("Test.Invalid");

	FDBAPlayableSkillRuntimeSpec OverrideSpec;
	OverrideSpec.SkillSlot = 1;
	OverrideSpec.SkillId = TEXT("Test.Skill.OverrideFireball");
	OverrideSpec.DisplayName = FText::FromString(TEXT("覆盖炎弹"));
	OverrideSpec.EffectShape = EDBAPlayableSkillEffectShape::Projectile;
	OverrideSpec.Element = EDBAElement::Fire;
	OverrideSpec.Magnitude = 99.0f;
	OverrideSpec.ProjectileSpeed = 2000.0f;
	OverrideSpec.ProjectileRadius = 64.0f;
	OverrideSpec.Cooldown = 9.5f;
	OverrideSpec.CastVFXScale = 1.4f;
	OverrideSpec.ProjectileClass = ADBAFireballProjectile::StaticClass();

	Catalog->SkillSpecs.Reset();
	Catalog->SkillSpecs.Add(InvalidSpec);
	Catalog->SkillSpecs.Add(OverrideSpec);

	const TArray<FDBAPlayableSkillRuntimeSpec> CatalogSpecs = Catalog->GetAllSkillSpecs();
	TestEqual(TEXT("技能目录应忽略无效槽位"), CatalogSpecs.Num(), 1);
	TestEqual(TEXT("技能目录覆盖槽位应匹配"), CatalogSpecs[0].SkillSlot, 1);

	UDBAPlayableSkillComponent* SkillComponent = NewObject<UDBAPlayableSkillComponent>();
	TestNotNull(TEXT("技能组件应能创建"), SkillComponent);
	if (!SkillComponent)
	{
		return false;
	}

	SkillComponent->SetSkillCatalog(Catalog);

	const TArray<FDBAPlayableSkillRuntimeSpec> EffectiveSpecs = SkillComponent->GetAllSkillSpecs();
	TestEqual(TEXT("技能目录覆盖不应追加 C++ 内置默认技能"), EffectiveSpecs.Num(), 1);

	FDBAPlayableSkillCatalogSummary OverrideSummary = SkillComponent->GetSkillCatalogSummary();
	TestEqual(TEXT("覆盖摘要来源应为纯数据资产"), OverrideSummary.Source, EDBAPlayableSkillCatalogSource::DataAssetOnly);
	TestEqual(TEXT("覆盖摘要目录标识应匹配"), OverrideSummary.CatalogId, FName(TEXT("DefaultPlayableSkillCatalog")));
	TestEqual(TEXT("覆盖摘要技能数量应匹配"), OverrideSummary.SkillCount, 1);
	TestEqual(TEXT("覆盖摘要已配置目录技能数量应匹配"), OverrideSummary.ConfiguredCatalogSkillCount, 1);
	TestFalse(TEXT("覆盖摘要不应追加 C++ 内置默认技能"), OverrideSummary.bAppendsBuiltInDefaults);

	const auto FindSkill = [](const TArray<FDBAPlayableSkillRuntimeSpec>& SkillSpecs, int32 SkillSlot) -> const FDBAPlayableSkillRuntimeSpec*
	{
		return SkillSpecs.FindByPredicate([SkillSlot](const FDBAPlayableSkillRuntimeSpec& Spec)
		{
			return Spec.SkillSlot == SkillSlot;
		});
	};

	const FDBAPlayableSkillRuntimeSpec* SlotOne = FindSkill(EffectiveSpecs, 1);
	TestNotNull(TEXT("槽位 1 覆盖技能应存在"), SlotOne);
	if (SlotOne)
	{
		TestEqual(TEXT("槽位 1 覆盖技能标识应匹配"), SlotOne->SkillId, FName(TEXT("Test.Skill.OverrideFireball")));
		TestEqual(TEXT("槽位 1 覆盖技能名称应匹配"), SlotOne->DisplayName.ToString(), FString(TEXT("覆盖炎弹")));
		TestEqual(TEXT("槽位 1 覆盖技能强度应匹配"), SlotOne->Magnitude, 99.0f);
		TestEqual(TEXT("槽位 1 覆盖技能冷却应匹配"), SlotOne->Cooldown, 9.5f);
		TestTrue(TEXT("槽位 1 覆盖技能应保留投射物类"), SlotOne->ProjectileClass == ADBAFireballProjectile::StaticClass());
	}

	const FDBAPlayableSkillRuntimeSpec* SlotTwo = FindSkill(EffectiveSpecs, 2);
	TestNull(TEXT("未配置槽位 2 时不应生成 C++ 默认兜底技能"), SlotTwo);

	SkillComponent->SetAppendDefaultSkillsWhenCatalogMissingSlots(false);
	const TArray<FDBAPlayableSkillRuntimeSpec> CatalogOnlySpecs = SkillComponent->GetAllSkillSpecs();
	TestEqual(TEXT("仅目录模式不应追加默认技能"), CatalogOnlySpecs.Num(), 1);

	const FDBAPlayableSkillCatalogSummary CatalogOnlySummary = SkillComponent->GetSkillCatalogSummary();
	TestEqual(TEXT("仅目录摘要来源应为纯数据资产"), CatalogOnlySummary.Source, EDBAPlayableSkillCatalogSource::DataAssetOnly);
	TestEqual(TEXT("仅目录摘要技能数量应匹配"), CatalogOnlySummary.SkillCount, 1);
	TestFalse(TEXT("仅目录摘要不应追加默认技能"), CatalogOnlySummary.bAppendsBuiltInDefaults);

	FDBAPlayableSkillRuntimeSpec MissingSpec;
	TestFalse(TEXT("仅目录模式不应存在槽位 2"), SkillComponent->GetSkillSpec(2, MissingSpec));

	return true;
}

#endif
