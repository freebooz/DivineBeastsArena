// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端 / Dedicated Server。
- 文件职责：实现单局战斗统计的复制、累加和 Runtime 结算 DTO 映射。
- 阅读重点：Dedicated Server 是权威写入方，客户端复制仅用于后续 HUD/记分板展示。
- 修改提示：保持这里无后端 HTTP 调用，网络提交由 GameMode / RuntimeService 负责。
*/

#include "GameDBA/Player/DBAPlayerState.h"

#include "AbilitySystemComponent.h"
#include "GameDBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "GameDBA/GAS/Attributes/DBAHeroGrowthAttributeSet.h"
#include "GameDBA/GAS/DBAAbilitySystemComponent.h"
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

	DOREPLIFETIME(ADBAPlayerState, MatchKills);
	DOREPLIFETIME(ADBAPlayerState, MatchDeaths);
	DOREPLIFETIME(ADBAPlayerState, MatchAssists);
	DOREPLIFETIME(ADBAPlayerState, MatchScore);
	DOREPLIFETIME(ADBAPlayerState, MatchExpDelta);
	DOREPLIFETIME(ADBAPlayerState, MatchResult);
	DOREPLIFETIME(ADBAPlayerState, MatchTeamId);
}

void ADBAPlayerState::OnRep_MatchStats()
{
}
