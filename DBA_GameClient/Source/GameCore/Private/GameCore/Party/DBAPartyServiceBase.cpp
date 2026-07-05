// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameCore/Party/DBAPartyServiceBase.h"

#include "GameCore/Account/DBAOnlineAccountService.h"
#include "GameCore/Session/DBAFrontendSessionSubsystem.h"
#include "Misc/DateTime.h"
#include "Misc/Guid.h"

UDBAPartyServiceBase::UDBAPartyServiceBase()
{
}

void UDBAPartyServiceBase::OnSubsystemInitialize()
{
	Super::OnSubsystemInitialize();
	LogSubsystemInfo(TEXT("组队服务已初始化。"));
	bIsInitialized = true;
}

void UDBAPartyServiceBase::OnSubsystemDeinitialize()
{
	LogSubsystemInfo(TEXT("组队服务已反初始化。"));
	Super::OnSubsystemDeinitialize();
}

bool UDBAPartyServiceBase::IsSupportedInCurrentEnvironment() const
{
	if (GetGameInstance() && GetGameInstance()->IsDedicatedServerInstance())
	{
		return false;
	}

	return true;
}

void UDBAPartyServiceBase::CreateParty(FDBAOnPartyCreated OnComplete)
{
	if (!EnsureGameThread(TEXT("CreateParty")))
	{
		return;
	}

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService || !AccountService->IsLoggedIn())
	{
		LogSubsystemError(TEXT("创建队伍失败：账号未登录。"));
		FDBAPartyInfo EmptyParty;
		OnComplete.ExecuteIfBound(EmptyParty);
		return;
	}

	const FDBAAccountInfo& AccountInfo = AccountService->GetCurrentAccountInfo();
	FDBAPartyMember LeaderMember;
	LeaderMember.AccountId = AccountInfo.AccountId;
	LeaderMember.DisplayName = AccountInfo.DisplayName;
	LeaderMember.Role = EDBAPartyMemberRole::Leader;
	LeaderMember.Status = EDBAPartyMemberStatus::Online;
	LeaderMember.JoinTime = FDateTime::UtcNow().ToUnixTimestamp();

	CurrentPartyInfo = FDBAPartyInfo();
	CurrentPartyInfo.PartyId = GeneratePartyId();
	CurrentPartyInfo.LeaderAccountId = LeaderMember.AccountId;
	CurrentPartyInfo.Members.Add(LeaderMember);
	CurrentPartyInfo.State = EDBAPartyState::Idle;
	CurrentPartyInfo.MaxMembers = 5;
	CurrentPartyInfo.CreateTime = FDateTime::UtcNow().ToUnixTimestamp();

	if (UDBAFrontendSessionSubsystem* FrontendSession = GetGameInstance()->GetSubsystem<UDBAFrontendSessionSubsystem>())
	{
		FrontendSession->SetCurrentPartyInfo(CurrentPartyInfo);
		FrontendSession->SetState(EDBAFrontendSessionState::InParty);
	}

	OnPartyUpdated.ExecuteIfBound(CurrentPartyInfo);
	OnComplete.ExecuteIfBound(CurrentPartyInfo);
}

void UDBAPartyServiceBase::InvitePlayer(const FDBAAccountId& AccountId, FDBAOnPartyOperationComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("InvitePlayer")))
	{
		return;
	}

	if (!IsInParty())
	{
		OnComplete.ExecuteIfBound(false, TEXT("队伍尚未创建。"));
		return;
	}

	if (CurrentPartyInfo.IsFull())
	{
		OnComplete.ExecuteIfBound(false, TEXT("队伍已满。"));
		return;
	}

	FDBAPartyMember NewMember;
	NewMember.AccountId = AccountId;
	NewMember.DisplayName = AccountId.ToString();
	NewMember.Role = EDBAPartyMemberRole::Member;
	NewMember.Status = EDBAPartyMemberStatus::Online;
	NewMember.JoinTime = FDateTime::UtcNow().ToUnixTimestamp();

	CurrentPartyInfo.Members.Add(NewMember);
	OnPartyUpdated.ExecuteIfBound(CurrentPartyInfo);
	OnComplete.ExecuteIfBound(true, TEXT(""));
}

void UDBAPartyServiceBase::AcceptInvite(const FDBAPartyInvite& Invite, FDBAOnPartyJoined OnComplete)
{
	if (!EnsureGameThread(TEXT("AcceptInvite")))
	{
		return;
	}

	if (!Invite.IsValid())
	{
		FDBAPartyInfo EmptyParty;
		OnComplete.ExecuteIfBound(EmptyParty);
		return;
	}

	CurrentPartyInfo = FDBAPartyInfo();
	CurrentPartyInfo.PartyId = Invite.PartyId;
	CurrentPartyInfo.LeaderAccountId = Invite.InviterAccountId;
	CurrentPartyInfo.State = EDBAPartyState::Idle;
	CurrentPartyInfo.MaxMembers = 5;
	CurrentPartyInfo.CreateTime = FDateTime::UtcNow().ToUnixTimestamp();

	FDBAPartyMember LeaderMember;
	LeaderMember.AccountId = Invite.InviterAccountId;
	LeaderMember.DisplayName = Invite.InviterDisplayName;
	LeaderMember.Role = EDBAPartyMemberRole::Leader;
	LeaderMember.Status = EDBAPartyMemberStatus::Online;
	LeaderMember.JoinTime = FDateTime::UtcNow().ToUnixTimestamp();
	CurrentPartyInfo.Members.Add(LeaderMember);

	if (UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr)
	{
		const FDBAAccountInfo& AccountInfo = AccountService->GetCurrentAccountInfo();
		if (AccountInfo.AccountId != Invite.InviterAccountId && AccountInfo.AccountId.IsValid())
		{
			FDBAPartyMember SelfMember;
			SelfMember.AccountId = AccountInfo.AccountId;
			SelfMember.DisplayName = AccountInfo.DisplayName;
			SelfMember.Role = EDBAPartyMemberRole::Member;
			SelfMember.Status = EDBAPartyMemberStatus::Online;
			SelfMember.JoinTime = FDateTime::UtcNow().ToUnixTimestamp();
			CurrentPartyInfo.Members.Add(SelfMember);
		}
	}

	if (UDBAFrontendSessionSubsystem* FrontendSession = GetGameInstance()->GetSubsystem<UDBAFrontendSessionSubsystem>())
	{
		FrontendSession->SetCurrentPartyInfo(CurrentPartyInfo);
		FrontendSession->SetState(EDBAFrontendSessionState::InParty);
	}

	OnPartyUpdated.ExecuteIfBound(CurrentPartyInfo);
	OnComplete.ExecuteIfBound(CurrentPartyInfo);
}

void UDBAPartyServiceBase::DeclineInvite(const FDBAPartyInvite& Invite, FDBAOnPartyOperationComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("DeclineInvite")))
	{
		return;
	}

	if (!Invite.IsValid())
	{
		OnComplete.ExecuteIfBound(false, TEXT("邀请无效。"));
		return;
	}

	OnComplete.ExecuteIfBound(true, TEXT(""));
}

void UDBAPartyServiceBase::LeaveParty(FDBAOnPartyLeft OnComplete)
{
	if (!EnsureGameThread(TEXT("LeaveParty")))
	{
		return;
	}

	CurrentPartyInfo = FDBAPartyInfo();
	if (UDBAFrontendSessionSubsystem* FrontendSession = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendSessionSubsystem>() : nullptr)
	{
		FrontendSession->ClearCurrentPartyInfo();
		FrontendSession->SetState(EDBAFrontendSessionState::MainLobby);
	}

	LogSubsystemInfo(TEXT("离开队伍成功。"));
	OnComplete.ExecuteIfBound();
}

void UDBAPartyServiceBase::KickMember(const FDBAAccountId& AccountId, FDBAOnPartyOperationComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("KickMember")))
	{
		return;
	}

	const int32 RemovedCount = CurrentPartyInfo.Members.RemoveAll([&AccountId](const FDBAPartyMember& Member)
	{
		return Member.AccountId == AccountId && Member.Role != EDBAPartyMemberRole::Leader;
	});

	if (RemovedCount > 0)
	{
		OnPartyUpdated.ExecuteIfBound(CurrentPartyInfo);
		OnComplete.ExecuteIfBound(true, TEXT(""));
		return;
	}

	OnComplete.ExecuteIfBound(false, TEXT("未找到成员或目标是队长。"));
}

void UDBAPartyServiceBase::PromoteLeader(const FDBAAccountId& AccountId, FDBAOnPartyOperationComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("PromoteLeader")))
	{
		return;
	}

	FDBAPartyMember* NewLeader = CurrentPartyInfo.FindMember(AccountId);
	if (!NewLeader)
	{
		OnComplete.ExecuteIfBound(false, TEXT("未找到成员。"));
		return;
	}

	for (FDBAPartyMember& Member : CurrentPartyInfo.Members)
	{
		Member.Role = (Member.AccountId == AccountId) ? EDBAPartyMemberRole::Leader : EDBAPartyMemberRole::Member;
	}
	CurrentPartyInfo.LeaderAccountId = AccountId;

	OnPartyUpdated.ExecuteIfBound(CurrentPartyInfo);
	OnComplete.ExecuteIfBound(true, TEXT(""));
}

bool UDBAPartyServiceBase::IsLeader() const
{
	if (!IsInParty())
	{
		return false;
	}

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		return false;
	}

	const FDBAAccountInfo& AccountInfo = AccountService->GetCurrentAccountInfo();
	return CurrentPartyInfo.LeaderAccountId == AccountInfo.AccountId;
}

FDBAPartyId UDBAPartyServiceBase::GeneratePartyId()
{
	return FDBAPartyId(FGuid::NewGuid().ToString());
}
