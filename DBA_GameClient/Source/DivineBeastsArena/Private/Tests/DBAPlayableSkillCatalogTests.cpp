// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Combat/DBAPlayableSkillComponent.h"
#include "GameDBA/Combat/DBAPlayableSkillTypes.h"
#include "GameDBA/Combat/DBABloomHealingSpell.h"
#include "GameDBA/Combat/DBAChainLightningSpell.h"
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
		TestTrue(TEXT("Fireball has cast SFX"), !Fireball->CastSFXAsset.IsNull());
		TestTrue(TEXT("Fireball has flight VFX"), !Fireball->ProjectileNiagaraVFXAsset.IsNull());
	}

	const FDBAPlayableSkillRuntimeSpec* Frost = FindSkill(2);
	TestNotNull(TEXT("Slot 2 frost shard"), Frost);
	if (Frost)
	{
		TestEqual(TEXT("Frost shape"), Frost->EffectShape, EDBAPlayableSkillEffectShape::Projectile);
		TestEqual(TEXT("Frost element"), Frost->Element, EDBAElement::Water);
		TestTrue(TEXT("Frost has projectile class"), Frost->ProjectileClass != nullptr);
	}

	const FDBAPlayableSkillRuntimeSpec* Bloom = FindSkill(3);
	TestNotNull(TEXT("Slot 3 bloom"), Bloom);
	if (Bloom)
	{
		TestEqual(TEXT("Bloom shape"), Bloom->EffectShape, EDBAPlayableSkillEffectShape::BloomHealing);
		TestEqual(TEXT("Bloom element"), Bloom->Element, EDBAElement::Wood);
		TestTrue(TEXT("Bloom has spell class"), Bloom->BloomHealingClass != nullptr);
	}

	const FDBAPlayableSkillRuntimeSpec* Chain = FindSkill(4);
	TestNotNull(TEXT("Slot 4 chain lightning"), Chain);
	if (Chain)
	{
		TestEqual(TEXT("Chain shape"), Chain->EffectShape, EDBAPlayableSkillEffectShape::ChainLightning);
		TestTrue(TEXT("Chain has spell class"), Chain->ChainLightningClass != nullptr);
	}

	const FDBAPlayableSkillRuntimeSpec* Shield = FindSkill(5);
	TestNotNull(TEXT("Slot 5 shield"), Shield);
	if (Shield)
	{
		TestEqual(TEXT("Shield shape"), Shield->EffectShape, EDBAPlayableSkillEffectShape::HolyShield);
		TestTrue(TEXT("Shield has spell class"), Shield->HolyShieldClass != nullptr);
	}

	const FDBAPlayableSkillRuntimeSpec* Shadow = FindSkill(6);
	TestNotNull(TEXT("Slot 6 shadow bolt"), Shadow);
	if (Shadow)
	{
		TestEqual(TEXT("Shadow shape"), Shadow->EffectShape, EDBAPlayableSkillEffectShape::Projectile);
		TestTrue(TEXT("Shadow has projectile class"), Shadow->ProjectileClass != nullptr);
		TestTrue(TEXT("Shadow has impact SFX"), !Shadow->ImpactSFXAsset.IsNull());
	}

	return true;
}

#endif
