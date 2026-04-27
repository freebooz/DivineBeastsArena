// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/Party/DBAPartySubsystem.h"
#include "Frontend/Invite/DBAInviteServiceStub.h"
#include "Common/DBALogChannels.h"

UDBAPartySubsystem::UDBAPartySubsystem()
{
}

void UDBAPartySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    InviteServiceStub = NewObject<UDBAInviteServiceStub>(this);

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 初始化完成"));
}

void UDBAPartySubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UDBAPartySubsystem::CreateParty()
{
    if (IsInParty())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 已在 Party 中，无法创建新 Party"));
        return;
    }

    CurrentPartyInfo.PartyId = FGuid::NewGuid().ToString();
    CurrentPartyInfo.State = EDBAPartySubsystemState::InLobby;
    CurrentPartyInfo.MaxMembers = 5;
    CurrentPartyInfo.Members.Empty();

    FDBAPartySubsystemMemberInfo LeaderInfo;
    LeaderInfo.PlayerId = GetCurrentPlayerId();
    LeaderInfo.DisplayName = TEXT("本地玩家");
    LeaderInfo.Level = 1;
    LeaderInfo.Role = EDBAPartySubsystemMemberRole::Leader;
    LeaderInfo.bIsReady = false;
    LeaderInfo.bIsOnline = true;

    CurrentPartyInfo.Members.Add(LeaderInfo);

    SetPartyState(EDBAPartySubsystemState::InLobby);

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 创建 Party: %s"), *CurrentPartyInfo.PartyId);
}

void UDBAPartySubsystem::DisbandParty()
{
    if (!IsInParty())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 当前不在 Party 中"));
        return;
    }

    if (!IsPartyLeader())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 只有队长可以解散 Party"));
        return;
    }

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 解散 Party: %s"), *CurrentPartyInfo.PartyId);

    CurrentPartyInfo = FDBAPartySubsystemInfo();
    SetPartyState(EDBAPartySubsystemState::None);
}

void UDBAPartySubsystem::LeaveParty()
{
    if (!IsInParty())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 当前不在 Party 中"));
        return;
    }

    FString CurrentPlayerId = GetCurrentPlayerId();

    if (IsPartyLeader())
    {
        DisbandParty();
    }
    else
    {
        CurrentPartyInfo.Members.RemoveAll([&CurrentPlayerId](const FDBAPartySubsystemMemberInfo& Member)
        {
            return Member.PlayerId == CurrentPlayerId;
        });

        OnPartyMemberLeft.Broadcast(CurrentPlayerId);

        CurrentPartyInfo = FDBAPartySubsystemInfo();
        SetPartyState(EDBAPartySubsystemState::None);
    }

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 离开 Party"));
}

void UDBAPartySubsystem::InvitePlayer(const FString& PlayerId)
{
    if (!IsInParty())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 当前不在 Party 中"));
        return;
    }

    if (!IsPartyLeader())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 只有队长可以邀请玩家"));
        return;
    }

    if (CurrentPartyInfo.IsFull())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] Party 已满员"));
        return;
    }

    if (InviteServiceStub)
    {
        InviteServiceStub->SendPartyInvite(PlayerId, CurrentPartyInfo.PartyId);
    }

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 邀请玩家: %s"), *PlayerId);
}

void UDBAPartySubsystem::AcceptPartyInvite(const FString& PartyId)
{
    if (IsInParty())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 已在 Party 中，无法接受邀请"));
        return;
    }

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 接受 Party 邀请: %s"), *PartyId);
}

void UDBAPartySubsystem::DeclinePartyInvite(const FString& PartyId)
{
    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 拒绝 Party 邀请: %s"), *PartyId);
}

void UDBAPartySubsystem::KickMember(const FString& PlayerId)
{
    if (!IsInParty())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 当前不在 Party 中"));
        return;
    }

    if (!IsPartyLeader())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 只有队长可以踢出成员"));
        return;
    }

    if (PlayerId == GetCurrentPlayerId())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 无法踢出自己"));
        return;
    }

    CurrentPartyInfo.Members.RemoveAll([&PlayerId](const FDBAPartySubsystemMemberInfo& Member)
    {
        return Member.PlayerId == PlayerId;
    });

    OnPartyMemberLeft.Broadcast(PlayerId);

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 踢出成员: %s"), *PlayerId);
}

void UDBAPartySubsystem::TransferLeadership(const FString& PlayerId)
{
    if (!IsInParty())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 当前不在 Party 中"));
        return;
    }

    if (!IsPartyLeader())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 只有队长可以转让队长"));
        return;
    }

    FDBAPartySubsystemMemberInfo* TargetMember = CurrentPartyInfo.Members.FindByPredicate([&PlayerId](const FDBAPartySubsystemMemberInfo& Member)
    {
        return Member.PlayerId == PlayerId;
    });

    if (!TargetMember)
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 目标玩家不在 Party 中"));
        return;
    }

    FString CurrentPlayerId = GetCurrentPlayerId();
    for (FDBAPartySubsystemMemberInfo& Member : CurrentPartyInfo.Members)
    {
        if (Member.PlayerId == CurrentPlayerId)
        {
            Member.Role = EDBAPartySubsystemMemberRole::Member;
        }
        else if (Member.PlayerId == PlayerId)
        {
            Member.Role = EDBAPartySubsystemMemberRole::Leader;
        }
    }

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 转让队长给: %s"), *PlayerId);
}

void UDBAPartySubsystem::SetReady(bool bReady)
{
    if (!IsInParty())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 当前不在 Party 中"));
        return;
    }

    FString CurrentPlayerId = GetCurrentPlayerId();

    for (FDBAPartySubsystemMemberInfo& Member : CurrentPartyInfo.Members)
    {
        if (Member.PlayerId == CurrentPlayerId)
        {
            Member.bIsReady = bReady;
            break;
        }
    }

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 设置准备状态: %s"), bReady ? TEXT("准备") : TEXT("未准备"));
}

bool UDBAPartySubsystem::IsPartyLeader() const
{
    if (!IsInParty())
    {
        return false;
    }

    return CurrentPartyInfo.IsLeader(GetCurrentPlayerId());
}

bool UDBAPartySubsystem::HasPermission(bool bRequireLeader) const
{
    if (!IsInParty())
    {
        return false;
    }

    if (bRequireLeader)
    {
        return IsPartyLeader();
    }

    return true;
}

void UDBAPartySubsystem::SetPartyState(EDBAPartySubsystemState NewState)
{
    if (CurrentPartyInfo.State != NewState)
    {
        CurrentPartyInfo.State = NewState;
        OnPartyStateChanged.Broadcast(NewState);
        UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] Party 状态变更: %d"), static_cast<int32>(NewState));
    }
}

FString UDBAPartySubsystem::GetCurrentPlayerId() const
{
	// 通过 PlayerController 获取本地玩家 ID
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (APlayerState* PlayerState = PC->GetPlayerState<APlayerState>())
			{
				return FString::Printf(TEXT("%d"), PlayerState->GetPlayerId());
			}
		}
	}

	// 降级方案：通过 PlayerController 获取唯一 ID
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			return FString::Printf(TEXT("Player_%lld"), PC->GetUniqueID());
		}
	}

	// 最终降级方案
	return TEXT("LocalPlayer");
}