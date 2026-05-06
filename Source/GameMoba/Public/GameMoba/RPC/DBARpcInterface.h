// Copyright Freebooz Games, Inc. All Rights Reserved.
// RPC接口层 - 定义客户端与服务端之间的RPC调用接口
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AbilitySystemComponent.h"
#include "DBARpcInterface.generated.h"

class AActor;

/**
 * FDBAAbilityRpcParams
 * RPC能力激活参数结构体
 * 客户端调用服务端技能激活时传递的参数
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAAbilityRpcParams
{
    GENERATED_BODY()

    /** 技能Spec Handle - 标识当前激活的技能实例 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    FGameplayAbilitySpecHandle AbilityHandle;

    /** 目标Actor - 技能的目标 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    TWeakObjectPtr<AActor> TargetActor;

    /** 目标位置 - 当没有目标时使用 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    FVector_NetQuantize10 TargetLocation;
};

/**
 * UDBARpcInterface
 * RPC接口定义基类
 * 定义客户端和服务端之间的RPC调用规范
 */
UINTERFACE(BlueprintType)
class DIVINEBEASTSARENA_API UDBARpcInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * IDBARpcInterface
 * RPC接口实现基类
 * 提供RPC接口的公共功能
 */
class DIVINEBEASTSARENA_API IDBARpcInterface
{
    GENERATED_BODY()
public:
    /** 客户端报告命中 (用于客户端预判+服务端验证) */
    UFUNCTION(Client, Reliable)
    virtual void ClientReportHit(FGameplayAbilitySpecHandle AbilityHandle, FVector_NetQuantize10 HitLocation, AActor* HitActor) = 0;

    /** 服务端推送完整状态同步 (用于断线重连) */
    UFUNCTION(Client, Reliable)
    virtual void ClientFullStateSync(float Health, float Energy, float Shield, float UltimateEnergy, int32 ChainLevel, int32 ResonanceLevel) = 0;

    /** 服务端推送移动校正 */
    UFUNCTION(Client, Reliable)
    virtual void ClientMoveCorrection(FVector_NetQuantize10 ServerLocation, float ServerTime) = 0;

    /** 服务端验证命中结果 */
    UFUNCTION(Client, Reliable)
    virtual void ClientHitConfirmed(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType) = 0;

    /** 服务端拒绝命中 (验证失败) */
    UFUNCTION(Client, Reliable)
    virtual void ClientHitRejected(FGameplayAbilitySpecHandle AbilityHandle) = 0;

    /** 服务端确认命中（带暴击标志） */
    UFUNCTION(Client, Reliable)
    virtual void ClientHitConfirmedWithCritical(FGameplayAbilitySpecHandle AbilityHandle, float Damage, FGameplayTag DamageType, bool bIsCritical, FVector_NetQuantize10 HitLocation) = 0;
};