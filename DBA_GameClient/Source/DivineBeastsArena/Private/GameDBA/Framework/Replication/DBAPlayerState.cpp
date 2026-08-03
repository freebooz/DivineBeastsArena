// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端 / Dedicated Server。
- 文件职责：实现单局战斗统计的复制、累加和 Runtime 结算 DTO 映射。
- 阅读重点：Dedicated Server 是权威写入方，客户端复制仅用于后续 HUD/记分板展示。
- 修改提示：保持这里无后端 HTTP 调用，网络提交由 GameMode / RuntimeService 负责。
*/

#include "GameDBA/Framework/Replication/DBAPlayerState.h"

#include "AbilitySystemComponent.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBABattleAttributeDeveloperSettings.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBABattleAttributeDefaultsDataAsset.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBABattleAttributeSet.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBAHeroGrowthAttributeSet.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBAHeroGrowthDefaultsDataAsset.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBAHeroGrowthDeveloperSettings.h"
#include "GameDBA/Gameplay/GAS/DBAAbilitySystemComponent.h"
#include "GameCore/Async/DBAAsyncAssetLoader.h"
#include "Net/UnrealNetwork.h"

ADBAPlayerState::ADBAPlayerState()
{
	bReplicates = true;

	AbilitySystemComponent = CreateDefaultSubobject<UDBAAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	BattleAttributeSet = CreateDefaultSubobject<UDBABattleAttributeSet>(TEXT("BattleAttributeSet"));
	HeroGrowthAttributeSet = CreateDefaultSubobject<UDBAHeroGrowthAttributeSet>(TEXT("HeroGrowthAttributeSet"));
}

void ADBAPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		RequestDefaultBattleAttributeDefaultsAsync();
		RequestDefaultHeroGrowthDefaultsAsync();
	}
}

void ADBAPlayerState::RequestDefaultBattleAttributeDefaultsAsync()
{
	const UDBABattleAttributeDeveloperSettings* Settings = GetDefault<UDBABattleAttributeDeveloperSettings>();
	const TSoftObjectPtr<UDBABattleAttributeDefaultsDataAsset> DefaultsAsset = Settings
		? Settings->DefaultBattleAttributeDefaults
		: TSoftObjectPtr<UDBABattleAttributeDefaultsDataAsset>();
	if (DefaultsAsset.IsNull())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[玩家状态] 未配置默认战斗属性数据资产：请在 DBA 战斗属性设置中配置 DefaultBattleAttributeDefaults。"));
		return;
	}

	if (UDBABattleAttributeDefaultsDataAsset* LoadedDefaults = DefaultsAsset.Get())
	{
		HandleDefaultBattleAttributeDefaultsLoaded(LoadedDefaults);
		return;
	}

	TWeakObjectPtr<ADBAPlayerState> WeakThis(this);
	DBAAsyncAssetLoader::RequestAsyncAsset<UDBABattleAttributeDefaultsDataAsset>(this, DefaultsAsset, [WeakThis](UDBABattleAttributeDefaultsDataAsset* LoadedDefaults)
	{
		if (ADBAPlayerState* StrongThis = WeakThis.Get())
		{
			StrongThis->HandleDefaultBattleAttributeDefaultsLoaded(LoadedDefaults);
		}
	});
}

void ADBAPlayerState::HandleDefaultBattleAttributeDefaultsLoaded(UDBABattleAttributeDefaultsDataAsset* LoadedDefaults)
{
	if (!HasAuthority())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[玩家状态] 客户端收到默认战斗属性加载回调，已拒绝写入复制属性。"));
		return;
	}

	if (!LoadedDefaults)
	{
		UE_LOG(LogDBACombat, Error, TEXT("[玩家状态] 默认战斗属性数据资产异步加载失败。"));
		return;
	}

	if (!BattleAttributeSet)
	{
		UE_LOG(LogDBACombat, Error, TEXT("[玩家状态] 应用默认战斗属性失败：BattleAttributeSet 不可用。"));
		return;
	}

	BattleAttributeSet->ApplyDefaultAttributes(LoadedDefaults);
}

void ADBAPlayerState::RequestDefaultHeroGrowthDefaultsAsync()
{
	const UDBAHeroGrowthDeveloperSettings* Settings = GetDefault<UDBAHeroGrowthDeveloperSettings>();
	const TSoftObjectPtr<UDBAHeroGrowthDefaultsDataAsset> DefaultsAsset = Settings
		? Settings->DefaultHeroGrowthDefaults
		: TSoftObjectPtr<UDBAHeroGrowthDefaultsDataAsset>();
	if (DefaultsAsset.IsNull())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[玩家状态] 未配置默认英雄成长属性数据资产：请在 DBA 英雄成长属性设置中配置 DefaultHeroGrowthDefaults。"));
		return;
	}

	// 已加载则直接应用
	if (UDBAHeroGrowthDefaultsDataAsset* LoadedDefaults = DefaultsAsset.Get())
	{
		HandleDefaultHeroGrowthDefaultsLoaded(LoadedDefaults);
		return;
	}

	// 异步加载并回调
	TWeakObjectPtr<ADBAPlayerState> WeakThis(this);
	DBAAsyncAssetLoader::RequestAsyncAsset<UDBAHeroGrowthDefaultsDataAsset>(this, DefaultsAsset, [WeakThis](UDBAHeroGrowthDefaultsDataAsset* LoadedDefaults)
	{
		if (ADBAPlayerState* StrongThis = WeakThis.Get())
		{
			StrongThis->HandleDefaultHeroGrowthDefaultsLoaded(LoadedDefaults);
		}
	});
}

void ADBAPlayerState::HandleDefaultHeroGrowthDefaultsLoaded(UDBAHeroGrowthDefaultsDataAsset* LoadedDefaults)
{
	if (!HasAuthority())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[玩家状态] 客户端收到默认英雄成长属性加载回调，已拒绝写入复制属性。"));
		return;
	}

	if (!LoadedDefaults)
	{
		UE_LOG(LogDBACombat, Error, TEXT("[玩家状态] 默认英雄成长属性数据资产异步加载失败。"));
		return;
	}

	if (!HeroGrowthAttributeSet)
	{
		UE_LOG(LogDBACombat, Error, TEXT("[玩家状态] 应用默认英雄成长属性失败：HeroGrowthAttributeSet 不可用。"));
		return;
	}

	HeroGrowthAttributeSet->ApplyDefaultAttributes(LoadedDefaults);
}

void ADBAPlayerState::RecordKill(int32 Count)
{
	if (!HasAuthority())
	{
		return;
	}

	MatchKills += FMath::Max(0, Count);
}

void ADBAPlayerState::RecordDeath(int32 Count)
{
	if (!HasAuthority())
	{
		return;
	}

	MatchDeaths += FMath::Max(0, Count);
}

void ADBAPlayerState::RecordAssist(int32 Count)
{
	if (!HasAuthority())
	{
		return;
	}

	MatchAssists += FMath::Max(0, Count);
}

void ADBAPlayerState::AddMatchScore(int32 DeltaScore)
{
	if (!HasAuthority())
	{
		return;
	}

	MatchScore = FMath::Max(0, MatchScore + DeltaScore);
}

void ADBAPlayerState::AddMatchExpDelta(int64 DeltaExp)
{
	if (!HasAuthority())
	{
		return;
	}

	MatchExpDelta = FMath::Max<int64>(0, MatchExpDelta + DeltaExp);
}

void ADBAPlayerState::SetMatchResult(const FString& InResult)
{
	if (!HasAuthority())
	{
		return;
	}

	MatchResult = InResult.IsEmpty() ? TEXT("completed") : InResult;
}

void ADBAPlayerState::SetMatchTeamId(int32 InTeamId)
{
	if (!HasAuthority())
	{
		return;
	}

	MatchTeamId = FMath::Max(0, InTeamId);
}

UAbilitySystemComponent* ADBAPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UDBAAbilitySystemComponent* ADBAPlayerState::GetDBAAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FDBA_GameBackendRuntimePlayerResult ADBAPlayerState::BuildRuntimePlayerResult(const FString& BackendPlayerId) const
{
	FDBA_GameBackendRuntimePlayerResult Result;
	Result.PlayerId = BackendPlayerId;
	Result.Team = MatchTeamId > 0 ? FString::Printf(TEXT("Team%d"), MatchTeamId) : TEXT("");
	Result.Result = MatchResult.IsEmpty() ? TEXT("completed") : MatchResult;
	Result.Kills = MatchKills;
	Result.Deaths = MatchDeaths;
	Result.Assists = MatchAssists;
	Result.Score = MatchScore;
	Result.ExpDelta = MatchExpDelta;
	return Result;
}

void ADBAPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ADBAPlayerState, MatchKills, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ADBAPlayerState, MatchDeaths, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ADBAPlayerState, MatchAssists, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ADBAPlayerState, MatchScore, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ADBAPlayerState, MatchExpDelta, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ADBAPlayerState, MatchResult, COND_OwnerOnly);
	DOREPLIFETIME(ADBAPlayerState, MatchTeamId);
}

void ADBAPlayerState::OnRep_MatchStats()
{
}
