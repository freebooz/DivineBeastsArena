// Copyright Freebooz Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "GameMoba/RPC/DBARpcServer.h"
#include "GameMoba/RPC/DBARpcClient.h"
#include "DBARpcHandler.generated.h"

class ADBAZodiacCharacterBase;

/**
 * ADBARpcHandler
 * RPC 处理器 - 挂在 Pawn 或 Controller 上处理网络调用
 * 放置在 DivineBeastsArena 模块中因为需要访问 ADBAZodiacCharacterBase
 */
UCLASS(Blueprintable)
class DIVINEBEASTSARENA_API ADBARpcHandler : public AActor, public IDBARpcServer, public IDBARpcClient
{
    GENERATED_BODY()

public:
    ADBARpcHandler();

    // ==================== IDBARpcServer 接口实现 ====================
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

    // ==================== IDBARpcClient 接口实现 ====================
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

    // ==================== IDBARpcInterface 客户端回调 ====================
    UFUNCTION(Client, Reliable)
    virtual void ClientReportHit_Implementation(FGameplayAbilitySpecHandle AbilityHandle, FVector_NetQuantize10 HitLocation, AActor* HitActor) override;

    UFUNCTION(Client, Reliable)
    virtual void ClientFullStateSync_Implementation(float Health, float Energy, float Shield, float UltimateEnergy, int32 ChainLevel, int32 ResonanceLevel) override;

    UFUNCTION(Client, Reliable)
    virtual void ClientMoveCorrection_Implementation(FVector_NetQuantize10 ServerLocation, float ServerTime) override;

    UFUNCTION(Client, Reliable)
    virtual void ClientHitConfirmed_Implementation(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType) override;

    UFUNCTION(Client, Reliable)
    virtual void ClientHitRejected_Implementation(FGameplayAbilitySpecHandle AbilityHandle) override;

    UFUNCTION(Client, Reliable)
    virtual void ClientHitConfirmedWithCritical_Implementation(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType, bool bIsCritical, FVector_NetQuantize10 HitLocation) override;

protected:
    /** 验证能量是否足够 */
    bool ValidateEnergyCost(float Cost) const;

    /** 验证目标是否有效 */
    bool ValidateTarget(AActor* Target) const;

    /** 验证距离是否在技能范围内 */
    bool ValidateCastRange(AActor* Target, float Range) const;

    /** 查找攻击目标 */
    AActor* FindAttackTarget(ADBAZodiacCharacterBase* Character) const;

    /** 计算普攻伤害 */
    float CalculateAttackDamage(ADBAZodiacCharacterBase* Attacker, AActor* Target, bool& OutbIsCritical) const;

    /** 判断是否是敌方 */
    bool IsEnemy(AActor* ActorA, AActor* ActorB) const;
};