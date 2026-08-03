// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端 / Dedicated Server。
- 文件职责：保存单局玩家战斗统计，并在权威服结算时映射为后端 Runtime match-results DTO。
- 阅读重点：这些字段是 MVP 结算统计源，后续击杀、助攻和得分事件应写入这里。
- 修改提示：新增结算字段时，请同步更新 Runtime match lifecycle 契约和后端 Runtime 结果 DTO。
*/

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GameBackendRuntimeService.h"
#include "DBAPlayerState.generated.h"

class UAbilitySystemComponent;
class UDBAAbilitySystemComponent;
class UDBABattleAttributeSet;
class UDBABattleAttributeDefaultsDataAsset;
class UDBAHeroGrowthAttributeSet;
class UDBAHeroGrowthDefaultsDataAsset;

UCLASS()
class DIVINEBEASTSARENA_API ADBAPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADBAPlayerState();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "DBA|MatchStats")
	void RecordKill(int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "DBA|MatchStats")
	void RecordDeath(int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "DBA|MatchStats")
	void RecordAssist(int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "DBA|MatchStats")
	void AddMatchScore(int32 DeltaScore);

	UFUNCTION(BlueprintCallable, Category = "DBA|MatchStats")
	void AddMatchExpDelta(int64 DeltaExp);

	UFUNCTION(BlueprintCallable, Category = "DBA|MatchStats")
	void SetMatchResult(const FString& InResult);

	UFUNCTION(BlueprintCallable, Category = "DBA|MatchStats")
	void SetMatchTeamId(int32 InTeamId);

	UFUNCTION(BlueprintPure, Category = "DBA|MatchStats")
	int32 GetMatchTeamId() const { return MatchTeamId; }

	UFUNCTION(BlueprintPure, Category = "DBA|MatchStats")
	int32 GetMatchKills() const { return MatchKills; }

	UFUNCTION(BlueprintPure, Category = "DBA|MatchStats")
	int32 GetMatchDeaths() const { return MatchDeaths; }

	UFUNCTION(BlueprintPure, Category = "DBA|MatchStats")
	int32 GetMatchScore() const { return MatchScore; }

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintPure, Category = "DBA|GAS")
	UDBAAbilitySystemComponent* GetDBAAbilitySystemComponent() const;

	UFUNCTION(BlueprintPure, Category = "DBA|GAS")
	UDBABattleAttributeSet* GetBattleAttributeSet() const { return BattleAttributeSet; }

	UFUNCTION(BlueprintPure, Category = "DBA|GAS")
	UDBAHeroGrowthAttributeSet* GetHeroGrowthAttributeSet() const { return HeroGrowthAttributeSet; }

	FDBA_GameBackendRuntimePlayerResult BuildRuntimePlayerResult(const FString& BackendPlayerId) const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDBAAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDBABattleAttributeSet> BattleAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDBAHeroGrowthAttributeSet> HeroGrowthAttributeSet;

	UPROPERTY(ReplicatedUsing = OnRep_MatchStats)
	int32 MatchKills = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchStats)
	int32 MatchDeaths = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchStats)
	int32 MatchAssists = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchStats)
	int32 MatchScore = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchStats)
	int64 MatchExpDelta = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchStats)
	FString MatchResult = TEXT("completed");

	UPROPERTY(ReplicatedUsing = OnRep_MatchStats)
	int32 MatchTeamId = 0;

	UFUNCTION()
	void OnRep_MatchStats();

	void RequestDefaultBattleAttributeDefaultsAsync();
	void HandleDefaultBattleAttributeDefaultsLoaded(UDBABattleAttributeDefaultsDataAsset* LoadedDefaults);

	/** 异步加载英雄成长属性默认值数据资产 */
	void RequestDefaultHeroGrowthDefaultsAsync();
	/** 英雄成长属性默认值加载完成回调 */
	void HandleDefaultHeroGrowthDefaultsLoaded(UDBAHeroGrowthDefaultsDataAsset* LoadedDefaults);
};
