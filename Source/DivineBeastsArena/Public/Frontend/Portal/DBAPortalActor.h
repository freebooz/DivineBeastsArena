// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Frontend/Interaction/DBAInteractableInterface.h"
#include "DBAPortalActor.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API ADBAPortalActor : public AActor, public IDBAInteractionInterface
{
    GENERATED_BODY()

public:
    ADBAPortalActor();

    UFUNCTION(BlueprintCallable, Category = "DBA|Portal")
    void SetTargetLevel(const FString& NewTargetLevel);

    UFUNCTION(BlueprintPure, Category = "DBA|Portal")
    FString GetTargetLevel() const { return TargetLevel; }

public:
    //~ Begin IDBAInteractionInterface
    virtual FText GetInteractionPromptText_Implementation() const override;
    virtual bool CanInteract_Implementation(AActor* Interactor) const override;
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual float GetInteractionRange_Implementation() const override;
    //~ End IDBAInteractionInterface

protected:
    /** 目标关卡路径 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
    FString TargetLevel;

    /** 交互提示文本 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
    FText PromptText;

    /** 交互范围 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portal")
    float InteractionRange = 200.0f;

    /** Portal 激活时的颜色 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal")
    FColor PortalColor;
};