// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
Readable notes:
- App: DBA_GameClient Unreal Engine client.
- Purpose: shared Niagara User parameter payload for skill VFX. Keep the names
  aligned with DBA_GameClient/Docs/NiagaraMagicVFX_Design.md.
*/

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "DBANiagaraSkillParameters.generated.h"

class UNiagaraComponent;

USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBANiagaraSkillParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Niagara")
	float EffectRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Niagara")
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Niagara")
	float TickInterval = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Niagara")
	float TrailLength = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Niagara", meta = (ClampMin = "0.0", ClampMax = "4.0"))
	float Intensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Niagara")
	FLinearColor TeamTint = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Niagara")
	FLinearColor ElementColorA = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Niagara")
	FLinearColor ElementColorB = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Niagara")
	FLinearColor HighlightColor = FLinearColor::White;
};

UCLASS()
class DIVINEBEASTSARENA_API UDBANiagaraSkillParameterLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DBA|Niagara")
	static FDBANiagaraSkillParameters MakeElementParameters(
		EDBAElement Element,
		float EffectRadius,
		float Duration,
		float TickInterval,
		float TrailLength,
		float Intensity = 1.0f);

	UFUNCTION(BlueprintPure, Category = "DBA|Niagara")
	static FLinearColor ResolveElementPrimaryColor(EDBAElement Element);

	UFUNCTION(BlueprintPure, Category = "DBA|Niagara")
	static FLinearColor ResolveElementSecondaryColor(EDBAElement Element);

	UFUNCTION(BlueprintPure, Category = "DBA|Niagara")
	static FLinearColor ResolveElementHighlightColor(EDBAElement Element);

	UFUNCTION(BlueprintCallable, Category = "DBA|Niagara")
	static void ApplySkillParameters(
		UNiagaraComponent* NiagaraComponent,
		const FDBANiagaraSkillParameters& Parameters,
		float Damage,
		const FVector& TargetLocation,
		const FVector& Direction,
		float ProjectileSpeed = 0.0f,
		float ProjectileRadius = 0.0f);
};
