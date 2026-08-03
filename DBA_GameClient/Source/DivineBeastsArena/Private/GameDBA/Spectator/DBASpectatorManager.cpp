// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Spectator/DBASpectatorManager.h"

#include "GameDBA/Characters/DBAZodiacCharacterBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

UDBASpectatorManager::UDBASpectatorManager()
	: Super()
	, bIsPaused(false)
{
}

void UDBASpectatorManager::OnSubsystemInitialize()
{
	// P1-1 改造：项目基类统一调用 Super::Initialize，此处仅执行派生类初始化
	UpdateObserverViewTargets();
}

void UDBASpectatorManager::OnSubsystemDeinitialize()
{
	// P1-1 改造：项目基类统一调用 Super::Deinitialize，此处仅清理派生类状态
	ObserverMap.Empty();
	MatchPlayers.Empty();
}

bool UDBASpectatorManager::ConnectObserver(APlayerController* ObserverController, EDBAObserverControlLevel ControlLevel)
{
	if (!ObserverController)
	{
		return false;
	}

	FDBAObserverInfo Info;
	if (APlayerState* PlayerState = ObserverController->PlayerState)
	{
		Info.ObserverID = PlayerState->GetUniqueId();
	}
	Info.ObserverName = FName(ObserverController->GetName());
	Info.ControlLevel = ControlLevel;
	Info.CurrentViewTargetIndex = MatchPlayers.Num() > 0 ? 0 : INDEX_NONE;
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
	if (MatchID.IsEmpty())
	{
		return false;
	}

	CurrentMatchID = MatchID;
	UpdateObserverViewTargets();
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
	FDBAObserverInfo* Info = GetObserverEntry(ObserverController);
	const int32 PlayerCount = MatchPlayers.Num();
	if (!Info || PlayerCount == 0)
	{
		return;
	}

	const int32 OldIndex = Info->CurrentViewTargetIndex;
	const int32 NewIndex = OldIndex == INDEX_NONE ? 0 : (OldIndex + 1) % PlayerCount;
	Info->CurrentViewTargetIndex = NewIndex;
	OnViewTargetChanged.Broadcast(OldIndex, NewIndex);
}

void UDBASpectatorManager::CycleToPreviousTarget(APlayerController* ObserverController)
{
	FDBAObserverInfo* Info = GetObserverEntry(ObserverController);
	const int32 PlayerCount = MatchPlayers.Num();
	if (!Info || PlayerCount == 0)
	{
		return;
	}

	const int32 OldIndex = Info->CurrentViewTargetIndex;
	const int32 NewIndex = OldIndex == INDEX_NONE || OldIndex <= 0 ? PlayerCount - 1 : OldIndex - 1;
	Info->CurrentViewTargetIndex = NewIndex;
	OnViewTargetChanged.Broadcast(OldIndex, NewIndex);
}

bool UDBASpectatorManager::SetViewTargetByIndex(APlayerController* ObserverController, int32 TargetIndex)
{
	FDBAObserverInfo* Info = GetObserverEntry(ObserverController);
	if (!Info || !MatchPlayers.IsValidIndex(TargetIndex))
	{
		return false;
	}

	const int32 OldIndex = Info->CurrentViewTargetIndex;
	Info->CurrentViewTargetIndex = TargetIndex;
	OnViewTargetChanged.Broadcast(OldIndex, TargetIndex);
	return true;
}

FDBAObserverViewTarget UDBASpectatorManager::GetCurrentViewTarget(APlayerController* ObserverController) const
{
	FDBAObserverViewTarget EmptyTarget;
	const FDBAObserverInfo* Info = GetObserverEntry(ObserverController);
	if (!Info || !MatchPlayers.IsValidIndex(Info->CurrentViewTargetIndex))
	{
		return EmptyTarget;
	}

	const FDBAObserverPlayerEntry& Entry = MatchPlayers[Info->CurrentViewTargetIndex];
	ADBAZodiacCharacterBase* Character = Entry.Character.Get();
	if (!Character)
	{
		return EmptyTarget;
	}

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
		if (ADBAZodiacCharacterBase* Character = Entry.Character.Get())
		{
			FDBAObserverViewTarget Target;
			Character->GetSpectatorData(Target);
			Result.Add(Target);
		}
	}
	return Result;
}

void UDBASpectatorManager::SetViewMode(APlayerController* ObserverController, EDBAObserverViewMode ViewMode)
{
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
	if (!RequesterController || !ObserverToKick || !HasControlLevel(RequesterController, EDBAObserverControlLevel::Kick))
	{
		return false;
	}

	DisconnectObserver(ObserverToKick);
	return true;
}

bool UDBASpectatorManager::HasControlLevel(APlayerController* ObserverController, EDBAObserverControlLevel RequiredLevel) const
{
	const FDBAObserverInfo* Info = GetObserverEntry(ObserverController);
	return Info && static_cast<uint8>(Info->ControlLevel) >= static_cast<uint8>(RequiredLevel);
}

FDBAObserverInfo* UDBASpectatorManager::GetObserverEntry(APlayerController* ObserverController)
{
	return ObserverController ? ObserverMap.Find(ObserverController) : nullptr;
}

const FDBAObserverInfo* UDBASpectatorManager::GetObserverEntry(APlayerController* ObserverController) const
{
	return ObserverController ? ObserverMap.Find(ObserverController) : nullptr;
}

void UDBASpectatorManager::UpdateObserverViewTargets()
{
	MatchPlayers.Empty();
	if (UWorld* World = GetWorld())
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(World, ADBAZodiacCharacterBase::StaticClass(), FoundActors);
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
	for (const TPair<TObjectPtr<APlayerController>, FDBAObserverInfo>& Pair : ObserverMap)
	{
		FDBAObserverViewTarget Target = GetCurrentViewTarget(Pair.Key);
		(void)Target;
	}
}

TArray<ADBAZodiacCharacterBase*> UDBASpectatorManager::GetMatchPlayers() const
{
	TArray<ADBAZodiacCharacterBase*> Result;
	for (const FDBAObserverPlayerEntry& Entry : MatchPlayers)
	{
		if (ADBAZodiacCharacterBase* Character = Entry.Character.Get())
		{
			Result.Add(Character);
		}
	}
	return Result;
}

bool UDBASpectatorManager::CanPause(APlayerController* RequesterController) const
{
	return HasControlLevel(RequesterController, EDBAObserverControlLevel::Pause);
}