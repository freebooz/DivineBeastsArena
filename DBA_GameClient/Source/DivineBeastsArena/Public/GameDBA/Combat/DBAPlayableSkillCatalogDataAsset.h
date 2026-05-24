// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
Readable notes:
- App: DBA_GameClient Unreal Engine client.
- Purpose: editable playable skill catalog for lobby and combat skill bars.
  Designers can override the built-in C++ defaults without touching character
  execution code.
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameDBA/Core/Interfaces/DBAValidatableInterface.h"
#include "GameDBA/Combat/DBAPlayableSkillTypes.h"
#include "DBAPlayableSkillCatalogDataAsset.generated.h"

UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAPlayableSkillCatalogDataAsset : public UDataAsset, public IDBAValidatableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|Playable Skill Catalog")
	bool GetSkillSpec(int32 SkillSlot, FDBAPlayableSkillRuntimeSpec& OutSpec) const;

	UFUNCTION(BlueprintCallable, Category = "DBA|Playable Skill Catalog")
	TArray<FDBAPlayableSkillRuntimeSpec> GetAllSkillSpecs() const;

	UFUNCTION(BlueprintCallable, Category = "DBA|Playable Skill Catalog")
	bool ValidateDataIntegrity(TArray<FString>& OutErrors) const;

	virtual bool ValidateData_Implementation(TArray<FString>& OutErrors) const override;

	static bool ValidateSkillSpecs(const TArray<FDBAPlayableSkillRuntimeSpec>& InSkillSpecs, TArray<FString>& OutErrors);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Playable Skill Catalog")
	FName CatalogId = TEXT("DefaultPlayableSkillCatalog");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Playable Skill Catalog")
	TArray<FDBAPlayableSkillRuntimeSpec> SkillSpecs;
};
