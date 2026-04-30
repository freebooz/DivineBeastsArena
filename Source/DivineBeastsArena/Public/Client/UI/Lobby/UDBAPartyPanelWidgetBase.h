// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "Common/Types/DBACommonEnums.h"
#include "UDBAPartyPanelWidgetBase.generated.h"

USTRUCT(BlueprintType)
struct FDBAPartyMemberData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Party")
	FString PlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Party")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Party")
	EDBAZodiac Zodiac;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Party")
	EDBAElement Element;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Party")
	EDBAFiveCamp FiveCamp;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Party")
	int32 Level;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Party")
	bool bIsLeader;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Party")
	bool bIsReady;

	FDBAPartyMemberData()
		: Zodiac(EDBAZodiac::None)
		, Element(EDBAElement::None)
		, FiveCamp(EDBAFiveCamp::None)
		, Level(1)
		, bIsLeader(false)
		, bIsReady(false)
	{
	}
};

/**
 * DBAPartyPanelWidgetBase
 *
 * 队伍面板 Widget 基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAPartyPanelWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAPartyPanelWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|PartyPanel")
	void RefreshPartyMembers();

	UFUNCTION(BlueprintCallable, Category = "DBA|PartyPanel")
	void InviteFriend();

	UFUNCTION(BlueprintCallable, Category = "DBA|PartyPanel")
	void KickMember(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "DBA|PartyPanel")
	void LeaveParty();

	UFUNCTION(BlueprintCallable, Category = "DBA|PartyPanel")
	void ToggleReady();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|PartyPanel", meta = (DisplayName = "On Party Members Refreshed"))
	void BP_OnPartyMembersRefreshed(const TArray<FDBAPartyMemberData>& Members);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|PartyPanel", meta = (DisplayName = "On Member Ready Changed"))
	void BP_OnMemberReadyChanged(const FString& PlayerId, bool bIsReady);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|PartyPanel")
	TArray<FDBAPartyMemberData> PartyMembers;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|PartyPanel")
	bool bIsLeader;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|PartyPanel")
	bool bIsLocalPlayerReady;
};
