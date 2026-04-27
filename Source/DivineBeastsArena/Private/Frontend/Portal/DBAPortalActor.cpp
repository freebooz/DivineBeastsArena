// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/Portal/DBAPortalActor.h"
#include "Components/StaticMeshComponent.h"
#include "Common/DBALogChannels.h"

ADBAPortalActor::ADBAPortalActor()
{
    PrimaryActorTick.bCanEverTick = true;

    PortalColor = FColor::Cyan;
}

void ADBAPortalActor::SetTargetLevel(const FString& NewTargetLevel)
{
    TargetLevel = NewTargetLevel;
}

FText ADBAPortalActor::GetInteractionPromptText_Implementation() const
{
    return PromptText.IsEmpty() ? FText::FromString(TEXT("传送")) : PromptText;
}

bool ADBAPortalActor::CanInteract_Implementation(AActor* Interactor) const
{
    return Interactor != nullptr && !TargetLevel.IsEmpty();
}

void ADBAPortalActor::Interact_Implementation(AActor* Interactor)
{
    if (!TargetLevel.IsEmpty())
    {
        UE_LOG(LogDBAUI, Log, TEXT("[ADBAPortalActor] 传送至: %s"), *TargetLevel);
    }
}

float ADBAPortalActor::GetInteractionRange_Implementation() const
{
    return InteractionRange;
}