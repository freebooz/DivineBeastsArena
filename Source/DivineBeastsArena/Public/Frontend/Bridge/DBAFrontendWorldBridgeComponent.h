// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DBAFrontendWorldBridgeComponent.generated.h"

class UDBAFrontendSubsystem;

UCLASS(ClassGroup=(DBA), meta=(BlueprintSpawnableComponent))
class DIVINEBEASTSARENA_API UDBAFrontendWorldBridgeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDBAFrontendWorldBridgeComponent();

    /**
     * 注册到全局 Frontend 子系统
     */
    UFUNCTION(BlueprintCallable, Category = "DBA|Frontend")
    void RegisterToFrontendSubsystem();

    /**
     * 从全局 Frontend 子系统注销
     */
    UFUNCTION(BlueprintCallable, Category = "DBA|Frontend")
    void UnregisterFromFrontendSubsystem();

    /**
     * 获取当前 Frontend 状态
     */
    UFUNCTION(BlueprintPure, Category = "DBA|Frontend")
    FString GetCurrentFrontendState() const { return CurrentFrontendState; }

protected:
    /** 当前 Frontend 状态 */
    UPROPERTY()
    FString CurrentFrontendState;
};