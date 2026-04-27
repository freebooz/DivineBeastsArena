// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/Invite/DBAInviteServiceStub.h"
#include "Common/DBALogChannels.h"

void UDBAInviteServiceStub::SendPartyInvite(const FString& TargetPlayerId, const FString& PartyId)
{
    FDBAInviteInfo InviteInfo;
    InviteInfo.InviteId = FGuid::NewGuid().ToString();
    InviteInfo.InviteType = EDBAInviteType::Party;
    InviteInfo.InviterId = TEXT("LocalPlayer");
    InviteInfo.InviterName = TEXT("本地玩家");
    InviteInfo.TargetId = PartyId;
    InviteInfo.InviteTime = FDateTime::Now();
    InviteInfo.bExpired = false;

    PendingInvites.Add(InviteInfo);

    UE_LOG(LogDBAUI, Log, TEXT("[DBAInviteServiceStub] 发送 Party 邀请: %s -> %s"), *TargetPlayerId, *PartyId);
}

void UDBAInviteServiceStub::SendFriendInvite(const FString& TargetPlayerId)
{
    FDBAInviteInfo InviteInfo;
    InviteInfo.InviteId = FGuid::NewGuid().ToString();
    InviteInfo.InviteType = EDBAInviteType::Friend;
    InviteInfo.InviterId = TEXT("LocalPlayer");
    InviteInfo.InviterName = TEXT("本地玩家");
    InviteInfo.TargetId = TargetPlayerId;
    InviteInfo.InviteTime = FDateTime::Now();
    InviteInfo.bExpired = false;

    PendingInvites.Add(InviteInfo);

    UE_LOG(LogDBAUI, Log, TEXT("[DBAInviteServiceStub] 发送好友邀请: %s -> %s"), *TargetPlayerId, *InviteInfo.InviteId);
}

void UDBAInviteServiceStub::ClearExpiredInvites()
{
    FDateTime Now = FDateTime::Now();
    PendingInvites.RemoveAll([&Now](const FDBAInviteInfo& Invite)
    {
        if (Invite.bExpired)
        {
            return true;
        }

        FTimespan Elapsed = Now - Invite.InviteTime;
        return Elapsed.GetTotalSeconds() > InviteTimeoutSeconds;
    });
}