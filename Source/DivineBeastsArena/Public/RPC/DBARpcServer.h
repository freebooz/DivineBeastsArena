// Copyright Freebooz Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DBARpcInterface.h"
#include "DBARpcServer.generated.h"

/**
 * UDBARpcServer
 * 服务端 RPC 接口 - 客户端调用服务端
 */
UINTERFACE(BlueprintType)
class DIVINEBEASTSARENA_API UDBARpcServer : public UDBARpcInterface
{
    GENERATED_BODY()
};

/**
 * IDBARpcServer
 * 服务端 RPC 接口实现 - 客户端调用服务端
 */
class DIVINEBEASTSARENA_API IDBARpcServer : public IDBARpcInterface
{
    GENERATED_BODY()

public:
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