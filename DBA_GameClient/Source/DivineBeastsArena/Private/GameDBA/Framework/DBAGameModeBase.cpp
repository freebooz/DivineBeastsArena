// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Framework/DBAGameModeBase.h"

#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Dog.h"
#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Dragon.h"
#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Goat.h"
#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Horse.h"
#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Monkey.h"
#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Ox.h"
#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Pig.h"
#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Rabbit.h"
#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Rat.h"
#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Rooster.h"
#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Snake.h"
#include "GameDBA/Character/Zodiac/DBAZodiacCharacter_Tiger.h"
#include "GameDBA/Character/Monster/DBALobbyTrainingMonster.h"
#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Framework/DBAUrlOptions.h"
#include "GameDBA/Player/DBAPlayerState.h"
#include "GameDBA/Player/DBALobbyPlayerController.h"
#include "GameDBA/UI/Lobby/Login/DBACharacterPreviewActor.h"
#include "GameDBA/UI/Lobby/Login/DBACharacterPresentationActor.h"
#include "GameBackendClientSubsystem.h"
#include "GameBackendRuntimeService.h"
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
		ADBACharacterPresentationActor::ApplyZodiacMaterialToMesh(MeshComponent, Zodiac, Character);
	}
}

ADBAGameModeBase::ADBAGameModeBase()
{
	PlayerControllerClass = ADBALobbyPlayerController::StaticClass();
	PlayerStateClass = ADBAPlayerState::StaticClass();
	DefaultPawnClass = nullptr;
}

EDBAZodiac ADBAGameModeBase::ResolveLobbyDisplayZodiac(const FString& Options, int32 JoinIndex)
{
	const EDBAZodiac OptionZodiac = ParseZodiacValue(DBAUrlOptions::ExtractUrlOption(Options, TEXT("DBALobbyZodiac")));
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
		for (TActorIterator<ADBACharacterPreviewActor> It(GetWorld()); It; ++It)
		{
			It->Destroy();
		}
		LobbyDisplayActors.Reset();
		SpawnLobbyTrainingMonsters();
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 大厅角色预览 Actor 已禁用，仅使用玩家角色 Pawn。"));
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

FString ADBAGameModeBase::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	const FString ErrorMessage = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
	if (NewPlayerController && GetNetMode() == NM_DedicatedServer)
	{
		BackendRuntimePlayerOptions.Add(TObjectKey<APlayerController>(NewPlayerController), Options);
		BackendRuntimePlayerTeamIds.Add(TObjectKey<APlayerController>(NewPlayerController), ResolveBackendMatchTeamIdFromOptions(Options));
	}

	if (NewPlayerController && IsLobbyMapWorld(GetWorld()) && !IsListenServerLocalControllerOptions(Options))
	{
		const TObjectKey<APlayerController> PlayerKey(NewPlayerController);
		const int32 JoinIndex = NextLobbyJoinIndex++;
		const EDBAZodiac LobbyZodiac = ResolveLobbyDisplayZodiac(Options, JoinIndex);
		LobbyJoinIndices.Add(PlayerKey, JoinIndex);
		LobbyJoinZodiacs.Add(PlayerKey, LobbyZodiac);
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 大厅玩家初始化完成：玩家=%s 序号=%d 生肖=%d 参数=%s"),
			*NewPlayerController->GetName(),
			JoinIndex,
			static_cast<int32>(LobbyZodiac),
			*Options);
	}
	return ErrorMessage;
}

void ADBAGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	FString Options;
	if (NewPlayer)
	{
		if (const FString* StoredOptions = BackendRuntimePlayerOptions.Find(TObjectKey<APlayerController>(NewPlayer)))
		{
			Options = *StoredOptions;
		}
	}
	SyncBackendMatchTeamId(NewPlayer);
	ReportBackendPlayerJoined(NewPlayer, Options);
}

void ADBAGameModeBase::Logout(AController* Exiting)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Exiting))
	{
		ReportBackendPlayerLeft(PlayerController);

		const TObjectKey<APlayerController> PlayerKey(PlayerController);
		if (TWeakObjectPtr<ADBACharacterPreviewActor>* DisplayActorPtr = LobbyDisplayActors.Find(PlayerKey))
		{
			if (ADBACharacterPreviewActor* DisplayActor = DisplayActorPtr->Get())
			{
				DisplayActor->Destroy();
			}
		}

		LobbyDisplayActors.Remove(PlayerKey);
		LobbyJoinIndices.Remove(PlayerKey);
		LobbyJoinZodiacs.Remove(PlayerKey);
		BackendRuntimePlayerIds.Remove(PlayerKey);
		BackendRuntimePlayerOptions.Remove(PlayerKey);
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
		if (UDBA_GameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>() : nullptr)
		{
			if (UDBA_GameBackendRuntimeService* RuntimeService = Backend->GetRuntimeService())
			{
				FDBA_GameBackendResponseDelegate EmptyCallback;
				RuntimeService->NotifyMatchEnded(EmptyCallback);
				bBackendMatchEndedSent = true;
				UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 运行时比赛结束请求已发送。"));
			}
		}
	}

	ReportBackendMatchResults();
}

void ADBAGameModeBase::TryInitializeBackendRuntime()
{
	UDBA_GameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>() : nullptr;
	UDBA_GameBackendRuntimeService* RuntimeService = Backend ? Backend->GetRuntimeService() : nullptr;
	if (!RuntimeService || !RuntimeService->ConfigureFromCommandLine())
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameModeBase] 专用服务器未配置运行时参数，跳过后端运行时注册。"));
		return;
	}

	FDBA_GameBackendResponseDelegate EmptyCallback;
	RuntimeService->RegisterServer(EmptyCallback);
	RuntimeService->MarkReady(EmptyCallback);
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
	UDBA_GameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>() : nullptr;
	UDBA_GameBackendRuntimeService* RuntimeService = Backend ? Backend->GetRuntimeService() : nullptr;
	if (!RuntimeService || !RuntimeService->IsConfigured())
	{
		return;
	}

	FDBA_GameBackendResponseDelegate EmptyCallback;
	RuntimeService->SendHeartbeat(EmptyCallback);
}

void ADBAGameModeBase::ReportBackendMatchStarted()
{
	if (GetNetMode() != NM_DedicatedServer || !bBackendRuntimeReadySent || bBackendMatchStartedSent)
	{
		return;
	}

	if (UDBA_GameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>() : nullptr)
	{
		if (UDBA_GameBackendRuntimeService* RuntimeService = Backend->GetRuntimeService())
		{
			FDBA_GameBackendResponseDelegate EmptyCallback;
			RuntimeService->NotifyMatchStarted(EmptyCallback);
			bBackendMatchStartedSent = true;
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 运行时比赛开始请求已发送。"));
		}
	}
}

void ADBAGameModeBase::ReportBackendPlayerJoined(APlayerController* PlayerController, const FString& Options)
{
	if (!PlayerController || GetNetMode() != NM_DedicatedServer || !bBackendRuntimeReadySent)
	{
		return;
	}

	const FString PlayerId = DBAUrlOptions::ExtractUrlOption(Options, TEXT("PlayerId"));
	const FString PlayerSessionToken = DBAUrlOptions::ExtractUrlOption(Options, TEXT("PlayerSessionToken"));
	if (PlayerId.IsEmpty() || PlayerSessionToken.IsEmpty())
	{
		UE_LOG(LogDBACore, Warning, TEXT("[DBAGameModeBase] 玩家缺少后端连接参数，无法上报运行时玩家加入：启动参数=%s"), *Options);
		return;
	}

	if (UDBA_GameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>() : nullptr)
	{
		if (UDBA_GameBackendRuntimeService* RuntimeService = Backend->GetRuntimeService())
		{
			const TObjectKey<APlayerController> PlayerKey(PlayerController);

			FDBACharacterBuildSummary AdmissionBuildSummary;
			if (!DBAUrlOptions::TryExtractCharacterBuildSummary(Options, AdmissionBuildSummary))
			{
				UE_LOG(LogDBACore, Warning, TEXT("[DBAGameModeBase] 玩家构筑摘要无效，拒绝上报运行时玩家加入：玩家=%s 启动参数=%s"), *PlayerId, *Options);
				return;
			}

			const int32 BackendTeamId = ResolveBackendMatchTeamId(PlayerController, BackendRuntimePlayerTeamIds);
			if (BackendTeamId <= 0)
			{
				UE_LOG(LogDBACore, Warning, TEXT("[DBAGameModeBase] 玩家队伍参数无效，拒绝上报运行时玩家加入：玩家=%s 启动参数=%s"), *PlayerId, *Options);
				return;
			}

			BackendRuntimePlayerIds.Add(PlayerKey, PlayerId);

			FDBA_GameBackendRuntimePlayerBuildSummary BuildSummary;
			BuildSummary.Zodiac = DBACharacterBuild::ToStableZodiacName(AdmissionBuildSummary.Zodiac);
			BuildSummary.PrimaryElement = DBACharacterBuild::ToStableElementName(AdmissionBuildSummary.PrimaryElement);
			BuildSummary.FiveCamp = ToStableFiveCampName(AdmissionBuildSummary.FiveCamp);
			BuildSummary.FixedSkillGroupId = AdmissionBuildSummary.FixedSkillGroupId.ToString();

			FDBA_GameBackendResponseDelegate EmptyCallback;
			const FString BackendRuntimeTeam = BuildBackendRuntimeTeamName(BackendTeamId);
			RuntimeService->NotifyPlayerJoined(PlayerId, PlayerSessionToken, BackendRuntimeTeam, 0, BuildSummary, EmptyCallback);
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 运行时玩家加入请求已发送：玩家=%s 队伍=%s 生肖=%s 元素=%s 固定技能组=%s"),
				*PlayerId,
				*BackendRuntimeTeam,
				*BuildSummary.Zodiac,
				*BuildSummary.PrimaryElement,
				*BuildSummary.FixedSkillGroupId);
		}
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

	if (UDBA_GameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>() : nullptr)
	{
		if (UDBA_GameBackendRuntimeService* RuntimeService = Backend->GetRuntimeService())
		{
			FDBA_GameBackendResponseDelegate EmptyCallback;
			RuntimeService->NotifyPlayerLeft(*PlayerId, EmptyCallback);
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 运行时玩家离开请求已发送：玩家=%s"), **PlayerId);
		}
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

	UDBA_GameBackendClientSubsystem* Backend = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBA_GameBackendClientSubsystem>() : nullptr;
	UDBA_GameBackendRuntimeService* RuntimeService = Backend ? Backend->GetRuntimeService() : nullptr;
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
	FDBA_GameBackendResponseDelegate EmptyCallback;
	RuntimeService->NotifyMatchResults(BackendMatchResultIdempotencyKey, ResultJson, PlayerResults, EmptyCallback);
	bBackendMatchResultsSent = true;
	UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] 运行时比赛结果请求已发送：玩家数=%d 幂等键=%s"),
		PlayerResults.Num(),
		*BackendMatchResultIdempotencyKey);
}

void ADBAGameModeBase::SpawnOrUpdateLobbyDisplayForPlayer(APlayerController* PlayerController)
{
	if (PlayerController)
	{
		const TObjectKey<APlayerController> PlayerKey(PlayerController);
		if (TWeakObjectPtr<ADBACharacterPreviewActor>* ExistingActor = LobbyDisplayActors.Find(PlayerKey))
		{
			if (ADBACharacterPreviewActor* DisplayActor = ExistingActor->Get())
			{
				DisplayActor->Destroy();
			}
			LobbyDisplayActors.Remove(PlayerKey);
		}
	}
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
	}

	// Fallback for listen/local controller paths that do not carry lobby option maps.
	return ResolveLobbyPawnClass(EDBAZodiac::Rat);
}

UClass* ADBAGameModeBase::ResolveLobbyPawnClass(EDBAZodiac Zodiac) const
{
	switch (Zodiac)
	{
	case EDBAZodiac::Rat: return ADBAZodiacCharacter_Rat::StaticClass();
	case EDBAZodiac::Ox: return ADBAZodiacCharacter_Ox::StaticClass();
	case EDBAZodiac::Tiger: return ADBAZodiacCharacter_Tiger::StaticClass();
	case EDBAZodiac::Rabbit: return ADBAZodiacCharacter_Rabbit::StaticClass();
	case EDBAZodiac::Dragon: return ADBAZodiacCharacter_Dragon::StaticClass();
	case EDBAZodiac::Snake: return ADBAZodiacCharacter_Snake::StaticClass();
	case EDBAZodiac::Horse: return ADBAZodiacCharacter_Horse::StaticClass();
	case EDBAZodiac::Goat: return ADBAZodiacCharacter_Goat::StaticClass();
	case EDBAZodiac::Monkey: return ADBAZodiacCharacter_Monkey::StaticClass();
	case EDBAZodiac::Rooster: return ADBAZodiacCharacter_Rooster::StaticClass();
	case EDBAZodiac::Dog: return ADBAZodiacCharacter_Dog::StaticClass();
	case EDBAZodiac::Pig: return ADBAZodiacCharacter_Pig::StaticClass();
	default: return DefaultPawnClass ? DefaultPawnClass.Get() : ADBAZodiacCharacter_Rat::StaticClass();
	}
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
			EDBAZodiac PawnZodiac = EDBAZodiac::Rat;
			if (APlayerController* PC = Cast<APlayerController>(NewPlayer))
			{
				const TObjectKey<APlayerController> PlayerKey(PC);
				if (const EDBAZodiac* ResolvedZodiac = LobbyJoinZodiacs.Find(PlayerKey))
				{
					PawnZodiac = *ResolvedZodiac;
				}
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
