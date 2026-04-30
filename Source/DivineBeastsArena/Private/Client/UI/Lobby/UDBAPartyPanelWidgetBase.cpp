// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "Client/UI/Lobby/UDBAPartyPanelWidgetBase.h"

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
	Member1.PlayerName = TEXT("张三");
	Member1.Zodiac = EDBAZodiac::Rat;
	Member1.Element = EDBAElement::Fire;
	Member1.FiveCamp = EDBAFiveCamp::Center;
	Member1.Level = 10;
	Member1.bIsLeader = true;
	Member1.bIsReady = true;
	PartyMembers.Add(Member1);

	FDBAPartyMemberData Member2;
	Member2.PlayerId = TEXT("player_002");
	Member2.PlayerName = TEXT("李四");
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
}

void UDBAPartyPanelWidgetBase::ToggleReady()
{
	bIsLocalPlayerReady = !bIsLocalPlayerReady;
	BP_OnMemberReadyChanged(TEXT("local_player"), bIsLocalPlayerReady);
}
