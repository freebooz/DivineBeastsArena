// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "Client/UI/Lobby/DBAInvitePanelWidgetBase.h"

UDBAInvitePanelWidgetBase::UDBAInvitePanelWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAInvitePanelWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBAInvitePanelWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshFriendList();
}

void UDBAInvitePanelWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBAInvitePanelWidgetBase::RefreshFriendList()
{
	FriendList.Empty();
	BP_OnFriendListRefreshed(FriendList);
}

void UDBAInvitePanelWidgetBase::InviteFriend(const FString& FriendId)
{
	BP_OnInviteSent(FriendId);
}

void UDBAInvitePanelWidgetBase::ClosePanel()
{
	RemoveFromParent();
}
