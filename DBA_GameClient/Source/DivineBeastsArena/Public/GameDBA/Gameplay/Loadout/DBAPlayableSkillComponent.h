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
#include "GameDBA/Gameplay/Loadout/DBAPlayableSkillCatalogDataAsset.h"
#include "GameDBA/Gameplay/Loadout/DBAPlayableSkillTypes.h"
#include "TimerManager.h"
#include "DBAPlayableSkillComponent.generated.h"

class UNiagaraSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDBAPlayableSkillCatalogChanged, const FDBAPlayableSkillCatalogSummary&, Summary);

UCLASS(ClassGroup=(DBA), meta=(BlueprintSpawnableComponent))
class DIVINEBEASTSARENA_API UDBAPlayableSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDBAPlayableSkillComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "DBA|Playable Skill")
	bool GetSkillSpec(int32 SkillSlot, FDBAPlayableSkillRuntimeSpec& OutSpec) const;

	UFUNCTION(BlueprintCallable, Category = "DBA|Playable Skill")
	TArray<FDBAPlayableSkillRuntimeSpec> GetAllSkillSpecs() const;

	UFUNCTION(BlueprintCallable, Category = "DBA|Playable Skill")
	void SetSkillSpec(int32 SkillSlot, const FDBAPlayableSkillRuntimeSpec& InSpec);

	UFUNCTION(BlueprintCallable, Category = "DBA|Playable Skill")
	void ResetToDefaultSkillSpecs();

	UFUNCTION(BlueprintCallable, Category = "DBA|Playable Skill")
	void RequestDefaultSkillCatalogAsync();

	UFUNCTION(BlueprintCallable, Category = "DBA|Playable Skill")
	void SetSkillCatalog(UDBAPlayableSkillCatalogDataAsset* InSkillCatalog);

	UFUNCTION(BlueprintPure, Category = "DBA|Playable Skill")
	UDBAPlayableSkillCatalogDataAsset* GetSkillCatalog() const { return SkillCatalog; }

	UFUNCTION(BlueprintCallable, Category = "DBA|Playable Skill")
	void SetAppendDefaultSkillsWhenCatalogMissingSlots(bool bInAppendDefaults);

	UFUNCTION(BlueprintCallable, Category = "DBA|Playable Skill")
	bool ValidateEffectiveSkillSpecs(TArray<FString>& OutErrors) const;

	UFUNCTION(BlueprintCallable, Category = "DBA|Playable Skill")
	FDBAPlayableSkillCatalogSummary GetSkillCatalogSummary() const;

	UFUNCTION(BlueprintCallable, Category = "DBA|Playable Skill")
	void PreloadSkillPresentationAssets() const;

	/** 目录异步加载完成或配置发生变化时广播，UI 应订阅该事件刷新。 */
	UPROPERTY(BlueprintAssignable, Category = "DBA|Playable Skill")
	FOnDBAPlayableSkillCatalogChanged OnSkillCatalogChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Playable Skill")
	bool bResolveSkillIdsFromEquippedSkillGroup = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Playable Skill")
	bool bAppendDefaultSkillsWhenCatalogMissingSlots = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Playable Skill")
	TObjectPtr<UDBAPlayableSkillCatalogDataAsset> SkillCatalog;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Playable Skill")
	TSoftObjectPtr<UDBAPlayableSkillCatalogDataAsset> DefaultSkillCatalog;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Playable Skill")
	TArray<FDBAPlayableSkillRuntimeSpec> SkillSpecs;

private:
	void BuildEffectiveSkillSpecs(TArray<FDBAPlayableSkillRuntimeSpec>& OutSpecs) const;
	const UDBAPlayableSkillCatalogDataAsset* GetEffectiveSkillCatalog() const;
	TSoftObjectPtr<UDBAPlayableSkillCatalogDataAsset> ResolveDefaultSkillCatalogAsset() const;
	void HandleDefaultSkillCatalogLoaded(UDBAPlayableSkillCatalogDataAsset* LoadedCatalog);
	void BroadcastSkillCatalogChanged();
	FName ResolveEquippedSkillId(int32 SkillSlot, FName FallbackSkillId) const;
	void QueueNiagaraWarmupAssets();
	void AddNiagaraWarmupPath(const TSoftObjectPtr<UNiagaraSystem>& Asset);
	void PumpNiagaraWarmupQueue();
	void WarmUpNiagaraSystem(UNiagaraSystem* NiagaraSystem) const;

	UPROPERTY(Transient)
	TObjectPtr<UDBAPlayableSkillCatalogDataAsset> LoadedDefaultSkillCatalog;

	TArray<FSoftObjectPath> PendingNiagaraWarmupPaths;
	FTimerHandle NiagaraWarmupTimerHandle;
};
