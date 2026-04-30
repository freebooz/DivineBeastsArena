// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "DBAInvitePanelWidgetBase.generated.h"

USTRUCT(BlueprintType)
struct FDBAFriendData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Friend")
	FString FriendId;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Friend")
	FString FriendName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Friend")
	bool bIsOnline;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Friend")
	FText CurrentStatus;

	FDBAFriendData()
		: bIsOnline(false)
	{
	}
};

/**
 * DBAInvitePanelWidgetBase
 *
 * 邀请面板 Widget 基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAInvitePanelWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAInvitePanelWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|InvitePanel")
	void RefreshFriendList();

	UFUNCTION(BlueprintCallable, Category = "DBA|InvitePanel")
	void InviteFriend(const FString& FriendId);

	UFUNCTION(BlueprintCallable, Category = "DBA|InvitePanel")
	void ClosePanel();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|InvitePanel", meta = (DisplayName = "On Friend List Refreshed"))
	void BP_OnFriendListRefreshed(const TArray<FDBAFriendData>& Friends);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|InvitePanel", meta = (DisplayName = "On Invite Sent"))
	void BP_OnInviteSent(const FString& FriendId);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|InvitePanel")
	TArray<FDBAFriendData> FriendList;
};
