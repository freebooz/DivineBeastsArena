// Copyright Freebooz Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DBARpcInterface.generated.h"

class AActor;

/**
 * FDBAAbilityRpcParams
 * RPC 能力激活参数
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAAbilityRpcParams
{
    GENERATED_BODY()

    /** 技能 Spec Handle */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    FGameplayAbilitySpecHandle AbilityHandle;

    /** 目标 Actor */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    TWeakObjectPtr<AActor> TargetActor;

    /** 额外参数 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    FVector_NetQuantize10 TargetLocation;
};

/**
 * UDBARpcInterface
 * RPC 接口定义基类
 */
UINTERFACE(BlueprintType)
class DIVINEBEASTSARENA_API UDBARpcInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * IDBARpcInterface
 * RPC 接口实现基类
 */
class DIVINEBEASTSARENA_API IDBARpcInterface
{
    GENERATED_BODY()
public:
};