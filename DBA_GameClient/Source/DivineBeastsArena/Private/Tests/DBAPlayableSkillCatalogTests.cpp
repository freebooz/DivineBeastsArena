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
	TestNotNull(TEXT("SkillComponent"), SkillComponent);
	if (!SkillComponent)
	{
		return false;
	}

	const TArray<FDBAPlayableSkillRuntimeSpec> SkillSpecs = SkillComponent->GetAllSkillSpecs();
	TestEqual(TEXT("Default skill count"), SkillSpecs.Num(), 6);

	TArray<FString> ValidationErrors;
	TestTrue(TEXT("Default skill catalog validates"), SkillComponent->ValidateEffectiveSkillSpecs(ValidationErrors));
	TestEqual(TEXT("Default skill catalog validation error count"), ValidationErrors.Num(), 0);

	const FDBAPlayableSkillCatalogSummary DefaultSummary = SkillComponent->GetSkillCatalogSummary();
	TestEqual(TEXT("Default summary source"), DefaultSummary.Source, EDBAPlayableSkillCatalogSource::BuiltInDefaults);
	TestEqual(TEXT("Default summary catalog id"), DefaultSummary.CatalogId, FName(TEXT("BuiltInDefaults")));
	TestEqual(TEXT("Default summary skill count"), DefaultSummary.SkillCount, 6);
	TestEqual(TEXT("Default summary configured catalog count"), DefaultSummary.ConfiguredCatalogSkillCount, 0);
	TestTrue(TEXT("Default summary is valid"), DefaultSummary.bIsValid);
	TestTrue(TEXT("Default summary appends defaults"), DefaultSummary.bAppendsBuiltInDefaults);

	const auto FindSkill = [&SkillSpecs](int32 SkillSlot) -> const FDBAPlayableSkillRuntimeSpec*
	{
		return SkillSpecs.FindByPredicate([SkillSlot](const FDBAPlayableSkillRuntimeSpec& Spec)
		{
			return Spec.SkillSlot == SkillSlot;
		});
	};

	const FDBAPlayableSkillRuntimeSpec* Fireball = FindSkill(1);
	TestNotNull(TEXT("Slot 1 fireball"), Fireball);
	if (Fireball)
	{
		TestEqual(TEXT("Fireball shape"), Fireball->EffectShape, EDBAPlayableSkillEffectShape::Projectile);
		TestEqual(TEXT("Fireball element"), Fireball->Element, EDBAElement::Fire);
		TestTrue(TEXT("Fireball has projectile class"), Fireball->ProjectileClass != nullptr);
		TestTrue(TEXT("Fireball has cast VFX"), !Fireball->CastNiagaraVFXAsset.IsNull());
		TestTrue(TEXT("Fireball has cast SFX"), !Fireball->CastSFXAsset.IsNull());
		TestTrue(TEXT("Fireball has flight VFX"), !Fireball->ProjectileNiagaraVFXAsset.IsNull());
		TestTrue(TEXT("Fireball has impact VFX"), !Fireball->ImpactNiagaraVFXAsset.IsNull());
	}

	const FDBAPlayableSkillRuntimeSpec* Frost = FindSkill(2);
	TestNotNull(TEXT("Slot 2 frost shard"), Frost);
	if (Frost)
	{
		TestEqual(TEXT("Frost shape"), Frost->EffectShape, EDBAPlayableSkillEffectShape::Projectile);
		TestEqual(TEXT("Frost element"), Frost->Element, EDBAElement::Water);
		TestTrue(TEXT("Frost has projectile class"), Frost->ProjectileClass != nullptr);
		TestTrue(TEXT("Frost has cast VFX"), !Frost->CastNiagaraVFXAsset.IsNull());
		TestTrue(TEXT("Frost has flight VFX"), !Frost->ProjectileNiagaraVFXAsset.IsNull());
		TestTrue(TEXT("Frost has impact VFX"), !Frost->ImpactNiagaraVFXAsset.IsNull());
	}

	const FDBAPlayableSkillRuntimeSpec* Bloom = FindSkill(3);
	TestNotNull(TEXT("Slot 3 bloom"), Bloom);
	if (Bloom)
	{
		TestEqual(TEXT("Bloom shape"), Bloom->EffectShape, EDBAPlayableSkillEffectShape::BloomHealing);
		TestEqual(TEXT("Bloom element"), Bloom->Element, EDBAElement::Wood);
		TestTrue(TEXT("Bloom has spell class"), Bloom->BloomHealingClass != nullptr);
		TestTrue(TEXT("Bloom has cast VFX"), !Bloom->CastNiagaraVFXAsset.IsNull());
		TestTrue(TEXT("Bloom has flight VFX"), !Bloom->ProjectileNiagaraVFXAsset.IsNull());
		TestTrue(TEXT("Bloom has impact VFX"), !Bloom->ImpactNiagaraVFXAsset.IsNull());
	}

	const FDBAPlayableSkillRuntimeSpec* Chain = FindSkill(4);
	TestNotNull(TEXT("Slot 4 chain lightning"), Chain);
	if (Chain)
	{
		TestEqual(TEXT("Chain shape"), Chain->EffectShape, EDBAPlayableSkillEffectShape::ChainLightning);
		TestTrue(TEXT("Chain has spell class"), Chain->ChainLightningClass != nullptr);
		TestTrue(TEXT("Chain has cast VFX"), !Chain->CastNiagaraVFXAsset.IsNull());
		TestTrue(TEXT("Chain has flight VFX"), !Chain->ProjectileNiagaraVFXAsset.IsNull());
		TestTrue(TEXT("Chain has impact VFX"), !Chain->ImpactNiagaraVFXAsset.IsNull());
	}

	const FDBAPlayableSkillRuntimeSpec* Shield = FindSkill(5);
	TestNotNull(TEXT("Slot 5 shield"), Shield);
	if (Shield)
	{
		TestEqual(TEXT("Shield shape"), Shield->EffectShape, EDBAPlayableSkillEffectShape::HolyShield);
		TestTrue(TEXT("Shield has spell class"), Shield->HolyShieldClass != nullptr);
		TestTrue(TEXT("Shield has cast VFX"), !Shield->CastNiagaraVFXAsset.IsNull());
		TestTrue(TEXT("Shield has flight VFX"), !Shield->ProjectileNiagaraVFXAsset.IsNull());
		TestTrue(TEXT("Shield has impact VFX"), !Shield->ImpactNiagaraVFXAsset.IsNull());
	}

	const FDBAPlayableSkillRuntimeSpec* Shadow = FindSkill(6);
	TestNotNull(TEXT("Slot 6 shadow bolt"), Shadow);
	if (Shadow)
	{
		TestEqual(TEXT("Shadow shape"), Shadow->EffectShape, EDBAPlayableSkillEffectShape::Projectile);
		TestTrue(TEXT("Shadow has projectile class"), Shadow->ProjectileClass != nullptr);
		TestTrue(TEXT("Shadow has cast VFX"), !Shadow->CastNiagaraVFXAsset.IsNull());
		TestTrue(TEXT("Shadow has flight VFX"), !Shadow->ProjectileNiagaraVFXAsset.IsNull());
		TestTrue(TEXT("Shadow has impact VFX"), !Shadow->ImpactNiagaraVFXAsset.IsNull());
		TestTrue(TEXT("Shadow has impact SFX"), !Shadow->ImpactSFXAsset.IsNull());
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAPlayableSkillCatalogValidationTest,
	"DivineBeastsArena.Combat.PlayableSkillCatalog.Validation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAPlayableSkillCatalogValidationTest::RunTest(const FString& Parameters)
{
	UDBAPlayableSkillCatalogDataAsset* Catalog = NewObject<UDBAPlayableSkillCatalogDataAsset>();
	TestNotNull(TEXT("Catalog"), Catalog);
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
	DuplicateSlot.DisplayName = FText::FromString(TEXT("Duplicate Slot"));
	DuplicateSlot.EffectShape = EDBAPlayableSkillEffectShape::HolyShield;
	DuplicateSlot.Magnitude = 10.0f;
	DuplicateSlot.Cooldown = 1.0f;
	DuplicateSlot.CastVFXScale = 1.0f;
	DuplicateSlot.HolyShieldClass = ADBAHolyShieldSpell::StaticClass();

	Catalog->SkillSpecs.Add(BrokenProjectile);
	Catalog->SkillSpecs.Add(DuplicateSlot);

	TArray<FString> ValidationErrors;
	TestFalse(TEXT("Broken catalog fails validation"), Catalog->ValidateDataIntegrity(ValidationErrors));
	TestTrue(TEXT("Broken catalog reports errors"), ValidationErrors.Num() > 0);

	const auto HasErrorContaining = [&ValidationErrors](const TCHAR* ExpectedText)
	{
		return ValidationErrors.ContainsByPredicate([ExpectedText](const FString& Error)
		{
			return Error.Contains(ExpectedText);
		});
	};

	TestTrue(TEXT("Missing catalog id is reported"), HasErrorContaining(TEXT("CatalogId")));
	TestTrue(TEXT("Missing projectile class is reported"), HasErrorContaining(TEXT("ProjectileClass")));
	TestTrue(TEXT("Duplicate slot is reported"), HasErrorContaining(TEXT("SkillSlot is duplicated")));
	TestTrue(TEXT("Missing cast VFX is reported"), HasErrorContaining(TEXT("CastNiagaraVFXAsset")));
	TestTrue(TEXT("Missing cast SFX is reported"), HasErrorContaining(TEXT("CastSFXAsset")));
	TestTrue(TEXT("Missing impact VFX is reported"), HasErrorContaining(TEXT("ImpactNiagaraVFXAsset")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAPlayableSkillCatalogDataAssetOverrideTest,
	"DivineBeastsArena.Combat.PlayableSkillCatalog.DataAssetOverride",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAPlayableSkillCatalogDataAssetOverrideTest::RunTest(const FString& Parameters)
{
	UDBAPlayableSkillCatalogDataAsset* Catalog = NewObject<UDBAPlayableSkillCatalogDataAsset>();
	TestNotNull(TEXT("Catalog"), Catalog);
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
	OverrideSpec.DisplayName = FText::FromString(TEXT("Override Fireball"));
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
	TestEqual(TEXT("Catalog ignores invalid slots"), CatalogSpecs.Num(), 1);
	TestEqual(TEXT("Catalog override slot"), CatalogSpecs[0].SkillSlot, 1);

	UDBAPlayableSkillComponent* SkillComponent = NewObject<UDBAPlayableSkillComponent>();
	TestNotNull(TEXT("SkillComponent"), SkillComponent);
	if (!SkillComponent)
	{
		return false;
	}

	SkillComponent->SetSkillCatalog(Catalog);

	const TArray<FDBAPlayableSkillRuntimeSpec> EffectiveSpecs = SkillComponent->GetAllSkillSpecs();
	TestEqual(TEXT("Catalog override keeps default fallback count"), EffectiveSpecs.Num(), 6);

	FDBAPlayableSkillCatalogSummary OverrideSummary = SkillComponent->GetSkillCatalogSummary();
	TestEqual(TEXT("Override summary source"), OverrideSummary.Source, EDBAPlayableSkillCatalogSource::DataAssetWithDefaults);
	TestEqual(TEXT("Override summary catalog id"), OverrideSummary.CatalogId, FName(TEXT("DefaultPlayableSkillCatalog")));
	TestEqual(TEXT("Override summary skill count"), OverrideSummary.SkillCount, 6);
	TestEqual(TEXT("Override summary configured catalog count"), OverrideSummary.ConfiguredCatalogSkillCount, 1);
	TestTrue(TEXT("Override summary appends defaults"), OverrideSummary.bAppendsBuiltInDefaults);

	const auto FindSkill = [](const TArray<FDBAPlayableSkillRuntimeSpec>& SkillSpecs, int32 SkillSlot) -> const FDBAPlayableSkillRuntimeSpec*
	{
		return SkillSpecs.FindByPredicate([SkillSlot](const FDBAPlayableSkillRuntimeSpec& Spec)
		{
			return Spec.SkillSlot == SkillSlot;
		});
	};

	const FDBAPlayableSkillRuntimeSpec* SlotOne = FindSkill(EffectiveSpecs, 1);
	TestNotNull(TEXT("Slot 1 override"), SlotOne);
	if (SlotOne)
	{
		TestEqual(TEXT("Slot 1 override skill id"), SlotOne->SkillId, FName(TEXT("Test.Skill.OverrideFireball")));
		TestEqual(TEXT("Slot 1 override name"), SlotOne->DisplayName.ToString(), FString(TEXT("Override Fireball")));
		TestEqual(TEXT("Slot 1 override magnitude"), SlotOne->Magnitude, 99.0f);
		TestEqual(TEXT("Slot 1 override cooldown"), SlotOne->Cooldown, 9.5f);
		TestTrue(TEXT("Slot 1 override keeps projectile class"), SlotOne->ProjectileClass == ADBAFireballProjectile::StaticClass());
	}

	const FDBAPlayableSkillRuntimeSpec* SlotTwo = FindSkill(EffectiveSpecs, 2);
	TestNotNull(TEXT("Slot 2 default fallback"), SlotTwo);
	if (SlotTwo)
	{
		TestEqual(TEXT("Slot 2 remains default frost skill"), SlotTwo->SkillId, FName(TEXT("Lobby.Skill02.FrostShard")));
	}

	SkillComponent->SetAppendDefaultSkillsWhenCatalogMissingSlots(false);
	const TArray<FDBAPlayableSkillRuntimeSpec> CatalogOnlySpecs = SkillComponent->GetAllSkillSpecs();
	TestEqual(TEXT("Catalog-only mode does not append defaults"), CatalogOnlySpecs.Num(), 1);

	const FDBAPlayableSkillCatalogSummary CatalogOnlySummary = SkillComponent->GetSkillCatalogSummary();
	TestEqual(TEXT("Catalog-only summary source"), CatalogOnlySummary.Source, EDBAPlayableSkillCatalogSource::DataAssetOnly);
	TestEqual(TEXT("Catalog-only summary skill count"), CatalogOnlySummary.SkillCount, 1);
	TestFalse(TEXT("Catalog-only summary does not append defaults"), CatalogOnlySummary.bAppendsBuiltInDefaults);

	FDBAPlayableSkillRuntimeSpec MissingSpec;
	TestFalse(TEXT("Catalog-only mode has no slot 2"), SkillComponent->GetSkillSpec(2, MissingSpec));

	return true;
}

#endif
