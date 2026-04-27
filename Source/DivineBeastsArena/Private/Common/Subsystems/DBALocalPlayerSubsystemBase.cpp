// Copyright FreeboozStudio. All Rights Reserved.

#include "Common/Subsystems/DBALocalPlayerSubsystemBase.h"
#include "Common/DBALogChannels.h"

void UDBALocalPlayerSubsystemBase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bIsInitialized = false;
	OnSubsystemInitialize();
	bIsInitialized = true;
	UE_LOG(LogDBACore, Log, TEXT("[%s] Initialized"), *GetSubsystemDisplayName());
}

void UDBALocalPlayerSubsystemBase::Deinitialize()
{
	UE_LOG(LogDBACore, Log, TEXT("[%s] Deinitializing"), *GetSubsystemDisplayName());
	CancelAllAsyncOperations();
	OnSubsystemDeinitialize();
	Super::Deinitialize();
}

bool UDBALocalPlayerSubsystemBase::ShouldCreateSubsystem(UObject* Outer) const
{
	return IsSupportedInCurrentEnvironment();
}

bool UDBALocalPlayerSubsystemBase::IsSupportedInCurrentEnvironment() const
{
	return true;
}
