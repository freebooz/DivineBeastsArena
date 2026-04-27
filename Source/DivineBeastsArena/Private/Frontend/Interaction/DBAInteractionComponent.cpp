// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/Interaction/DBAInteractionComponent.h"
#include "Frontend/Interaction/DBAInteractableInterface.h"
#include "Common/DBALogChannels.h"

UDBAInteractionComponent::UDBAInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UDBAInteractionComponent::FindInteractableTarget(TScriptInterface<IDBAInteractionInterface>& OutTarget)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }

    FindInteractablesInRange();

    if (CurrentTarget)
    {
        OutTarget.SetInterface(Cast<IDBAInteractionInterface>(CurrentTarget));
        OutTarget.SetObject(CurrentTarget);
        return true;
    }

    return false;
}

void UDBAInteractionComponent::InteractWith(AActor* Target)
{
    if (!Target)
    {
        return;
    }

    IDBAInteractionInterface* Interactable = Cast<IDBAInteractionInterface>(Target);
    if (Interactable && Interactable->CanInteract(GetOwner()))
    {
        Interactable->Interact(GetOwner());
        UE_LOG(LogDBAUI, Log, TEXT("[UDBAInteractionComponent] 交互: %s"), *Target->GetName());
    }
}

void UDBAInteractionComponent::FindInteractablesInRange()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    TArray<FHitResult> HitResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Owner);

    FVector OwnerLocation = Owner->GetActorLocation();

    GetWorld()->SweepMultiByChannel(
        HitResults,
        OwnerLocation,
        OwnerLocation,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(InteractionRange),
        QueryParams
    );

    CurrentTarget = nullptr;
    float ClosestDistance = InteractionRange;

    for (const FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();
        if (HitActor && IsActorInteractable(HitActor))
        {
            float Distance = Hit.Distance;
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                CurrentTarget = HitActor;
            }
        }
    }
}

bool UDBAInteractionComponent::IsActorInteractable(AActor* Actor) const
{
    if (!Actor)
    {
        return false;
    }

    return Actor->GetClass()->ImplementsInterface(UDBAInteractionInterface::StaticClass());
}