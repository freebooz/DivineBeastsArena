// Copyright FreeboozStudio. All Rights Reserved.

#include "Common/Subsystems/DBAWorldSubsystemBase.h"
#include "Common/DBALogChannels.h"

void UDBAWorldSubsystemBase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bIsInitialized = false;
	OnSubsystemInitialize();
	bIsInitialized = true;
	UE_LOG(LogDBACore, Log, TEXT("[%s] Initialized"), *GetSubsystemDisplayName());
}

void UDBAWorldSubsystemBase::Deinitialize()
{
	UE_LOG(LogDBACore, Log, TEXT("[%s] Deinitializing"), *GetSubsystemDisplayName());
	CancelAllAsyncOperations();
	OnSubsystemDeinitialize();
	Super::Deinitialize();
}

bool UDBAWorldSubsystemBase::ShouldCreateSubsystem(UObject* Outer) const
{
	return IsSupportedInCurrentEnvironment();
}

void UDBAWorldSubsystemBase::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	OnWorldBeginPlayInternal();
}

bool UDBAWorldSubsystemBase::IsSupportedInCurrentEnvironment() const
{
	return true;
}
