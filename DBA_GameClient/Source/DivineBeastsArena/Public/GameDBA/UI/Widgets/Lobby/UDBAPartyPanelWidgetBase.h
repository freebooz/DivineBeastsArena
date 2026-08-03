// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "GameCore/Types/DBACommonEnums.h"
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
class DIVINEBEASTSARENA_API UDBAPartyPanelWidgetBase : public UDBAMobaUserWidgetBase
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
