// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Spectator/DBASpectatorManager.h"
#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/Framework/DBAGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameInstance.h"

UDBASpectatorManager::UDBASpectatorManager()
	: Super()
	, bIsPaused(false)
{
}

void UDBASpectatorManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 获取当前Match的玩家
	if (UWorld* World = GetWorld())
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(World, ADBAZodiacCharacterBase::StaticClass(), FoundActors);

		MatchPlayers.Reserve(FoundActors.Num());
		for (AActor* Actor : FoundActors)
		{
			if (ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(Actor))
			{
				MatchPlayers.Add(FDBAObserverPlayerEntry(Character));
			}
		}
	}
}

void UDBASpectatorManager::Deinitialize()
{
	ObserverMap.Empty();
	MatchPlayers.Empty();
	Super::Deinitialize();
}

bool UDBASpectatorManager::ConnectObserver(APlayerController* ObserverController, EDBAObserverControlLevel ControlLevel)
{
	if (!ObserverController)
	{
		return false;
	}

	FDBAObserverInfo Info;
	Info.ObserverID = ObserverController->GetUniqueID();
	Info.ObserverName = FName(ObserverController->GetName());
	Info.ControlLevel = ControlLevel;
	Info.CurrentViewTargetIndex = 0;
	Info.ViewMode = EDBAObserverViewMode::Follow;

	ObserverMap.Add(ObserverController, Info);

	OnObserverConnected.Broadcast(Info);

	return true;
}

void UDBASpectatorManager::DisconnectObserver(APlayerController* ObserverController)
{
	if (!ObserverController)
	{
		return;
	}

	if (FDBAObserverInfo* Info = ObserverMap.Find(ObserverController))
	{
		FUniqueNetIdRepl DisconnectedID = Info->ObserverID;
		ObserverMap.Remove(ObserverController);
		OnObserverDisconnected.Broadcast(DisconnectedID);
	}
}

bool UDBASpectatorManager::ConnectToMatch(APlayerController* ObserverController, FString MatchID, EDBAObserverControlLevel ControlLevel)
{
	// 验证MatchID (这里简化处理，实际需要验证Match是否存在)
	if (MatchID.IsEmpty())
	{
		return false;
	}

	CurrentMatchID = MatchID;
	return ConnectObserver(ObserverController, ControlLevel);
}

TArray<FDBAObserverInfo> UDBASpectatorManager::GetObserverList() const
{
	TArray<FDBAObserverInfo> Result;
	ObserverMap.GenerateValueArray(Result);
	return Result;
}

FDBAObserverInfo UDBASpectatorManager::GetObserverInfo(APlayerController* ObserverController) const
{
	if (const FDBAObserverInfo* Info = ObserverMap.Find(ObserverController))
	{
		return *Info;
	}
	return FDBAObserverInfo();
}

void UDBASpectatorManager::CycleToNextTarget(APlayerController* ObserverController)
{
	if (!ObserverController)
	{
		return;
	}

	FDBAObserverInfo* Info = GetObserverEntry(ObserverController);
	if (!Info)
	{
		return;
	}

	// 获取玩家数量
	int32 PlayerCount = MatchPlayers.Num();
	if (PlayerCount == 0)
	{
		return;
	}

	// 计算下一个索引
	int32 OldIndex = Info->CurrentViewTargetIndex;
	int32 NewIndex = (OldIndex + 1) % PlayerCount;

	Info->CurrentViewTargetIndex = NewIndex;
	OnViewTargetChanged.Broadcast(OldIndex, NewIndex);
}

void UDBASpectatorManager::CycleToPreviousTarget(APlayerController* ObserverController)
{
	if (!ObserverController)
	{
		return;
	}

	FDBAObserverInfo* Info = GetObserverEntry(ObserverController);
	if (!Info)
	{
		return;
	}

	int32 PlayerCount = MatchPlayers.Num();
	if (PlayerCount == 0)
	{
		return;
	}

	int32 OldIndex = Info->CurrentViewTargetIndex;
	int32 NewIndex = OldIndex - 1;
	if (NewIndex < 0)
	{
		NewIndex = PlayerCount - 1;
	}

	Info->CurrentViewTargetIndex = NewIndex;
	OnViewTargetChanged.Broadcast(OldIndex, NewIndex);
}

bool UDBASpectatorManager::SetViewTargetByIndex(APlayerController* ObserverController, int32 TargetIndex)
{
	if (!ObserverController)
	{
		return false;
	}

	FDBAObserverInfo* Info = GetObserverEntry(ObserverController);
	if (!Info)
	{
		return false;
	}

	int32 PlayerCount = MatchPlayers.Num();
	if (PlayerCount == 0 || TargetIndex < 0 || TargetIndex >= PlayerCount)
	{
		return false;
	}

	int32 OldIndex = Info->CurrentViewTargetIndex;
	Info->CurrentViewTargetIndex = TargetIndex;
	OnViewTargetChanged.Broadcast(OldIndex, TargetIndex);

	return true;
}

FDBAObserverViewTarget UDBASpectatorManager::GetCurrentViewTarget(APlayerController* ObserverController) const
{
	FDBAObserverViewTarget EmptyTarget;

	if (!ObserverController)
	{
		return EmptyTarget;
	}

	const FDBAObserverInfo* Info = GetObserverEntry(ObserverController);
	if (!Info)
	{
		return EmptyTarget;
	}

	if (Info->CurrentViewTargetIndex < 0 || Info->CurrentViewTargetIndex >= MatchPlayers.Num())
	{
		return EmptyTarget;
	}

	const FDBAObserverPlayerEntry& Entry = MatchPlayers[Info->CurrentViewTargetIndex];
	if (!Entry.Character.IsValid())
	{
		return EmptyTarget;
	}

	ADBAZodiacCharacterBase* Character = Entry.Character.Get();
	FDBAObserverViewTarget Target;
	Character->GetSpectatorData(Target);
	return Target;
}

TArray<FDBAObserverViewTarget> UDBASpectatorManager::GetAllViewTargets() const
{
	TArray<FDBAObserverViewTarget> Result;
	Result.Reserve(MatchPlayers.Num());

	for (const FDBAObserverPlayerEntry& Entry : MatchPlayers)
	{
		if (!Entry.Character.IsValid())
		{
			continue;
		}

		ADBAZodiacCharacterBase* Character = Entry.Character.Get();
		FDBAObserverViewTarget Target;
		Character->GetSpectatorData(Target);
		Result.Add(Target);
	}

	return Result;
}

void UDBASpectatorManager::SetViewMode(APlayerController* ObserverController, EDBAObserverViewMode ViewMode)
{
	if (!ObserverController)
	{
		return;
	}

	if (FDBAObserverInfo* Info = GetObserverEntry(ObserverController))
	{
		Info->ViewMode = ViewMode;
	}
}

bool UDBASpectatorManager::TogglePause(APlayerController* RequesterController)
{
	if (!CanPause(RequesterController))
	{
		return false;
	}

	bIsPaused = !bIsPaused;
	OnMatchPaused.Broadcast(bIsPaused);

	return true;
}

bool UDBASpectatorManager::KickObserver(APlayerController* RequesterController, APlayerController* ObserverToKick)
{
	if (!RequesterController || !ObserverToKick)
	{
		return false;
	}

	// 检查请求者权限
	if (!HasControlLevel(RequesterController, EDBAObserverControlLevel::Kick))
	{
		return false;
	}

	DisconnectObserver(ObserverToKick);
	return true;
}

bool UDBASpectatorManager::HasControlLevel(APlayerController* ObserverController, EDBAObserverControlLevel RequiredLevel) const
{
	if (!ObserverController)
	{
		return false;
	}

	const FDBAObserverInfo* Info = GetObserverEntry(ObserverController);
	if (!Info)
	{
		return false;
	}

	return static_cast<uint8>(Info->ControlLevel) >= static_cast<uint8>(RequiredLevel);
}

FDBAObserverInfo* UDBASpectatorManager::GetObserverEntry(APlayerController* ObserverController)
{
	return ObserverMap.Find(ObserverController);
}

const FDBAObserverInfo* UDBASpectatorManager::GetObserverEntry(APlayerController* ObserverController) const
{
	return ObserverMap.Find(ObserverController);
}

void UDBASpectatorManager::UpdateObserverViewTargets()
{
	// 刷新MatchPlayers列表
	if (UWorld* World = GetWorld())
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(World, ADBAZodiacCharacterBase::StaticClass(), FoundActors);

		MatchPlayers.Empty();
		for (AActor* Actor : FoundActors)
		{
			if (ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(Actor))
			{
				MatchPlayers.Add(FDBAObserverPlayerEntry(Character));
			}
		}
	}
}

void UDBASpectatorManager::BroadcastViewTargetUpdate()
{
	// 广播视角数据更新到所有观战者
	for (const TPair<TObjectPtr<APlayerController>, FDBAObserverInfo>& Pair : ObserverMap)
	{
		FDBAObserverViewTarget Target = GetCurrentViewTarget(Pair.Key);
		// TODO: 通过RPC发送到观战者客户端
	}
}

TArray<ADBAZodiacCharacterBase*> UDBASpectatorManager::GetMatchPlayers() const
{
	TArray<ADBAZodiacCharacterBase*> Result;
	for (const FDBAObserverPlayerEntry& Entry : MatchPlayers)
	{
		if (Entry.Character.IsValid())
		{
			Result.Add(Entry.Character.Get());
		}
	}
	return Result;
}

bool UDBASpectatorManager::CanPause(APlayerController* RequesterController) const
{
	return HasControlLevel(RequesterController, EDBAObserverControlLevel::Pause);
}
