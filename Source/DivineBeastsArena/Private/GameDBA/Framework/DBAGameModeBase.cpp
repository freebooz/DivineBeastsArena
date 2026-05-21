// Copyright Freebooz Games, Inc. All Rights Reserved.

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
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Player/DBALobbyPlayerController.h"
#include "GameDBA/UI/Lobby/Login/DBACharacterPreviewActor.h"
#include "GameDBA/UI/Lobby/Login/DBACharacterPresentationActor.h"
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

	FString ExtractUrlOption(const FString& Options, const FString& Key)
	{
		TArray<FString> Parts;
		Options.ParseIntoArray(Parts, TEXT("?"), true);

		TArray<FString> AmpersandParts;
		for (const FString& Part : Parts)
		{
			TArray<FString> SplitParts;
			Part.ParseIntoArray(SplitParts, TEXT("&"), true);
			AmpersandParts.Append(SplitParts);
		}

		const FString Prefix = Key + TEXT("=");
		for (FString Part : AmpersandParts)
		{
			Part.TrimStartAndEndInline();
			if (Part.StartsWith(Prefix, ESearchCase::IgnoreCase))
			{
				return Part.RightChop(Prefix.Len()).TrimStartAndEnd();
			}
		}

		return FString();
	}

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

	bool IsLobbyMapWorld(const UWorld* World)
	{
		if (!World || !World->PersistentLevel)
		{
			return false;
		}

		const FString LevelPath = World->PersistentLevel->GetOutermost()->GetName();
		return LevelPath.Contains(TEXT("LobbyMap")) || LevelPath.Contains(TEXT("MainLobby"));
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
	DefaultPawnClass = nullptr;
}

EDBAZodiac ADBAGameModeBase::ResolveLobbyDisplayZodiac(const FString& Options, int32 JoinIndex)
{
	const EDBAZodiac OptionZodiac = ParseZodiacValue(ExtractUrlOption(Options, TEXT("DBALobbyZodiac")));
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

	UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] BeginPlay - DivineBeastsArena started"));

	if (GetNetMode() == NM_DedicatedServer)
	{
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] Running in dedicated server mode"));
	}

	if (IsLobbyMapWorld(GetWorld()))
	{
		for (TActorIterator<ADBACharacterPreviewActor> It(GetWorld()); It; ++It)
		{
			It->Destroy();
		}
		LobbyDisplayActors.Reset();
		SpawnLobbyTrainingMonsters();
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] Lobby character preview actors disabled; using player pawns only."));
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

	const FVector BaseLocation(680.0f, -900.0f, 96.0f);
	constexpr float MonsterSpacingX = 420.0f;
	constexpr float MonsterSpacingY = 420.0f;
	constexpr int32 MonsterCount = 10;
	constexpr int32 MonstersPerRow = 5;

	for (int32 Index = 0; Index < MonsterCount; ++Index)
	{
		const int32 Row = Index / MonstersPerRow;
		const int32 Column = Index % MonstersPerRow;
		const FVector SpawnLocation = BaseLocation + FVector(
			static_cast<float>(Row) * MonsterSpacingX,
			static_cast<float>(Column) * MonsterSpacingY,
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
			UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] Spawned lobby training monster: index=%d actor=%s location=%s"),
				Index,
				*Monster->GetName(),
				*SpawnLocation.ToString());
		}
	}
}

FString ADBAGameModeBase::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	const FString ErrorMessage = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
	if (NewPlayerController && IsLobbyMapWorld(GetWorld()) && !IsListenServerLocalControllerOptions(Options))
	{
		const TObjectKey<APlayerController> PlayerKey(NewPlayerController);
		const int32 JoinIndex = NextLobbyJoinIndex++;
		const EDBAZodiac LobbyZodiac = ResolveLobbyDisplayZodiac(Options, JoinIndex);
		LobbyJoinIndices.Add(PlayerKey, JoinIndex);
		LobbyJoinZodiacs.Add(PlayerKey, LobbyZodiac);
		UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] Lobby player initialized: %s index=%d zodiac=%d options=%s"),
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
}

void ADBAGameModeBase::Logout(AController* Exiting)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Exiting))
	{
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
	}

	Super::Logout(Exiting);
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
				UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] Skip local host pawn spawn for headless lobby server: %s"), *PC->GetName());
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

		UE_LOG(LogDBACore, Log, TEXT("[DBAGameModeBase] SpawnDefaultPawnAtTransform (AlwaysSpawn): player=%s pawn=%s class=%s"),
			*NewPlayer->GetName(),
			*SpawnedPawn->GetName(),
			*PawnClass->GetName());
	}
	return SpawnedPawn;
}
