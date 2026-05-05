// Copyright Freebooz Games, Inc. All Rights Reserved.
#include "GameMoba/RPC/DBARpcHandler.h"
#include "GameDBA/GAS/DBAAbilitySystemComponent.h"
#include "GameDBA/Character/DBAZodiacCharacterBase.h"

ADBARpcHandler::ADBARpcHandler()
{
    bReplicates = true;
    NetUpdateFrequency = 30.0f;
}

bool ADBARpcHandler::ServerTryActivateAbility_Validate(const FDBAAbilityRpcParams& Params)
{
    // 基础验证 - 可以在子类重写
    if (!Params.AbilityHandle.IsValid()) return false;
    return true;
}

void ADBARpcHandler::ServerTryActivateAbility_Implementation(const FDBAAbilityRpcParams& Params)
{
    if (ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner()))
    {
        if (UDBAAbilitySystemComponent* ASC = Character->GetDBAAbilitySystemComponent())
        {
            ASC->TryActivateAbility(Params.AbilityHandle);
        }
    }
}

bool ADBARpcHandler::ServerCancelAbility_Validate(FGameplayAbilitySpecHandle Handle)
{
    return Handle.IsValid();
}

void ADBARpcHandler::ServerCancelAbility_Implementation(FGameplayAbilitySpecHandle Handle)
{
    if (ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner()))
    {
        if (UDBAAbilitySystemComponent* ASC = Character->GetDBAAbilitySystemComponent())
        {
            ASC->CancelAbilitySpec(Handle);
        }
    }
}

bool ADBARpcHandler::ServerLockTarget_Validate(AActor* TargetActor)
{
    return ValidateTarget(TargetActor);
}

void ADBARpcHandler::ServerLockTarget_Implementation(AActor* TargetActor)
{
    // 实现目标锁定逻辑
}

bool ADBARpcHandler::ServerMoveTo_Validate(FVector_NetQuantize10 Location)
{
    // 简单验证位置有效性
    return !Location.IsZero();
}

void ADBARpcHandler::ServerMoveTo_Implementation(FVector_NetQuantize10 Location)
{
    // 实现移动逻辑
}

bool ADBARpcHandler::ServerRequestAttack_Validate()
{
    return true; // 普攻无特殊验证
}

void ADBARpcHandler::ServerRequestAttack_Implementation()
{
    // 实现普攻逻辑
}

bool ADBARpcHandler::ServerUltimateAbility_Validate(const FDBAAbilityRpcParams& Params)
{
    // 大招验证更严格
    if (!Params.AbilityHandle.IsValid()) return false;
    return ValidateEnergyCost(100.0f); // 大招需要能量
}

void ADBARpcHandler::ServerUltimateAbility_Implementation(const FDBAAbilityRpcParams& Params)
{
    ServerTryActivateAbility_Implementation(Params);
}

bool ADBARpcHandler::ValidateEnergyCost(float Cost) const
{
    return true; // 简化实现
}

bool ADBARpcHandler::ValidateTarget(AActor* Target) const
{
    return Target != nullptr && IsValid(Target);
}

bool ADBARpcHandler::ValidateCastRange(AActor* Target, float Range) const
{
    if (!Target) return false;
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= Range;
}

void ADBARpcHandler::ClientReceiveDamage_Implementation(float Damage, FVector_NetQuantize10 Position, FGameplayTag DamageType)
{
    // 客户端显示伤害
}

void ADBARpcHandler::ClientReceiveEffect_Implementation(FGameplayTag EffectTag, float Magnitude)
{
    // 客户端播放效果
}

void ADBARpcHandler::ClientReplicateState_Implementation(uint8 NewState, const FVector_NetQuantize10& Location)
{
    // 客户端同步状态
}

void ADBARpcHandler::ClientAbilityActivated_Implementation(FGameplayAbilitySpecHandle Handle)
{
    // 客户端显示技能激活
}

void ADBARpcHandler::ClientAbilityFailed_Implementation(FGameplayAbilitySpecHandle Handle, FGameplayTag FailureTag)
{
    // 客户端显示技能失败
}

void ADBARpcHandler::ClientReportHit_Implementation(FGameplayAbilitySpecHandle AbilityHandle, FVector_NetQuantize10 HitLocation, AActor* HitActor)
{
    // 服务端收到客户端报告的命中，开始验证
}

void ADBARpcHandler::ClientFullStateSync_Implementation(float Health, float Energy, float Shield, float UltimateEnergy, int32 ChainLevel, int32 ResonanceLevel)
{
    // 客户端收到完整状态同步，用于断线重连后恢复状态
}

void ADBARpcHandler::ClientMoveCorrection_Implementation(FVector_NetQuantize10 ServerLocation, float ServerTime)
{
    // 客户端收到服务端位置校正，平滑移动到正确位置
}

void ADBARpcHandler::ClientHitConfirmed_Implementation(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType)
{
    // 服务端确认命中，客户端播放命中特效
}

void ADBARpcHandler::ClientHitRejected_Implementation(FGameplayAbilitySpecHandle AbilityHandle)
{
    // 服务端拒绝命中，客户端撤销之前预判的命中效果
}