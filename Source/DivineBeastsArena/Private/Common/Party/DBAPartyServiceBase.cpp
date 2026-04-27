// Copyright FreeboozStudio. All Rights Reserved.

#include "Common/Party/DBAPartyServiceBase.h"
#include "Common/Account/DBAAccountServiceBase.h"
#include "Misc/Guid.h"

UDBAPartyServiceBase::UDBAPartyServiceBase()
{
}

void UDBAPartyServiceBase::OnSubsystemInitialize()
{
	Super::OnSubsystemInitialize();

	LogSubsystemInfo(TEXT("Party 服务初始化"));

	bIsInitialized = true;
}

void UDBAPartyServiceBase::OnSubsystemDeinitialize()
{
	LogSubsystemInfo(TEXT("Party 服务反初始化"));

	Super::OnSubsystemDeinitialize();
}

bool UDBAPartyServiceBase::IsSupportedInCurrentEnvironment() const
{
	// Dedicated Server 不需要 Party 服务
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

	LogSubsystemError(TEXT("CreateParty - 基类未实现，派生类必须重写此方法"));

	FDBAPartyInfo EmptyParty;
	OnComplete.ExecuteIfBound(EmptyParty);
}

void UDBAPartyServiceBase::InvitePlayer(const FDBAAccountId& AccountId, FDBAOnPartyOperationComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("InvitePlayer")))
	{
		return;
	}

	LogSubsystemError(TEXT("InvitePlayer - 基类未实现，派生类必须重写此方法"));

	OnComplete.ExecuteIfBound(false, TEXT("未实现"));
}

void UDBAPartyServiceBase::AcceptInvite(const FDBAPartyInvite& Invite, FDBAOnPartyJoined OnComplete)
{
	if (!EnsureGameThread(TEXT("AcceptInvite")))
	{
		return;
	}

	LogSubsystemError(TEXT("AcceptInvite - 基类未实现，派生类必须重写此方法"));

	FDBAPartyInfo EmptyParty;
	OnComplete.ExecuteIfBound(EmptyParty);
}

void UDBAPartyServiceBase::DeclineInvite(const FDBAPartyInvite& Invite, FDBAOnPartyOperationComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("DeclineInvite")))
	{
		return;
	}

	LogSubsystemError(TEXT("DeclineInvite - 基类未实现，派生类必须重写此方法"));

	OnComplete.ExecuteIfBound(false, TEXT("未实现"));
}

void UDBAPartyServiceBase::LeaveParty(FDBAOnPartyLeft OnComplete)
{
	if (!EnsureGameThread(TEXT("LeaveParty")))
	{
		return;
	}

	CurrentPartyInfo = FDBAPartyInfo();

	LogSubsystemInfo(TEXT("离开 Party 成功"));

	OnComplete.ExecuteIfBound();
}

void UDBAPartyServiceBase::KickMember(const FDBAAccountId& AccountId, FDBAOnPartyOperationComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("KickMember")))
	{
		return;
	}

	LogSubsystemError(TEXT("KickMember - 基类未实现，派生类必须重写此方法"));

	OnComplete.ExecuteIfBound(false, TEXT("未实现"));
}

void UDBAPartyServiceBase::PromoteLeader(const FDBAAccountId& AccountId, FDBAOnPartyOperationComplete OnComplete)
{
	if (!EnsureGameThread(TEXT("PromoteLeader")))
	{
		return;
	}

	LogSubsystemError(TEXT("PromoteLeader - 基类未实现，派生类必须重写此方法"));

	OnComplete.ExecuteIfBound(false, TEXT("未实现"));
}

bool UDBAPartyServiceBase::IsLeader() const
{
	if (!IsInParty())
	{
		return false;
	}

	// 获取当前账户服务
	UDBAAccountServiceBase* AccountService = GetGameInstance()->GetSubsystem<UDBAAccountServiceBase>();
	if (!AccountService)
	{
		return false;
	}

	const FDBAAccountInfo& AccountInfo = AccountService->GetCurrentAccountInfo();
	return CurrentPartyInfo.LeaderAccountId == AccountInfo.AccountId;
}

FDBAPartyId UDBAPartyServiceBase::GeneratePartyId()
{
	const FString PartyIdString = FGuid::NewGuid().ToString();
	return FDBAPartyId(PartyIdString);
}
