// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
Readable notes:
- App: DBA_GameClient Unreal Engine client.
- Purpose: character-owned playable skill catalog. It provides a stable C++ and
  Blueprint surface for skill bar UI, test maps, and future DataAsset driven
  content while the existing combat actors keep doing the actual execution.
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameDBA/Combat/DBAPlayableSkillTypes.h"
#include "DBAPlayableSkillComponent.generated.h"

UCLASS(ClassGroup=(DBA), meta=(BlueprintSpawnableComponent))
class DIVINEBEASTSARENA_API UDBAPlayableSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDBAPlayableSkillComponent();

	UFUNCTION(BlueprintCallable, Category = "DBA|Playable Skill")
	bool GetSkillSpec(int32 SkillSlot, FDBAPlayableSkillRuntimeSpec& OutSpec) const;

	UFUNCTION(BlueprintCallable, Category = "DBA|Playable Skill")
	TArray<FDBAPlayableSkillRuntimeSpec> GetAllSkillSpecs() const;

	UFUNCTION(BlueprintCallable, Category = "DBA|Playable Skill")
	void SetSkillSpec(int32 SkillSlot, const FDBAPlayableSkillRuntimeSpec& InSpec);

	UFUNCTION(BlueprintCallable, Category = "DBA|Playable Skill")
	void ResetToDefaultSkillSpecs();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Playable Skill")
	bool bResolveSkillIdsFromEquippedSkillGroup = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Playable Skill")
	TArray<FDBAPlayableSkillRuntimeSpec> SkillSpecs;

private:
	FName ResolveEquippedSkillId(int32 SkillSlot, FName FallbackSkillId) const;
};
