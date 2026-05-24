// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Combat/DBAPlayableSkillCatalogDataAsset.h"

#include "GameDBA/Combat/DBABloomHealingSpell.h"
#include "GameDBA/Combat/DBAChainLightningSpell.h"
#include "GameDBA/Combat/DBAHolyShieldSpell.h"
#include "GameDBA/Combat/DBASkillProjectileBase.h"

namespace
{
	void AddSkillValidationError(TArray<FString>& OutErrors, const FDBAPlayableSkillRuntimeSpec& Spec, const FString& Message)
	{
		OutErrors.Add(FString::Printf(TEXT("SkillSlot=%d SkillId=%s: %s"), Spec.SkillSlot, *Spec.SkillId.ToString(), *Message));
	}

	bool ValidateRequiredClass(const FDBAPlayableSkillRuntimeSpec& Spec, TArray<FString>& OutErrors)
	{
		switch (Spec.EffectShape)
		{
		case EDBAPlayableSkillEffectShape::Projectile:
			if (!Spec.ProjectileClass)
			{
				AddSkillValidationError(OutErrors, Spec, TEXT("ProjectileClass is not configured"));
				return false;
			}
			return true;
		case EDBAPlayableSkillEffectShape::ChainLightning:
			if (!Spec.ChainLightningClass)
			{
				AddSkillValidationError(OutErrors, Spec, TEXT("ChainLightningClass is not configured"));
				return false;
			}
			return true;
		case EDBAPlayableSkillEffectShape::BloomHealing:
			if (!Spec.BloomHealingClass)
			{
				AddSkillValidationError(OutErrors, Spec, TEXT("BloomHealingClass is not configured"));
				return false;
			}
			return true;
		case EDBAPlayableSkillEffectShape::HolyShield:
			if (!Spec.HolyShieldClass)
			{
				AddSkillValidationError(OutErrors, Spec, TEXT("HolyShieldClass is not configured"));
				return false;
			}
			return true;
		default:
			AddSkillValidationError(OutErrors, Spec, TEXT("EffectShape is invalid"));
			return false;
		}
	}

	bool ValidateShapeTuning(const FDBAPlayableSkillRuntimeSpec& Spec, TArray<FString>& OutErrors)
	{
		bool bIsValid = true;
		if (Spec.EffectShape == EDBAPlayableSkillEffectShape::Projectile)
		{
			if (Spec.ProjectileSpeed <= 0.0f)
			{
				AddSkillValidationError(OutErrors, Spec, TEXT("ProjectileSpeed must be greater than 0"));
				bIsValid = false;
			}

			if (Spec.ProjectileRadius <= 0.0f)
			{
				AddSkillValidationError(OutErrors, Spec, TEXT("ProjectileRadius must be greater than 0"));
				bIsValid = false;
			}
		}

		return bIsValid;
	}

	bool ValidatePresentationAssets(const FDBAPlayableSkillRuntimeSpec& Spec, TArray<FString>& OutErrors)
	{
		bool bIsValid = true;

		if (Spec.ProjectileNiagaraVFXAsset.IsNull())
		{
			AddSkillValidationError(OutErrors, Spec, TEXT("ProjectileNiagaraVFXAsset is not configured"));
			bIsValid = false;
		}

		if (Spec.ImpactNiagaraVFXAsset.IsNull())
		{
			AddSkillValidationError(OutErrors, Spec, TEXT("ImpactNiagaraVFXAsset is not configured"));
			bIsValid = false;
		}

		if (Spec.CastSFXAsset.IsNull())
		{
			AddSkillValidationError(OutErrors, Spec, TEXT("CastSFXAsset is not configured"));
			bIsValid = false;
		}

		if (Spec.FlySFXAsset.IsNull())
		{
			AddSkillValidationError(OutErrors, Spec, TEXT("FlySFXAsset is not configured"));
			bIsValid = false;
		}

		if (Spec.ImpactSFXAsset.IsNull())
		{
			AddSkillValidationError(OutErrors, Spec, TEXT("ImpactSFXAsset is not configured"));
			bIsValid = false;
		}

		return bIsValid;
	}
}

bool UDBAPlayableSkillCatalogDataAsset::GetSkillSpec(int32 SkillSlot, FDBAPlayableSkillRuntimeSpec& OutSpec) const
{
	const TArray<FDBAPlayableSkillRuntimeSpec> NormalizedSpecs = GetAllSkillSpecs();
	for (const FDBAPlayableSkillRuntimeSpec& Spec : NormalizedSpecs)
	{
		if (Spec.SkillSlot == SkillSlot)
		{
			OutSpec = Spec;
			return true;
		}
	}

	return false;
}

TArray<FDBAPlayableSkillRuntimeSpec> UDBAPlayableSkillCatalogDataAsset::GetAllSkillSpecs() const
{
	TArray<FDBAPlayableSkillRuntimeSpec> NormalizedSpecs;
	NormalizedSpecs.Reserve(SkillSpecs.Num());

	for (const FDBAPlayableSkillRuntimeSpec& Spec : SkillSpecs)
	{
		if (Spec.SkillSlot <= 0)
		{
			continue;
		}

		if (FDBAPlayableSkillRuntimeSpec* ExistingSpec = NormalizedSpecs.FindByPredicate([&Spec](const FDBAPlayableSkillRuntimeSpec& Candidate)
			{
				return Candidate.SkillSlot == Spec.SkillSlot;
			}))
		{
			*ExistingSpec = Spec;
		}
		else
		{
			NormalizedSpecs.Add(Spec);
		}
	}

	NormalizedSpecs.Sort([](const FDBAPlayableSkillRuntimeSpec& Left, const FDBAPlayableSkillRuntimeSpec& Right)
	{
		return Left.SkillSlot < Right.SkillSlot;
	});

	return NormalizedSpecs;
}

bool UDBAPlayableSkillCatalogDataAsset::ValidateDataIntegrity(TArray<FString>& OutErrors) const
{
	OutErrors.Empty();

	bool bIsValid = true;
	if (CatalogId.IsNone())
	{
		OutErrors.Add(TEXT("CatalogId is not configured"));
		bIsValid = false;
	}

	TArray<FString> SkillErrors;
	if (!ValidateSkillSpecs(SkillSpecs, SkillErrors))
	{
		OutErrors.Append(SkillErrors);
		bIsValid = false;
	}

	return bIsValid;
}

bool UDBAPlayableSkillCatalogDataAsset::ValidateData_Implementation(TArray<FString>& OutErrors) const
{
	return ValidateDataIntegrity(OutErrors);
}

bool UDBAPlayableSkillCatalogDataAsset::ValidateSkillSpecs(const TArray<FDBAPlayableSkillRuntimeSpec>& InSkillSpecs, TArray<FString>& OutErrors)
{
	OutErrors.Empty();
	bool bIsValid = true;

	if (InSkillSpecs.IsEmpty())
	{
		OutErrors.Add(TEXT("SkillSpecs is empty"));
		return false;
	}

	TSet<int32> SeenSlots;
	for (const FDBAPlayableSkillRuntimeSpec& Spec : InSkillSpecs)
	{
		if (Spec.SkillSlot <= 0)
		{
			AddSkillValidationError(OutErrors, Spec, TEXT("SkillSlot must be greater than 0"));
			bIsValid = false;
			continue;
		}

		if (SeenSlots.Contains(Spec.SkillSlot))
		{
			AddSkillValidationError(OutErrors, Spec, TEXT("SkillSlot is duplicated"));
			bIsValid = false;
		}
		SeenSlots.Add(Spec.SkillSlot);

		if (Spec.SkillId.IsNone())
		{
			AddSkillValidationError(OutErrors, Spec, TEXT("SkillId is not configured"));
			bIsValid = false;
		}

		if (Spec.DisplayName.IsEmptyOrWhitespace())
		{
			AddSkillValidationError(OutErrors, Spec, TEXT("DisplayName is not configured"));
			bIsValid = false;
		}

		if (Spec.Magnitude <= 0.0f)
		{
			AddSkillValidationError(OutErrors, Spec, TEXT("Magnitude must be greater than 0"));
			bIsValid = false;
		}

		if (Spec.Cooldown <= 0.0f)
		{
			AddSkillValidationError(OutErrors, Spec, TEXT("Cooldown must be greater than 0"));
			bIsValid = false;
		}

		if (Spec.CastVFXScale <= 0.0f)
		{
			AddSkillValidationError(OutErrors, Spec, TEXT("CastVFXScale must be greater than 0"));
			bIsValid = false;
		}

		bIsValid &= ValidateRequiredClass(Spec, OutErrors);
		bIsValid &= ValidateShapeTuning(Spec, OutErrors);
		bIsValid &= ValidatePresentationAssets(Spec, OutErrors);
	}

	return bIsValid;
}
