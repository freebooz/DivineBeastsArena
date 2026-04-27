// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DBAInteractableInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UDBAInteractionInterface : public UInterface
{
    GENERATED_BODY()
};

class DIVINEBEASTSARENA_API IDBAInteractionInterface
{
    GENERATED_BODY()

public:
    /**
     * 获取交互提示文本
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "DBA|Interaction")
    FText GetInteractionPromptText() const;

    /**
     * 检查是否可以交互
     * @param Interactor 交互者 Actor
     * @return 是否可以交互
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "DBA|Interaction")
    bool CanInteract(AActor* Interactor) const;

    /**
     * 执行交互
     * @param Interactor 交互者 Actor
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "DBA|Interaction")
    void Interact(AActor* Interactor);

    /**
     * 获取交互范围
     */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "DBA|Interaction")
    float GetInteractionRange() const;
};