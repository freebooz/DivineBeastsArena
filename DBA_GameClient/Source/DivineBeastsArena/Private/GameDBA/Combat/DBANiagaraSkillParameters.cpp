// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Combat/DBANiagaraSkillParameters.h"

#include "NiagaraComponent.h"

namespace
{
	FVector SafeNiagaraDirection(FVector Direction)
	{
		Direction = Direction.GetSafeNormal();
		return Direction.IsNearlyZero() ? FVector::ForwardVector : Direction;
	}
}

FDBANiagaraSkillParameters UDBANiagaraSkillParameterLibrary::MakeElementParameters(
	EDBAElement Element,
	float EffectRadius,
	float Duration,
	float TickInterval,
	float TrailLength,
	float Intensity)
{
	FDBANiagaraSkillParameters Parameters;
	Parameters.EffectRadius = EffectRadius;
	Parameters.Duration = Duration;
	Parameters.TickInterval = TickInterval;
	Parameters.TrailLength = TrailLength;
	Parameters.Intensity = FMath::Max(Intensity, 0.0f);
	Parameters.ElementColorA = ResolveElementPrimaryColor(Element);
	Parameters.ElementColorB = ResolveElementSecondaryColor(Element);
	Parameters.HighlightColor = ResolveElementHighlightColor(Element);
	return Parameters;
}

FLinearColor UDBANiagaraSkillParameterLibrary::ResolveElementPrimaryColor(EDBAElement Element)
{
	switch (Element)
	{
	case EDBAElement::Fire:
		return FLinearColor(1.0f, 0.267f, 0.0f, 1.0f);
	case EDBAElement::Water:
		return FLinearColor(0.0f, 0.8f, 1.0f, 1.0f);
	case EDBAElement::Wood:
		return FLinearColor(0.267f, 1.0f, 0.267f, 1.0f);
	case EDBAElement::Gold:
		return FLinearColor(1.0f, 0.867f, 0.267f, 1.0f);
	case EDBAElement::Earth:
		return FLinearColor(0.533f, 0.4f, 0.267f, 1.0f);
	default:
		return FLinearColor(0.65f, 0.78f, 1.0f, 1.0f);
	}
}

FLinearColor UDBANiagaraSkillParameterLibrary::ResolveElementSecondaryColor(EDBAElement Element)
{
	switch (Element)
	{
	case EDBAElement::Fire:
		return FLinearColor(1.0f, 0.533f, 0.0f, 1.0f);
	case EDBAElement::Water:
		return FLinearColor(0.533f, 0.867f, 1.0f, 1.0f);
	case EDBAElement::Wood:
		return FLinearColor(0.533f, 1.0f, 0.533f, 1.0f);
	case EDBAElement::Gold:
		return FLinearColor(1.0f, 1.0f, 0.533f, 1.0f);
	case EDBAElement::Earth:
		return FLinearColor(0.667f, 0.533f, 0.4f, 1.0f);
	default:
		return FLinearColor(0.75f, 0.55f, 1.0f, 1.0f);
	}
}

FLinearColor UDBANiagaraSkillParameterLibrary::ResolveElementHighlightColor(EDBAElement Element)
{
	switch (Element)
	{
	case EDBAElement::Fire:
		return FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
	case EDBAElement::Water:
	case EDBAElement::Gold:
	case EDBAElement::Wood:
		return FLinearColor::White;
	case EDBAElement::Earth:
		return FLinearColor(1.0f, 0.78f, 0.34f, 1.0f);
	default:
		return FLinearColor(0.667f, 0.267f, 1.0f, 1.0f);
	}
}

void UDBANiagaraSkillParameterLibrary::ApplySkillParameters(
	UNiagaraComponent* NiagaraComponent,
	const FDBANiagaraSkillParameters& Parameters,
	float Damage,
	const FVector& TargetLocation,
	const FVector& Direction,
	float ProjectileSpeed,
	float ProjectileRadius)
{
	if (!NiagaraComponent)
	{
		return;
	}

	const FVector SafeDirection = SafeNiagaraDirection(Direction);
	const float EffectiveRadius = ProjectileRadius > 0.0f ? ProjectileRadius : Parameters.EffectRadius;

	NiagaraComponent->SetVariableFloat(TEXT("User.EffectRadius"), EffectiveRadius);
	NiagaraComponent->SetVariableFloat(TEXT("User.Damage"), Damage);
	NiagaraComponent->SetVariableFloat(TEXT("User.Duration"), Parameters.Duration);
	NiagaraComponent->SetVariableFloat(TEXT("User.Intensity"), Parameters.Intensity);
	NiagaraComponent->SetVariableFloat(TEXT("User.TickInterval"), Parameters.TickInterval);
	NiagaraComponent->SetVariableFloat(TEXT("User.TickPulse"), 0.0f);
	NiagaraComponent->SetVariableFloat(TEXT("User.ProjectileSpeed"), ProjectileSpeed);
	NiagaraComponent->SetVariableFloat(TEXT("User.ProjectileRadius"), ProjectileRadius);
	NiagaraComponent->SetVariableFloat(TEXT("User.TrailLength"), Parameters.TrailLength);
	NiagaraComponent->SetVariableVec3(TEXT("User.TargetLocation"), TargetLocation);
	NiagaraComponent->SetVariableVec3(TEXT("User.Direction"), SafeDirection);
	NiagaraComponent->SetVariableLinearColor(TEXT("User.TeamTint"), Parameters.TeamTint);
	NiagaraComponent->SetVariableLinearColor(TEXT("User.ElementColorA"), Parameters.ElementColorA);
	NiagaraComponent->SetVariableLinearColor(TEXT("User.ElementColorB"), Parameters.ElementColorB);
	NiagaraComponent->SetVariableLinearColor(TEXT("User.HighlightColor"), Parameters.HighlightColor);
}
