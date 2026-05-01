// Copyright Freebooz Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "RPC/DBARpcServer.h"
#include "RPC/DBARpcClient.h"
#include "DBARpcHandler.generated.h"

/**
 * ADBARpcHandler
 * RPC 处理器 - 挂在 Pawn 或 Controller 上处理网络调用
 */
UCLASS(Blueprintable)
class DIVINEBEASTSARENA_API ADBARpcHandler : public AActor, public IDBARpcServer, public IDBARpcClient
{
    GENERATED_BODY()

public:
    ADBARpcHandler();

    // ==================== IDBARpcServer 接口实现 ====================
    virtual void ServerTryActivateAbility_Implementation(const FDBAAbilityRpcParams& Params);
    virtual bool ServerTryActivateAbility_Validate(const FDBAAbilityRpcParams& Params);

    virtual void ServerCancelAbility_Implementation(FGameplayAbilitySpecHandle Handle);
    virtual bool ServerCancelAbility_Validate(FGameplayAbilitySpecHandle Handle);

    virtual void ServerLockTarget_Implementation(AActor* TargetActor);
    virtual bool ServerLockTarget_Validate(AActor* TargetActor);

    virtual void ServerMoveTo_Implementation(FVector_NetQuantize10 Location);
    virtual bool ServerMoveTo_Validate(FVector_NetQuantize10 Location);

    virtual void ServerRequestAttack_Implementation();
    virtual bool ServerRequestAttack_Validate();

    virtual void ServerUltimateAbility_Implementation(const FDBAAbilityRpcParams& Params);
    virtual bool ServerUltimateAbility_Validate(const FDBAAbilityRpcParams& Params);

    // ==================== IDBARpcClient 接口实现 ====================
    UFUNCTION(Client, Reliable)
    virtual void ClientReceiveDamage_Implementation(float Damage, FVector_NetQuantize10 Position, FGameplayTag DamageType);

    UFUNCTION(Client, Reliable)
    virtual void ClientReceiveEffect_Implementation(FGameplayTag EffectTag, float Magnitude);

    UFUNCTION(Client, Reliable)
    virtual void ClientReplicateState_Implementation(uint8 NewState, const FVector_NetQuantize10& Location);

    UFUNCTION(Client, Reliable)
    virtual void ClientAbilityActivated_Implementation(FGameplayAbilitySpecHandle Handle);

    UFUNCTION(Client, Reliable)
    virtual void ClientAbilityFailed_Implementation(FGameplayAbilitySpecHandle Handle, FGameplayTag FailureTag);

protected:
    /** 验证能量是否足够 */
    bool ValidateEnergyCost(float Cost) const;

    /** 验证目标是否有效 */
    bool ValidateTarget(AActor* Target) const;

    /** 验证距离是否在技能范围内 */
    bool ValidateCastRange(AActor* Target, float Range) const;
};