// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Framework/GameModes/DBAGameModeBase.h"

#include "GameDBA/Data/Registries/DBAZodiacCharacterRegistry.h"
#include "GameDBA/Characters/Monster/DBALobbyTrainingMonster.h"
#include "GameDBA/Characters/DBAZodiacCharacterBase.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Presentation/Visual/DBAZodiacVisualDeveloperSettings.h"
#include "GameDBA/Framework/Travel/DBAUrlOptions.h"
#include "GameDBA/Framework/Replication/DBAPlayerState.h"
#include "GameDBA/Framework/Server/DBAServerRuntimeSubsystem.h"
#include "GameDBA/Frontend/Lobby/DBALobbyPlayerController.h"
#include "GameDBA/Frontend/CharacterSelection/DBACharacterPresentationActor.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

namespace
{
	const EDBAZodiac LobbyDisplayOrder[] = {
		EDBAZodiac::Rat,
		EDBAZodiac::Ox,
		EDBAZodiac::Tiger,
		EDBAZodiac::Rabbit,
		EDBAZodiac::Dragon,
		EDBAZodiac::Snake,
		EDBAZodiac::Horse,
		EDBAZodiac::Goat,
		EDBAZodiac::Monkey,
		EDBAZodiac::Rooster,
		EDBAZodiac::Dog,
		EDBAZodiac::Pig
	};

	EDBAZodiac ParseZodiacValue(const FString& RawValue)
	{
		if (RawValue.IsEmpty())
		{
			return EDBAZodiac::None;
		}

		if (RawValue.IsNumeric())
		{
			const int32 NumericValue = FCString::Atoi(*RawValue);
			if (NumericValue >= static_cast<int32>(EDBAZodiac::Rat)
				&& NumericValue <= static_cast<int32>(EDBAZodiac::Pig))
			{
				return static_cast<EDBAZodiac>(NumericValue);
			}
		}

		struct FZodiacName
		{
			const TCHAR* Name;
			EDBAZodiac Zodiac;
		};

		const FZodiacName Names[] = {
			{ TEXT("Rat"), EDBAZodiac::Rat },
			{ TEXT("Ox"), EDBAZodiac::Ox },
			{ TEXT("Tiger"), EDBAZodiac::Tiger },
			{ TEXT("Rabbit"), EDBAZodiac::Rabbit },
			{ TEXT("Dragon"), EDBAZodiac::Dragon },
			{ TEXT("Snake"), EDBAZodiac::Snake },
			{ TEXT("Horse"), EDBAZodiac::Horse },
			{ TEXT("Goat"), EDBAZodiac::Goat },
			{ TEXT("Monkey"), EDBAZodiac::Monkey },
			{ TEXT("Rooster"), EDBAZodiac::Rooster },
			{ TEXT("Dog"), EDBAZodiac::Dog },
			{ TEXT("Pig"), EDBAZodiac::Pig }
		};

		for (const FZodiacName& Entry : Names)
		{
			if (RawValue.Equals(Entry.Name, ESearchCase::IgnoreCase))
			{
				return Entry.Zodiac;
			}
		}

		return EDBAZodiac::None;
	}

	const TCHAR* ToStableFiveCampName(EDBAFiveCamp FiveCamp)
	{
		switch (FiveCamp)
		{
		case EDBAFiveCamp::East:
			return TEXT("East");
		case EDBAFiveCamp::West:
			return TEXT("West");
		case EDBAFiveCamp::South:
			return TEXT("South");
		case EDBAFiveCamp::North:
			return TEXT("North");
		case EDBAFiveCamp::Center:
			return TEXT("Center");
		default:
			return TEXT("None");
		}
	}

	bool IsLobbyMapWorld(const UWorld* World)
	{
		if (!World || !World->PersistentLevel)
		{
			return false;
		}

		const FString LevelPath = World->PersistentLevel->GetOutermost()->GetName();
		return LevelPath.Contains(TEXT("LobbyMap")) || LevelPath.Contains(TEXT("MainLobby"));
	}

	int32 CompareBackendMatchResultRank(
		const FDBA_GameBackendRuntimePlayerResult& Left,
		const FDBA_GameBackendRuntimePlayerResult& Right)
	{
		if (Left.Score != Right.Score)
		{
			return Left.Score > Right.Score ? 1 : -1;
		}
		if (Left.Kills != Right.Kills)
		{
			return Left.Kills > Right.Kills ? 1 : -1;
		}
		if (Left.Deaths != Right.Deaths)
		{
			return Left.Deaths < Right.Deaths ? 1 : -1;
		}
		return 0;
	}

	bool HasBackendMatchResultActivity(const FDBA_GameBackendRuntimePlayerResult& PlayerResult)
	{
		return PlayerResult.Score > 0
			|| PlayerResult.Kills > 0
			|| PlayerResult.Assists > 0
			|| PlayerResult.Deaths > 0;
	}

	int32 ResolveBackendMatchTeamIdFromOptions(const FString& Options)
	{
		int32 TeamId = 0;
		return DBAUrlOptions::TryExtractTeamId(Options, TeamId) ? TeamId : 0;
	}

	int32 ResolveBackendMatchTeamId(const APlayerController* PlayerController, const TMap<TObjectKey<APlayerController>, int32>& StoredTeamIds)
	{
		if (!PlayerController)
		{
			return 0;
		}

		const int32 StoredTeamId = StoredTeamIds.FindRef(TObjectKey<APlayerController>(PlayerController));
		if (StoredTeamId > 0)
		{
			return StoredTeamId;
		}

		if (const ADBAZodiacCharacterBase* ZodiacCharacter = Cast<ADBAZodiacCharacterBase>(PlayerController->GetPawn()))
		{
			return FMath::Max(0, ZodiacCharacter->GetTeamID());
		}

		return 0;
	}

	FString BuildBackendRuntimeTeamName(int32 TeamId)
	{
		switch (TeamId)
		{
		case 1:
			return TEXT("blue");
		case 2:
			return TEXT("red");
		default:
			return TeamId > 0 ? FString::FromInt(TeamId) : FString();
		}
	}

	struct FBackendMatchTeamScore
	{
		int32 Score = 0;
		int32 Kills = 0;
		int32 Deaths = 0;
		bool bHasActivity = false;
	};

	struct FBackendMatchTeamOutcome
	{
		FString WinnerPlayerId;
		FString WinnerTeam;
		bool bUsedTeamOutcome = false;
	};

	int32 CompareBackendMatchTeamScore(const FBackendMatchTeamScore& Left, const FBackendMatchTeamScore& Right)
	{
		if (Left.Score != Right.Score)
		{
			return Left.Score > Right.Score ? 1 : -1;
		}
		if (Left.Kills != Right.Kills)
		{
			return Left.Kills > Right.Kills ? 1 : -1;
		}
		if (Left.Deaths != Right.Deaths)
		{
			return Left.Deaths < Right.Deaths ? 1 : -1;
		}
		return 0;
	}

	FBackendMatchTeamOutcome BuildBackendMatchTeamOutcome(TArray<FDBA_GameBackendRuntimePlayerResult>& PlayerResults)
	{
		FBackendMatchTeamOutcome Outcome;
		TMap<FString, FBackendMatchTeamScore> TeamScores;

		for (const FDBA_GameBackendRuntimePlayerResult& PlayerResult : PlayerResults)
		{
			if (PlayerResult.Team.IsEmpty())
			{
				continue;
			}

			FBackendMatchTeamScore& TeamScore = TeamScores.FindOrAdd(PlayerResult.Team);
			TeamScore.Score += PlayerResult.Score;
			TeamScore.Kills += PlayerResult.Kills;
			TeamScore.Deaths += PlayerResult.Deaths;
			TeamScore.bHasActivity = TeamScore.bHasActivity || HasBackendMatchResultActivity(PlayerResult);
		}

		if (TeamScores.Num() < 2)
		{
			return Outcome;
		}

		FString BestTeam;
		FBackendMatchTeamScore BestScore;
		bool bHasActivity = false;
		bool bHasTie = false;
		bool bHasBestTeam = false;

		for (const TPair<FString, FBackendMatchTeamScore>& Entry : TeamScores)
		{
			bHasActivity = bHasActivity || Entry.Value.bHasActivity;
			if (!bHasBestTeam)
			{
				BestTeam = Entry.Key;
				BestScore = Entry.Value;
				bHasBestTeam = true;
				bHasTie = false;
				continue;
			}

			const int32 RankComparison = CompareBackendMatchTeamScore(Entry.Value, BestScore);
			if (RankComparison > 0)
			{
				BestTeam = Entry.Key;
				BestScore = Entry.Value;
				bHasTie = false;
			}
			else if (RankComparison == 0)
			{
				bHasTie = true;
			}
		}

		Outcome.bUsedTeamOutcome = true;
		if (!bHasActivity || bHasTie)
		{
			for (FDBA_GameBackendRuntimePlayerResult& PlayerResult : PlayerResults)
			{
				PlayerResult.Result = TEXT("draw");
			}
			return Outcome;
		}

		Outcome.WinnerTeam = BestTeam;
		for (FDBA_GameBackendRuntimePlayerResult& PlayerResult : PlayerResults)
		{
			if (PlayerResult.Team == BestTeam)
			{
				PlayerResult.Result = TEXT("win");
				if (Outcome.WinnerPlayerId.IsEmpty())
				{
					Outcome.WinnerPlayerId = PlayerResult.PlayerId;
				}
			}
			else
			{
				PlayerResult.Result = TEXT("loss");
			}
		}
		return Outcome;
	}

	FBackendMatchTeamOutcome ApplyBackendMatchResultsOutcome(TArray<FDBA_GameBackendRuntimePlayerResult>& PlayerResults)
	{
		FBackendMatchTeamOutcome TeamOutcome = BuildBackendMatchTeamOutcome(PlayerResults);
		if (TeamOutcome.bUsedTeamOutcome)
		{
			return TeamOutcome;
		}

		int32 BestIndex = INDEX_NONE;
		bool bHasActivity = false;
		bool bHasTie = false;

		for (int32 Index = 0; Index < PlayerResults.Num(); ++Index)
		{
			const FDBA_GameBackendRuntimePlayerResult& PlayerResult = PlayerResults[Index];
			bHasActivity = bHasActivity || HasBackendMatchResultActivity(PlayerResult);

			if (BestIndex == INDEX_NONE)
			{
				BestIndex = Index;
				bHasTie = false;
				continue;
			}

			const int32 RankComparison = CompareBackendMatchResultRank(PlayerResult, PlayerResults[BestIndex]);
			if (RankComparison > 0)
			{
				BestIndex = Index;
				bHasTie = false;
			}
			else if (RankComparison == 0)
			{
				bHasTie = true;
			}
		}

		if (!bHasActivity || BestIndex == INDEX_NONE || bHasTie)
		{
			for (FDBA_GameBackendRuntimePlayerResult& PlayerResult : PlayerResults)
			{
				PlayerResult.Result = TEXT("draw");
			}
			return FBackendMatchTeamOutcome();
		}

		FBackendMatchTeamOutcome Outcome;
		Outcome.WinnerPlayerId = PlayerResults[BestIndex].PlayerId;
		for (FDBA_GameBackendRuntimePlayerResult& PlayerResult : PlayerResults)
		{
			if (PlayerResult.PlayerId == Outcome.WinnerPlayerId)
			{
				PlayerResult.Result = TEXT("win");
			}
			else
			{
				PlayerResult.Result = TEXT("loss");
			}
		}
		return Outcome;
	}

	FString EscapeBackendMatchResultsJsonString(const FString& Value)
	{
		FString Escaped = Value;
		Escaped.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
		Escaped.ReplaceInline(TEXT("\""), TEXT("\\\""));
		return Escaped;
	}

	FString BuildBackendMatchResultsJson(const FBackendMatchTeamOutcome& Outcome)
	{
		const FString WinnerPlayerValue = Outcome.WinnerPlayerId.IsEmpty()
			? TEXT("null")
			: FString::Printf(TEXT("\"%s\""), *EscapeBackendMatchResultsJsonString(Outcome.WinnerPlayerId));
		const FString WinnerTeamValue = Outcome.WinnerTeam.IsEmpty()
			? TEXT("null")
			: FString::Printf(TEXT("\"%s\""), *EscapeBackendMatchResultsJsonString(Outcome.WinnerTeam));
		return FString::Printf(
			TEXT("{\"winnerPlayerId\":%s,\"winnerTeam\":%s,\"source\":\"DedicatedServer\",\"schema\":\"mvp-stat-outcome\"}"),
			*WinnerPlayerValue,
			*WinnerTeamValue);
	}

	bool IsListenServerLocalControllerOptions(const FString& Options)
	{
		return Options.Contains(TEXT("?listen"), ESearchCase::IgnoreCase)
			|| Options.Equals(TEXT("listen"), ESearchCase::IgnoreCase);
	}

	EDBAZodiac ResolveLobbyZodiacFromWorldUrl(const UWorld* World)
	{
		if (!World)
		{
			return EDBAZodiac::None;
		}

		const TCHAR* ZodiacOption = World->URL.GetOption(TEXT("DBALobbyZodiac="), TEXT(""));
		return ParseZodiacValue(ZodiacOption ? FString(ZodiacOption) : FString());
	}

	void ApplyLobbyPawnVisuals(ACharacter* Character, EDBAZodiac Zodiac)
	{
		if (!Character || !Character->GetMesh())
		{
			return;
		}

		USkeletalMeshComponent* MeshComponent = Character->GetMesh();
		MeshComponent->SetVisibility(true);
		MeshComponent->SetHiddenInGame(false);
		MeshComponent->SetComponentTickEnabled(true);
		MeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
		if (ADBAZodiacCharacterBase* ZodiacCharacter = Cast<ADBAZodiacCharacterBase>(Character))
		{
			ZodiacCharacter->SetLobbyDisplayZodiac(Zodiac);
			return;
		}

		if (Zodiac != EDBAZodiac::None)
		{
			ADBACharacterPresentationActor::ApplyZodiacMaterialToMesh(MeshComponent, Zodiac, Character);
		}
	}
}

ADBAGameModeBase::ADBAGameModeBase()
{
	PlayerControllerClass = ADBALobbyPlayerController::StaticClass();
	PlayerStateClass = ADBAPlayerState::StaticClass();
	DefaultPawnClass = nullptr;

	// 开启无缝旅行，降低地图切换时的客户端断线与卡顿
	bUseSeamlessTravel = true;
}

EDBAZodiac ADBAGameModeBase::ResolveLobbyDisplayZodiac(const FString& Options, int32 JoinIndex)
{
	EDBAZodiac OptionZodiac = ParseZodiacValue(DBAUrlOptions::ExtractUrlOption(Options, TEXT("DBAZodiac")));
	if (OptionZodiac == EDBAZodiac::None)
	{
		OptionZodiac = ParseZodiacValue(DBAUrlOptions::ExtractUrlOption(Options, TEXT("DBALobbyZodiac")));
	}
	if (OptionZodiac != EDBAZodiac::None)
	{
		return OptionZodiac;
	}

	const int32 SafeIndex = FMath::Max(0, JoinIndex);
	return LobbyDisplayOrder[SafeIndex % UE_ARRAY_COUNT(LobbyDisplayOrder)];
}

FTransform ADBAGameModeBase::GetLobbyDisplayTransform(int32 JoinIndex)
{
	const int32 SafeIndex = FMath::Max(0, JoinIndex);
	const int32 PairIndex = SafeIndex / 2;
	const float Side = (SafeIndex % 2 == 0) ? -1.0f : 1.0f;
	const FVector Location(160.0f - PairIndex * 88.0f, Side * (82.0f + PairIndex * 20.0f), 0.0f);
	return FTransform(FRotator(0.0f, 180.0f, 0.0f), Location, FVector(0.58f));
}

void ADBAGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 游戏模式开始运行，DivineBeastsArena 已启动。"));

	if (GetNetMode() == NM_DedicatedServer)
	{
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 当前运行模式：独立服务器。"));
		TryInitializeBackendRuntime();
	}

	if (IsLobbyMapWorld(GetWorld()))
	{
		SpawnLobbyTrainingMonsters();
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 大厅仅使用玩家角色 Pawn，不创建额外角色展示 Actor。"));
	}
}

void ADBAGameModeBase::SpawnLobbyTrainingMonsters()
{
	if (!HasAuthority() || !GetWorld())
	{
		return;
	}

	for (TActorIterator<ADBALobbyTrainingMonster> It(GetWorld()); It; ++It)
	{
		It->Destroy();
	}
	LobbyTrainingMonsters.Reset();

	const FVector FormationCenter(1300.0f, -900.0f, 96.0f);
	constexpr float MonsterSpacingX = 1250.0f;
	constexpr float MonsterSpacingY = 1250.0f;
	constexpr int32 MonsterCount = 10;
	constexpr int32 MonstersPerRow = 5;
	constexpr float CenteredRowOffset = 0.5f;
	constexpr float CenteredColumnOffset = 2.0f;

	for (int32 Index = 0; Index < MonsterCount; ++Index)
	{
		const int32 Row = Index / MonstersPerRow;
		const int32 Column = Index % MonstersPerRow;
		const FVector SpawnLocation = FormationCenter + FVector(
			(static_cast<float>(Row) - CenteredRowOffset) * MonsterSpacingX,
			(static_cast<float>(Column) - CenteredColumnOffset) * MonsterSpacingY,
			0.0f);
		const FRotator SpawnRotation(0.0f, 180.0f + static_cast<float>(Column) * 12.0f, 0.0f);
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ADBALobbyTrainingMonster* Monster = GetWorld()->SpawnActor<ADBALobbyTrainingMonster>(
			ADBALobbyTrainingMonster::StaticClass(),
			SpawnLocation,
			SpawnRotation,
			SpawnParams);

		if (Monster)
		{
			Monster->ConfigureLobbyMonster(Index);
			LobbyTrainingMonsters.Add(Monster);
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 已生成大厅训练怪物：序号=%d Actor=%s 位置=%s"),
				Index,
				*Monster->GetName(),
				*SpawnLocation.ToString());
		}
	}
}

void ADBAGameModeBase::PreLoginAsync(
	const FString& Options,
	const FString& Address,
	const FUniqueNetIdRepl& UniqueId,
	const FOnPreLoginCompleteDelegate& OnComplete)
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		Super::PreLoginAsync(Options, Address, UniqueId, OnComplete);
		return;
	}

	FString BaseErrorMessage;
	PreLogin(Options, Address, UniqueId, BaseErrorMessage);
	if (!BaseErrorMessage.IsEmpty())
	{
		OnComplete.ExecuteIfBound(BaseErrorMessage);
		return;
	}

	UDBAServerRuntimeSubsystem* RuntimeService = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDBAServerRuntimeSubsystem>()
		: nullptr;
	if (!RuntimeService || !RuntimeService->IsConfigured())
	{
		UE_LOG(LogDBACore, Error, TEXT("[DBAGameModeBase] 独立服务器未完成后台运行时配置，拒绝玩家连接。"));
		OnComplete.ExecuteIfBound(TEXT("大厅服务器尚未完成后台注册。"));
		return;
	}

	const FString PlayerId = DBAUrlOptions::ExtractUrlOption(Options, TEXT("PlayerId"));
	const FString CharacterId = DBAUrlOptions::ExtractUrlOption(Options, TEXT("CharacterId"));
	FString JoinTicket = DBAUrlOptions::ExtractUrlOption(Options, TEXT("JoinTicket"));
	if (JoinTicket.IsEmpty())
	{
		JoinTicket = DBAUrlOptions::ExtractUrlOption(Options, TEXT("PlayerSessionToken"));
	}
	FDBACharacterBuildSummary AdmissionBuildSummary;
	const bool bHasValidBuildSummary = DBAUrlOptions::TryExtractCharacterBuildSummary(Options, AdmissionBuildSummary);
	if (PlayerId.IsEmpty() || CharacterId.IsEmpty() || JoinTicket.IsEmpty() || !bHasValidBuildSummary)
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameModeBase] 入服请求缺少玩家标识、角色标识、一次性票据或角色构筑摘要。"));
		OnComplete.ExecuteIfBound(TEXT("入服连接参数不完整。"));
		return;
	}

	const int32 TeamId = ResolveBackendMatchTeamIdFromOptions(Options);
	FDBA_GameBackendRuntimePlayerBuildSummary BuildSummary;
	BuildSummary.Zodiac = AdmissionBuildSummary.ZodiacId.ToString();
	BuildSummary.PrimaryElement = AdmissionBuildSummary.PrimaryElementId.ToString();
	BuildSummary.FiveCamp = AdmissionBuildSummary.FiveCampId.ToString();
	BuildSummary.FixedSkillGroupId = AdmissionBuildSummary.FixedSkillGroupId.ToString();

	RuntimeService->ValidateJoinTicket(
		PlayerId,
		CharacterId,
		JoinTicket,
		BuildBackendRuntimeTeamName(TeamId),
		0,
		BuildSummary,
		[OnComplete, PlayerId](bool bSuccess, const FString& ErrorMessage, const FString&)
		{
			if (!bSuccess)
			{
				UE_LOG(LogDBACore, Warning, TEXT("[DBAGameModeBase] 一次性入服票据验证失败：玩家=%s 原因=%s"), *PlayerId, *ErrorMessage);
				OnComplete.ExecuteIfBound(ErrorMessage.IsEmpty() ? TEXT("一次性入服票据验证失败。") : ErrorMessage);
				return;
			}

			UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 一次性入服票据验证通过：玩家=%s。"), *PlayerId);
			OnComplete.ExecuteIfBound(FString());
		});
}

FString ADBAGameModeBase::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	const FString ErrorMessage = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	int32 LobbyJoinIndex = INDEX_NONE;
	if (NewPlayerController && IsLobbyMapWorld(GetWorld()) && !IsListenServerLocalControllerOptions(Options))
	{
		const TObjectKey<APlayerController> PlayerKey(NewPlayerController);
		LobbyJoinIndex = NextLobbyJoinIndex++;
		const EDBAZodiac LobbyZodiac = ResolveLobbyDisplayZodiac(Options, LobbyJoinIndex);
		LobbyJoinIndices.Add(PlayerKey, LobbyJoinIndex);
		LobbyJoinZodiacs.Add(PlayerKey, LobbyZodiac);
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 大厅玩家初始化完成：玩家=%s 序号=%d 生肖=%d"),
			*NewPlayerController->GetName(),
			LobbyJoinIndex,
			static_cast<int32>(LobbyZodiac));
	}

	if (NewPlayerController && GetNetMode() == NM_DedicatedServer)
	{
		const FString PlayerId = DBAUrlOptions::ExtractUrlOption(Options, TEXT("PlayerId"));
		if (!PlayerId.IsEmpty())
		{
			BackendRuntimePlayerIds.Add(TObjectKey<APlayerController>(NewPlayerController), PlayerId);
		}

		int32 BackendTeamId = ResolveBackendMatchTeamIdFromOptions(Options);
		if (BackendTeamId <= 0 && LobbyJoinIndex != INDEX_NONE)
		{
			BackendTeamId = (LobbyJoinIndex % 2) + 1;
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 连接参数未包含队伍标识，已按大厅加入序号分配默认队伍：序号=%d 队伍=%d"),
				LobbyJoinIndex,
				BackendTeamId);
		}

		BackendRuntimePlayerTeamIds.Add(TObjectKey<APlayerController>(NewPlayerController), BackendTeamId);
	}

	return ErrorMessage;
}

void ADBAGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	SyncBackendMatchTeamId(NewPlayer);
}

void ADBAGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (!NewPlayer || !IsLobbyMapWorld(GetWorld()) || Cast<ADBAZodiacCharacterBase>(NewPlayer->GetPawn()))
	{
		return;
	}

	if (APawn* ResidualPawn = NewPlayer->GetPawn())
	{
		NewPlayer->UnPossess();
		ResidualPawn->Destroy();
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 已清理地图切换遗留 Pawn：玩家=%s Pawn=%s。"),
			*NewPlayer->GetName(),
			*ResidualPawn->GetName());
	}

	RestartPlayer(NewPlayer);
	if (!Cast<ADBAZodiacCharacterBase>(NewPlayer->GetPawn()))
	{
		const int32 JoinIndex = LobbyJoinIndices.FindRef(TObjectKey<APlayerController>(NewPlayer));
		FTransform FallbackTransform = GetLobbyDisplayTransform(JoinIndex);
		FallbackTransform.SetScale3D(FVector::OneVector);
		RestartPlayerAtTransform(NewPlayer, FallbackTransform);
	}

	if (ADBAZodiacCharacterBase* LobbyPawn = Cast<ADBAZodiacCharacterBase>(NewPlayer->GetPawn()))
	{
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 大厅玩家角色已生成并完成占有：玩家=%s Pawn=%s。"),
			*NewPlayer->GetName(),
			*LobbyPawn->GetName());
	}
	else
	{
		UE_LOG(LogDBACore, Error, TEXT("[DBAGameModeBase] 大厅玩家角色生成失败：玩家=%s。"), *NewPlayer->GetName());
	}
}

void ADBAGameModeBase::Logout(AController* Exiting)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Exiting))
	{
		ReportBackendPlayerLeft(PlayerController);

		const TObjectKey<APlayerController> PlayerKey(PlayerController);
		LobbyJoinIndices.Remove(PlayerKey);
		LobbyJoinZodiacs.Remove(PlayerKey);
		BackendRuntimePlayerIds.Remove(PlayerKey);
		BackendRuntimePlayerTeamIds.Remove(PlayerKey);
	}

	Super::Logout(Exiting);
}

void ADBAGameModeBase::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	ReportBackendMatchStarted();
}

void ADBAGameModeBase::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	if (GetNetMode() == NM_DedicatedServer && bBackendRuntimeReadySent && !bBackendMatchEndedSent)
	{
		if (UDBAServerRuntimeSubsystem* RuntimeService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAServerRuntimeSubsystem>() : nullptr)
		{
			RuntimeService->NotifyMatchEnded();
			bBackendMatchEndedSent = true;
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 运行时比赛结束请求已发送。"));
		}
	}

	ReportBackendMatchResults();
}

void ADBAGameModeBase::TryInitializeBackendRuntime()
{
	UDBAServerRuntimeSubsystem* RuntimeService = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDBAServerRuntimeSubsystem>()
		: nullptr;
	if (!RuntimeService || !RuntimeService->ConfigureAndRegister())
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameModeBase] 专用服务器未配置运行时参数，跳过后端运行时注册。"));
		return;
	}

	bBackendRuntimeReadySent = true;
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 运行时注册和就绪请求已发送。"));
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			BackendRuntimeHeartbeatTimerHandle,
			this,
			&ADBAGameModeBase::SendBackendHeartbeat,
			20.0f,
			true);
	}
}

void ADBAGameModeBase::SendBackendHeartbeat()
{
	UDBAServerRuntimeSubsystem* RuntimeService = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDBAServerRuntimeSubsystem>()
		: nullptr;
	if (!RuntimeService || !RuntimeService->IsConfigured())
	{
		return;
	}

	RuntimeService->SendHeartbeat();
}

void ADBAGameModeBase::ReportBackendMatchStarted()
{
	if (GetNetMode() != NM_DedicatedServer || !bBackendRuntimeReadySent || bBackendMatchStartedSent)
	{
		return;
	}

	if (UDBAServerRuntimeSubsystem* RuntimeService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAServerRuntimeSubsystem>() : nullptr)
	{
		RuntimeService->NotifyMatchStarted();
		bBackendMatchStartedSent = true;
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 运行时比赛开始请求已发送。"));
	}
}

void ADBAGameModeBase::SyncBackendMatchTeamId(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	const TObjectKey<APlayerController> PlayerKey(PlayerController);
	const int32 StoredTeamId = BackendRuntimePlayerTeamIds.FindRef(PlayerKey);
	if (StoredTeamId <= 0)
	{
		return;
	}

	if (ADBAPlayerState* DBAPlayerState = Cast<ADBAPlayerState>(PlayerController->PlayerState))
	{
		DBAPlayerState->SetMatchTeamId(StoredTeamId);
	}

	if (ADBAZodiacCharacterBase* ZodiacCharacter = Cast<ADBAZodiacCharacterBase>(PlayerController->GetPawn()))
	{
		ZodiacCharacter->SetTeamID(StoredTeamId);
	}
}

void ADBAGameModeBase::ReportBackendPlayerLeft(APlayerController* PlayerController)
{
	if (!PlayerController || GetNetMode() != NM_DedicatedServer)
	{
		return;
	}

	const TObjectKey<APlayerController> PlayerKey(PlayerController);
	const FString* PlayerId = BackendRuntimePlayerIds.Find(PlayerKey);
	if (!PlayerId || PlayerId->IsEmpty())
	{
		return;
	}

	if (UDBAServerRuntimeSubsystem* RuntimeService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAServerRuntimeSubsystem>() : nullptr)
	{
		RuntimeService->NotifyPlayerLeft(*PlayerId);
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 运行时玩家离开请求已发送：玩家=%s"), **PlayerId);
	}
}

void ADBAGameModeBase::ReportBackendMatchResults()
{
	if (GetNetMode() != NM_DedicatedServer || !bBackendRuntimeReadySent || bBackendMatchResultsSent)
	{
		return;
	}

	if (BackendRuntimePlayerIds.IsEmpty())
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameModeBase] 运行时比赛结果跳过：没有已上报玩家加入的玩家。"));
		return;
	}

	UDBAServerRuntimeSubsystem* RuntimeService = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDBAServerRuntimeSubsystem>()
		: nullptr;
	if (!RuntimeService || !RuntimeService->IsConfigured())
	{
		return;
	}

	if (BackendMatchResultIdempotencyKey.IsEmpty())
	{
		BackendMatchResultIdempotencyKey = FString::Printf(TEXT("ue-match-result-%s"), *FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens));
	}

	TArray<FDBA_GameBackendRuntimePlayerResult> PlayerResults;
	PlayerResults.Reserve(BackendRuntimePlayerIds.Num());
	for (const TPair<TObjectKey<APlayerController>, FString>& Entry : BackendRuntimePlayerIds)
	{
		if (Entry.Value.IsEmpty())
		{
			continue;
		}

		FDBA_GameBackendRuntimePlayerResult PlayerResult;
		if (APlayerController* PlayerController = Entry.Key.ResolveObjectPtr())
		{
			if (ADBAPlayerState* DBAPlayerState = Cast<ADBAPlayerState>(PlayerController->PlayerState))
			{
				DBAPlayerState->SetMatchTeamId(ResolveBackendMatchTeamId(PlayerController, BackendRuntimePlayerTeamIds));
				PlayerResult = DBAPlayerState->BuildRuntimePlayerResult(Entry.Value);
			}
		}

		if (PlayerResult.PlayerId.IsEmpty())
		{
			PlayerResult.PlayerId = Entry.Value;
			PlayerResult.Team = TEXT("");
			PlayerResult.Result = TEXT("draw");
			PlayerResult.Kills = 0;
			PlayerResult.Deaths = 0;
			PlayerResult.Assists = 0;
			PlayerResult.Score = 0;
			PlayerResult.ExpDelta = 0;
		}
		PlayerResults.Add(PlayerResult);
	}

	if (PlayerResults.IsEmpty())
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameModeBase] 运行时比赛结果跳过：玩家结果为空。"));
		return;
	}

	const FBackendMatchTeamOutcome MatchOutcome = ApplyBackendMatchResultsOutcome(PlayerResults);
	const FString ResultJson = BuildBackendMatchResultsJson(MatchOutcome);
	RuntimeService->NotifyMatchResults(BackendMatchResultIdempotencyKey, ResultJson, PlayerResults);
	bBackendMatchResultsSent = true;
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 运行时比赛结果请求已发送：玩家数=%d 幂等键=%s"),
		PlayerResults.Num(),
		*BackendMatchResultIdempotencyKey);
}

UClass* ADBAGameModeBase::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (!InController)
	{
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	if (!IsLobbyMapWorld(GetWorld()))
	{
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	if (APlayerController* PC = Cast<APlayerController>(InController))
	{
		const TObjectKey<APlayerController> PlayerKey(PC);
		if (LobbyJoinZodiacs.Contains(PlayerKey))
		{
			if (UClass* PawnClass = ResolveLobbyPawnClass(LobbyJoinZodiacs[PlayerKey]))
			{
				return PawnClass;
			}
		}

		const EDBAZodiac TravelZodiac = ResolveLobbyZodiacFromWorldUrl(GetWorld());
		if (TravelZodiac != EDBAZodiac::None)
		{
			LobbyJoinZodiacs.FindOrAdd(PlayerKey) = TravelZodiac;
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 从本地大厅旅行参数恢复生肖：玩家=%s 生肖=%d"),
				*PC->GetName(),
				static_cast<int32>(TravelZodiac));
			return ResolveLobbyPawnClass(TravelZodiac);
		}
	}

	UE_LOG(LogDBACore, Warning, TEXT("[DBAGameModeBase] 大厅 Pawn 生成未收到生肖旅行参数，使用注册表默认 Pawn。"));
	return ResolveLobbyPawnClass(EDBAZodiac::None);
}

UClass* ADBAGameModeBase::ResolveLobbyPawnClass(EDBAZodiac Zodiac) const
{
	if (const UDBAZodiacVisualDeveloperSettings* VisualSettings = GetDefault<UDBAZodiacVisualDeveloperSettings>())
	{
		if (VisualSettings->bUseTintedPlaceholderMesh)
		{
			// 当前阶段全生肖共用同一个 C++ 角色类和骨骼网格；生肖差异由 DataTable 染色驱动。
			return ADBAZodiacCharacterBase::StaticClass();
		}
	}

	// 优先从生肖角色注册表数据资产查询，遵循数据驱动原则
	if (const UDBAZodiacCharacterRegistry* Registry = ZodiacCharacterRegistry.LoadSynchronous())
	{
		if (TSubclassOf<ADBAZodiacCharacterBase> CharacterClass = Registry->GetCharacterClassForZodiac(Zodiac))
		{
			return CharacterClass;
		}
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameModeBase] 生肖角色注册表中未配置生肖 %d 的角色类，回退到 DefaultPawnClass。"), static_cast<int32>(Zodiac));
	}
	else
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameModeBase] 生肖角色注册表数据资产未配置，请在 GameMode 默认设置中指定 ZodiacCharacterRegistry。"));
	}

	// 回退到 DefaultPawnClass（由父类或蓝图配置）
	if (DefaultPawnClass)
	{
		return DefaultPawnClass.Get();
	}

	// 开发/联机验证兜底：未配置注册表与 DefaultPawnClass 时仍允许生成通用生肖角色
	return ADBAZodiacCharacterBase::StaticClass();
}

APawn* ADBAGameModeBase::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	if (!NewPlayer)
	{
		return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
	}

	if (!IsLobbyMapWorld(GetWorld()))
	{
		return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
	}

	// Headless lobby server should not spawn an extra local host pawn.
	if (FParse::Param(FCommandLine::Get(), TEXT("DBAHeadlessLobbyServer")))
	{
		if (APlayerController* PC = Cast<APlayerController>(NewPlayer))
		{
			if (PC->IsLocalController())
			{
				UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 无头大厅服务器跳过本地主机 Pawn 生成：%s"), *PC->GetName());
				return nullptr;
			}
		}
	}

	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
	if (!PawnClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = NewPlayer;
	SpawnInfo.Instigator = NewPlayer->GetPawn();
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo);
	if (SpawnedPawn)
	{
		SpawnedPawn->SetReplicates(true);
		SpawnedPawn->SetReplicateMovement(true);

		if (ACharacter* SpawnedCharacter = Cast<ACharacter>(SpawnedPawn))
		{
			EDBAZodiac PawnZodiac = EDBAZodiac::None;
			if (APlayerController* PC = Cast<APlayerController>(NewPlayer))
			{
				const TObjectKey<APlayerController> PlayerKey(PC);
				if (const EDBAZodiac* ResolvedZodiac = LobbyJoinZodiacs.Find(PlayerKey))
				{
					PawnZodiac = *ResolvedZodiac;
				}
				else
				{
					PawnZodiac = ResolveLobbyZodiacFromWorldUrl(GetWorld());
				}
			}

			if (PawnZodiac == EDBAZodiac::None)
			{
				UE_LOG(LogDBACore, Warning, TEXT("[DBAGameModeBase] 大厅 Pawn 未获得有效生肖，跳过生肖染色：Pawn=%s"), *SpawnedPawn->GetName());
			}
			ApplyLobbyPawnVisuals(SpawnedCharacter, PawnZodiac);
		}

		if (APlayerController* PC = Cast<APlayerController>(NewPlayer))
		{
			SyncBackendMatchTeamId(PC);
		}

		UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 已生成默认 Pawn（强制生成）：玩家=%s Pawn=%s 类=%s"),
			*NewPlayer->GetName(),
			*SpawnedPawn->GetName(),
			*PawnClass->GetName());
	}
	return SpawnedPawn;
}
