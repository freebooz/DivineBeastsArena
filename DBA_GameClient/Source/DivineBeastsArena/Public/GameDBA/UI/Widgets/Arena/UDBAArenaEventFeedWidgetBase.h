// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明 Arena HUD 事件流 Widget 基类，供 UMG 蓝图承接战斗事件条目。
- 阅读重点：AddEventEntry / ClearEventFeed 是 C++ 到蓝图的最小桥接入口。
- 修改提示：保持该类仅负责 UI 事件转发，不在此处耦合 GAS、角色或后端状态。
*/

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "UDBAArenaEventFeedWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAArenaEventFeedWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAArenaEventFeedWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaEventFeed")
	void AddEventEntry(const FText& Text, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaEventFeed")
	void ClearEventFeed();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|ArenaEventFeed", meta = (DisplayName = "On Event Entry Added"))
	void BP_OnEventEntryAdded(const FText& Text, float Duration);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|ArenaEventFeed", meta = (DisplayName = "On Event Feed Cleared"))
	void BP_OnEventFeedCleared();
};
