// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/Bridge/DBAFrontendWorldBridgeComponent.h"
#include "Engine/World.h"
#include "Common/DBALogChannels.h"

UDBAFrontendWorldBridgeComponent::UDBAFrontendWorldBridgeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UDBAFrontendWorldBridgeComponent::RegisterToFrontendSubsystem()
{
    UE_LOG(LogDBAUI, Log, TEXT("[UDBAFrontendWorldBridgeComponent] 注册到 Frontend 子系统: %s"), *GetName());
}

void UDBAFrontendWorldBridgeComponent::UnregisterFromFrontendSubsystem()
{
    UE_LOG(LogDBAUI, Log, TEXT("[UDBAFrontendWorldBridgeComponent] 从 Frontend 子系统注销: %s"), *GetName());
}