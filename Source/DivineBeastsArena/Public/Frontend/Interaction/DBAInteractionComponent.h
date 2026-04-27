// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UObject/ScriptInterface.h"
#include "DBAInteractionComponent.generated.h"

class IDBAInteractionInterface;

UCLASS(ClassGroup=(DBA), meta=(BlueprintSpawnableComponent))
class DIVINEBEASTSARENA_API UDBAInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDBAInteractionComponent();

    /**
     * 查找可交互目标
     * @param OutTarget 输出可交互目标
     * @return 是否找到
     */
    UFUNCTION(BlueprintCallable, Category = "DBA|Interaction")
    bool FindInteractableTarget(TScriptInterface<IDBAInteractionInterface>& OutTarget);

    /**
     * 执行交互
     * @param Target 交互目标
     */
    UFUNCTION(BlueprintCallable, Category = "DBA|Interaction")
    void InteractWith(AActor* Target);

    /**
     * 获取当前可交互目标
     */
    UFUNCTION(BlueprintPure, Category = "DBA|Interaction")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

protected:
    /**
     * 查找范围内的可交互对象
     */
    void FindInteractablesInRange();

    /**
     * 检查对象是否可交互
     */
    bool IsActorInteractable(AActor* Actor) const;

private:
    /** 当前目标 */
    UPROPERTY()
    TObjectPtr<AActor> CurrentTarget;

    /** 交互检测半径 */
    UPROPERTY(EditDefaultsOnly, Category = "Interaction")
    float InteractionRange = 200.0f;
};