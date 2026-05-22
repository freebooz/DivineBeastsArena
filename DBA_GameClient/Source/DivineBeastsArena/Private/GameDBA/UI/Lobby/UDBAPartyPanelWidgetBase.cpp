// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Lobby/UDBAPartyPanelWidgetBase.h"

#include "GameDBA/UI/DBAGameUIManager.h"

UDBAPartyPanelWidgetBase::UDBAPartyPanelWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsLeader(false)
	, bIsLocalPlayerReady(false)
{
}

void UDBAPartyPanelWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBAPartyPanelWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshPartyMembers();
}

void UDBAPartyPanelWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBAPartyPanelWidgetBase::RefreshPartyMembers()
{
	PartyMembers.Empty();

	FDBAPartyMemberData Member1;
	Member1.PlayerId = TEXT("player_001");
	Member1.PlayerName = TEXT("寮犱笁");
	Member1.Zodiac = EDBAZodiac::Rat;
	Member1.Element = EDBAElement::Fire;
	Member1.FiveCamp = EDBAFiveCamp::Center;
	Member1.Level = 10;
	Member1.bIsLeader = true;
	Member1.bIsReady = true;
	PartyMembers.Add(Member1);

	FDBAPartyMemberData Member2;
	Member2.PlayerId = TEXT("player_002");
	Member2.PlayerName = TEXT("鏉庡洓");
	Member2.Zodiac = EDBAZodiac::Dragon;
	Member2.Element = EDBAElement::Water;
	Member2.FiveCamp = EDBAFiveCamp::Center;
	Member2.Level = 5;
	Member2.bIsLeader = false;
	Member2.bIsReady = false;
	PartyMembers.Add(Member2);

	bIsLeader = Member1.bIsLeader;

	BP_OnPartyMembersRefreshed(PartyMembers);
}

void UDBAPartyPanelWidgetBase::InviteFriend()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
		{
			UIManager->ShowInvitePanel();
		}
	}
}

void UDBAPartyPanelWidgetBase::KickMember(const FString& PlayerId)
{
	if (!bIsLeader)
	{
		return;
	}
}

void UDBAPartyPanelWidgetBase::LeaveParty()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
		{
			UIManager->HidePartyPanel();
		}
	}
}

void UDBAPartyPanelWidgetBase::ToggleReady()
{
	bIsLocalPlayerReady = !bIsLocalPlayerReady;
	BP_OnMemberReadyChanged(TEXT("local_player"), bIsLocalPlayerReady);
}

