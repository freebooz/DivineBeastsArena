// Copyright FreeboozStudio. All Rights Reserved.

#include "GameCore/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "GameCore/Core/DBALogChannels.h"

void UDBAGameInstanceSubsystemBase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bIsInitialized = false;
	OnSubsystemInitialize();
	bIsInitialized = true;
	UE_LOG(LogDBACore, Log, TEXT("[%s] 已初始化"), *GetSubsystemDisplayName());
}

void UDBAGameInstanceSubsystemBase::Deinitialize()
{
	UE_LOG(LogDBACore, Log, TEXT("[%s] 正在反初始化"), *GetSubsystemDisplayName());
	CancelAllAsyncOperations();
	OnSubsystemDeinitialize();
	Super::Deinitialize();
}

bool UDBAGameInstanceSubsystemBase::ShouldCreateSubsystem(UObject* Outer) const
{
	return IsSupportedInCurrentEnvironment();
}

bool UDBAGameInstanceSubsystemBase::IsSupportedInCurrentEnvironment() const
{
	return true;
}
