// Copyright FreeboozStudio. All Rights Reserved.

#include "Common/Queue/DBAQueueServiceBase.h"
#include "Misc/Guid.h"

UDBAQueueServiceBase::UDBAQueueServiceBase()
{
}

void UDBAQueueServiceBase::OnSubsystemInitialize()
{
	Super::OnSubsystemInitialize();

	LogSubsystemInfo(TEXT("Queue 服务初始化"));

	bIsInitialized = true;
}

void UDBAQueueServiceBase::OnSubsystemDeinitialize()
{
	LogSubsystemInfo(TEXT("Queue 服务反初始化"));

	Super::OnSubsystemDeinitialize();
}

bool UDBAQueueServiceBase::IsSupportedInCurrentEnvironment() const
{
	// Dedicated Server 不需要 Queue 服务
	if (GetGameInstance() && GetGameInstance()->IsDedicatedServerInstance())
	{
		return false;
	}

	return true;
}

void UDBAQueueServiceBase::StartQueue(EDBAQueueType QueueType, FDBAOnQueueStarted OnComplete)
{
	if (!EnsureGameThread(TEXT("StartQueue")))
	{
		return;
	}

	LogSubsystemError(TEXT("StartQueue - 基类未实现，派生类必须重写此方法"));

	FDBAQueueInfo EmptyQueue;
	OnComplete.ExecuteIfBound(EmptyQueue);
}

void UDBAQueueServiceBase::CancelQueue(FDBAOnQueueCancelled OnComplete)
{
	if (!EnsureGameThread(TEXT("CancelQueue")))
	{
		return;
	}

	CurrentQueueInfo = FDBAQueueInfo();

	LogSubsystemInfo(TEXT("取消匹配成功"));

	OnComplete.ExecuteIfBound();
}

void UDBAQueueServiceBase::ConfirmReady(const FDBAReadyCheckId& ReadyCheckId, FDBAOnReadyCheckCompleted OnComplete)
{
	if (!EnsureGameThread(TEXT("ConfirmReady")))
	{
		return;
	}

	LogSubsystemError(TEXT("ConfirmReady - 基类未实现，派生类必须重写此方法"));

	OnComplete.ExecuteIfBound(false);
}

void UDBAQueueServiceBase::DeclineReady(const FDBAReadyCheckId& ReadyCheckId, FDBAOnReadyCheckCompleted OnComplete)
{
	if (!EnsureGameThread(TEXT("DeclineReady")))
	{
		return;
	}

	LogSubsystemError(TEXT("DeclineReady - 基类未实现，派生类必须重写此方法"));

	OnComplete.ExecuteIfBound(false);
}

FDBAQueueId UDBAQueueServiceBase::GenerateQueueId()
{
	const FString QueueIdString = FGuid::NewGuid().ToString();
	return FDBAQueueId(QueueIdString);
}

FDBAReadyCheckId UDBAQueueServiceBase::GenerateReadyCheckId()
{
	const FString ReadyCheckIdString = FGuid::NewGuid().ToString();
	return FDBAReadyCheckId(ReadyCheckIdString);
}

FDBAMatchSessionId UDBAQueueServiceBase::GenerateMatchSessionId()
{
	const FString SessionIdString = FGuid::NewGuid().ToString();
	return FDBAMatchSessionId(SessionIdString);
}
