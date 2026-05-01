// Copyright Freebooz Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "DBARpcInterface.h"
#include "DBARpcServer.generated.h"

/**
 * IDBARpcServer
 * 服务端 RPC 接口 - 客户端调用服务端
 */
UINTERFACE()
class DIVINEBEASTSARENA_API IDBARpcServer : public IDBARpcInterface
{
    GENERATED_BODY()
};

class DIVINEBEASTSARENA_API IDBARpcServer
{
    GENERATED_BODY()

public:
    // 技能相关
    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerTryActivateAbility(const FDBAAbilityRpcParams& Params) = 0;

    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerCancelAbility(FGameplayAbilitySpecHandle Handle) = 0;

    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerLockTarget(AActor* TargetActor) = 0;

    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerMoveTo(FVector_NetQuantize10 Location) = 0;

    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerRequestAttack() = 0;

    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerUltimateAbility(const FDBAAbilityRpcParams& Params) = 0;
};