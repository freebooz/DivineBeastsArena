// Copyright Freebooz Games, Inc. All Rights Reserved.
// 服务端RPC接口 - 客户端调用服务端的RPC方法
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DBARpcInterface.h"
#include "DBARpcServer.generated.h"

class AActor;

/**
 * UDBARpcServer
 * 服务端RPC接口 - 客户端调用服务端
 */
UINTERFACE(BlueprintType)
class DIVINEBEASTSARENA_API UDBARpcServer : public UDBARpcInterface
{
    GENERATED_BODY()
};

/**
 * IDBARpcServer
 * 服务端RPC接口实现 - 客户端调用服务端
 */
class DIVINEBEASTSARENA_API IDBARpcServer : public IDBARpcInterface
{
    GENERATED_BODY()

public:
    // 尝试激活技能
    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerTryActivateAbility(const FDBAAbilityRpcParams& Params) = 0;

    // 取消技能
    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerCancelAbility(FGameplayAbilitySpecHandle Handle) = 0;

    // 锁定目标
    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerLockTarget(AActor* TargetActor) = 0;

    // 移动到位置
    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerMoveTo(FVector_NetQuantize10 Location) = 0;

    // 请求攻击
    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerRequestAttack() = 0;

    // 终极技能
    UFUNCTION(Server, Reliable, WithValidation)
    virtual void ServerUltimateAbility(const FDBAAbilityRpcParams& Params) = 0;
};