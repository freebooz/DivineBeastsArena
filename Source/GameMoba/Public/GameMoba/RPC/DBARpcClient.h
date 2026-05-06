// Copyright Freebooz Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DBARpcInterface.h"
#include "DBARpcClient.generated.h"

/**
 * UDBARpcClient
 * 客户端 RPC 接口 - 服务端通知客户端
 */
UINTERFACE(BlueprintType)
class DIVINEBEASTSARENA_API UDBARpcClient : public UDBARpcInterface
{
    GENERATED_BODY()
};

/**
 * IDBARpcClient
 * 客户端 RPC 接口实现 - 服务端通知客户端
 */
class DIVINEBEASTSARENA_API IDBARpcClient : public IDBARpcInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(Client, Reliable)
    virtual void ClientReceiveDamage(float Damage, FVector_NetQuantize10 Position, FGameplayTag DamageType) = 0;

    UFUNCTION(Client, Reliable)
    virtual void ClientReceiveEffect(FGameplayTag EffectTag, float Magnitude) = 0;

    UFUNCTION(Client, Reliable)
    virtual void ClientReplicateState(uint8 NewState, const FVector_NetQuantize10& Location) = 0;

    UFUNCTION(Client, Reliable)
    virtual void ClientAbilityActivated(FGameplayAbilitySpecHandle Handle) = 0;

    UFUNCTION(Client, Reliable)
    virtual void ClientAbilityFailed(FGameplayAbilitySpecHandle Handle, FGameplayTag FailureTag) = 0;
};