// Copyright FreeboozStudio. All Rights Reserved.
// 队伍子系统实现 - 管理本地玩家的组队逻辑

#include "Frontend/Party/DBAPartySubsystem.h"
#include "Frontend/Invite/DBAInviteServiceStub.h"
#include "Common/DBALogChannels.h"

// 构造函数 - 初始化队伍子系统
UDBAPartySubsystem::UDBAPartySubsystem()
{
}

// 初始化子系统 - 创建邀请服务存根
void UDBAPartySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 创建邀请服务存根用于本地测试
    InviteServiceStub = NewObject<UDBAInviteServiceStub>(this);

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 初始化完成"));
}

// 反初始化子系统 - 清理资源
void UDBAPartySubsystem::Deinitialize()
{
    Super::Deinitialize();
}

// 创建队伍 - 以当前玩家为队长创建新队伍
void UDBAPartySubsystem::CreateParty()
{
    // 检查是否已在队伍中
    if (IsInParty())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 已在 Party 中，无法创建新 Party"));
        return;
    }

    // 生成唯一队伍ID
    CurrentPartyInfo.PartyId = FGuid::NewGuid().ToString();
    CurrentPartyInfo.State = EDBAPartySubsystemState::InLobby;
    CurrentPartyInfo.MaxMembers = 5;
    CurrentPartyInfo.Members.Empty();

    // 创建队长信息
    FDBAPartySubsystemMemberInfo LeaderInfo;
    LeaderInfo.PlayerId = GetCurrentPlayerId();
    LeaderInfo.DisplayName = TEXT("本地玩家");
    LeaderInfo.Level = 1;
    LeaderInfo.Role = EDBAPartySubsystemMemberRole::Leader;
    LeaderInfo.bIsReady = false;
    LeaderInfo.bIsOnline = true;

    // 添加队长到队伍成员列表
    CurrentPartyInfo.Members.Add(LeaderInfo);

    // 更新队伍状态
    SetPartyState(EDBAPartySubsystemState::InLobby);

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 创建 Party: %s"), *CurrentPartyInfo.PartyId);
}

// 解散队伍 - 队长解散当前队伍
void UDBAPartySubsystem::DisbandParty()
{
    // 检查是否在队伍中
    if (!IsInParty())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 当前不在 Party 中"));
        return;
    }

    // 只有队长可以解散队伍
    if (!IsPartyLeader())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 只有队长可以解散 Party"));
        return;
    }

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 解散 Party: %s"), *CurrentPartyInfo.PartyId);

    // 清空队伍信息并更新状态
    CurrentPartyInfo = FDBAPartySubsystemInfo();
    SetPartyState(EDBAPartySubsystemState::None);
}

// 离开队伍 - 当前玩家离开所在队伍
void UDBAPartySubsystem::LeaveParty()
{
    // 检查是否在队伍中
    if (!IsInParty())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 当前不在 Party 中"));
        return;
    }

    FString CurrentPlayerId = GetCurrentPlayerId();

    // 如果是队长，则解散队伍（队长离开意味着队伍解散）
    if (IsPartyLeader())
    {
        DisbandParty();
    }
    else
    {
        // 从成员列表中移除当前玩家
        CurrentPartyInfo.Members.RemoveAll([&CurrentPlayerId](const FDBAPartySubsystemMemberInfo& Member)
        {
            return Member.PlayerId == CurrentPlayerId;
        });

        // 广播成员离开事件
        OnPartyMemberLeft.Broadcast(CurrentPlayerId);

        // 重置队伍信息
        CurrentPartyInfo = FDBAPartySubsystemInfo();
        SetPartyState(EDBAPartySubsystemState::None);
    }

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 离开 Party"));
}

// 邀请玩家 - 队长邀请指定玩家加入队伍
void UDBAPartySubsystem::InvitePlayer(const FString& PlayerId)
{
    // 检查是否在队伍中
    if (!IsInParty())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 当前不在 Party 中"));
        return;
    }

    // 只有队长可以邀请玩家
    if (!IsPartyLeader())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 只有队长可以邀请玩家"));
        return;
    }

    // 检查队伍是否已满
    if (CurrentPartyInfo.IsFull())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] Party 已满员"));
        return;
    }

    // 通过邀请服务发送邀请
    if (InviteServiceStub)
    {
        InviteServiceStub->SendPartyInvite(PlayerId, CurrentPartyInfo.PartyId);
    }

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 邀请玩家: %s"), *PlayerId);
}

// 接受队伍邀请 - 接受指定队伍ID的邀请
void UDBAPartySubsystem::AcceptPartyInvite(const FString& PartyId)
{
    // 检查是否已在队伍中
    if (IsInParty())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 已在 Party 中，无法接受邀请"));
        return;
    }

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 接受 Party 邀请: %s"), *PartyId);
}

// 拒绝队伍邀请 - 拒绝指定队伍ID的邀请
void UDBAPartySubsystem::DeclinePartyInvite(const FString& PartyId)
{
    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 拒绝 Party 邀请: %s"), *PartyId);
}

// 踢出成员 - 队长踢出指定玩家
void UDBAPartySubsystem::KickMember(const FString& PlayerId)
{
    // 检查是否在队伍中
    if (!IsInParty())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 当前不在 Party 中"));
        return;
    }

    // 只有队长可以踢出成员
    if (!IsPartyLeader())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 只有队长可以踢出成员"));
        return;
    }

    // 不能踢出自己
    if (PlayerId == GetCurrentPlayerId())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 无法踢出自己"));
        return;
    }

    // 从成员列表中移除指定玩家
    CurrentPartyInfo.Members.RemoveAll([&PlayerId](const FDBAPartySubsystemMemberInfo& Member)
    {
        return Member.PlayerId == PlayerId;
    });

    // 广播成员离开事件
    OnPartyMemberLeft.Broadcast(PlayerId);

    UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] 踢出成员: %s"), *PlayerId);
}

// 转让队长 - 队长将队长职位转让给指定玩家
void UDBAPartySubsystem::TransferLeadership(const FString& PlayerId)
{
    // 检查是否在队伍中
    if (!IsInParty())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 当前不在 Party 中"));
        return;
    }

    // 只有队长可以转让队长
    if (!IsPartyLeader())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 只有队长可以转让队长"));
        return;
    }

    // 查找目标成员
    FDBAPartySubsystemMemberInfo* TargetMember = CurrentPartyInfo.Members.FindByPredicate([&PlayerId](const FDBAPartySubsystemMemberInfo& Member)
    {
        return Member.PlayerId == PlayerId;
    });

    // 目标玩家不在队伍中
    if (!TargetMember)
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 目标玩家不在 Party 中"));
        return;
    }

    // 更新所有成员的职责：原队长变为普通成员，目标玩家变为队长
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

// 设置准备状态 - 当前玩家设置自己的准备状态
void UDBAPartySubsystem::SetReady(bool bReady)
{
    // 检查是否在队伍中
    if (!IsInParty())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBAPartySubsystem] 当前不在 Party 中"));
        return;
    }

    FString CurrentPlayerId = GetCurrentPlayerId();

    // 更新当前玩家的准备状态
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

// 判断是否是队长 - 检查当前玩家是否为队伍队长
bool UDBAPartySubsystem::IsPartyLeader() const
{
    if (!IsInParty())
    {
        return false;
    }

    return CurrentPartyInfo.IsLeader(GetCurrentPlayerId());
}

// 判断是否有权限 - 检查当前玩家是否有特定权限
bool UDBAPartySubsystem::HasPermission(bool bRequireLeader) const
{
    // 不在队伍中则没有权限
    if (!IsInParty())
    {
        return false;
    }

    // 如果需要队长权限，则检查是否是队长
    if (bRequireLeader)
    {
        return IsPartyLeader();
    }

    // 否则只要在队伍中就有权限
    return true;
}

// 设置队伍状态 - 内部方法，更新队伍状态并广播事件
void UDBAPartySubsystem::SetPartyState(EDBAPartySubsystemState NewState)
{
    if (CurrentPartyInfo.State != NewState)
    {
        CurrentPartyInfo.State = NewState;
        OnPartyStateChanged.Broadcast(NewState);
        UE_LOG(LogDBAUI, Log, TEXT("[DBAPartySubsystem] Party 状态变更: %d"), static_cast<int32>(NewState));
    }
}

// 获取当前玩家ID - 获取本地玩家的唯一标识符
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