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
UCLASS()
class DIVINEBEASTSARENA_API ADBARpcHandler : public AActor, public IDBARpcServer, public IDBARpcClient
{
    GENERATED_BODY()

public:
    ADBARpcHandler();

    // IDBARpcServer 接口实现
    virtual void ServerTryActivateAbility_Implementation(const FDBAAbilityRpcParams& Params) override;
    virtual bool ServerTryActivateAbility_Validate(const FDBAAbilityRpcParams& Params) override;

    virtual void ServerCancelAbility_Implementation(FGameplayAbilitySpecHandle Handle) override;
    virtual bool ServerCancelAbility_Validate(FGameplayAbilitySpecHandle Handle) override;

    virtual void ServerLockTarget_Implementation(AActor* TargetActor) override;
    virtual bool ServerLockTarget_Validate(AActor* TargetActor) override;

    virtual void ServerMoveTo_Implementation(FVector_NetQuantize10 Location) override;
    virtual bool ServerMoveTo_Validate(FVector_NetQuantize10 Location) override;

    virtual void ServerRequestAttack_Implementation() override;
    virtual bool ServerRequestAttack_Validate() override;

    virtual void ServerUltimateAbility_Implementation(const FDBAAbilityRpcParams& Params) override;
    virtual bool ServerUltimateAbility_Validate(const FDBAAbilityRpcParams& Params) override;

    // IDBARpcClient 接口实现
    UFUNCTION(Client, Reliable)
    virtual void ClientReceiveDamage_Implementation(float Damage, FVector_NetQuantize10 Position, FGameplayTag DamageType) override;

    UFUNCTION(Client, Reliable)
    virtual void ClientReceiveEffect_Implementation(FGameplayTag EffectTag, float Magnitude) override;

    UFUNCTION(Client, Reliable)
    virtual void ClientReplicateState_Implementation(uint8 NewState, const FVector_NetQuantize10& Location) override;

    UFUNCTION(Client, Reliable)
    virtual void ClientAbilityActivated_Implementation(FGameplayAbilitySpecHandle Handle) override;

    UFUNCTION(Client, Reliable)
    virtual void ClientAbilityFailed_Implementation(FGameplayAbilitySpecHandle Handle, FGameplayTag FailureTag) override;

protected:
    /** 验证能量是否足够 */
    bool ValidateEnergyCost(float Cost) const;

    /** 验证目标是否有效 */
    bool ValidateTarget(AActor* Target) const;

    /** 验证距离是否在技能范围内 */
    bool ValidateCastRange(AActor* Target, float Range) const;
};