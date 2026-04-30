// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAWidgetController.h"
#include "UUDBAQueueWidgetController.generated.h"

/**
 * 队列状态委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueueStateChangedDelegate, int32, NewState);

/**
 * 匹配成功委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchFoundDelegate, const FString&, MatchId);

/**
 * 队列取消委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQueueCancelledDelegate, const FString&, Reason);

/**
 * 队列 Widget 控制器
 *
 * 管理匹配队列 UI 的数据绑定和交互
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAQueueWidgetController : public UDBAWidgetController
{
	GENERATED_BODY()

public:
	UDBAQueueWidgetController(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|Queue")
	void RequestJoinQueue(int32 Mode);

	UFUNCTION(BlueprintCallable, Category = "DBA|Queue")
	void RequestLeaveQueue();

	UFUNCTION(BlueprintCallable, Category = "DBA|Queue")
	void RequestAcceptMatch();

	UFUNCTION(BlueprintCallable, Category = "DBA|Queue")
	void RequestDeclineMatch();

public:
	UPROPERTY(BlueprintAssignable, Category = "DBA|Queue")
	FOnQueueStateChangedDelegate OnQueueStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "DBA|Queue")
	FOnMatchFoundDelegate OnMatchFound;

	UPROPERTY(BlueprintAssignable, Category = "DBA|Queue")
	FOnQueueCancelledDelegate OnQueueCancelled;
};
