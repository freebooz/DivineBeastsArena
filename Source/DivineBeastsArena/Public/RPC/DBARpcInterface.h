// Copyright Freebooz Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DBARpcInterface.generated.h"

class AActor;

/**
 * IDBARpcInterface
 * RPC 接口定义 - 所有可网络调用的功能都实现此接口
 */
UINTERFACE()
class DIVINEBEASTSARENA_API IDBARpcInterface : public UInterface
{
    GENERATED_BODY()
};

/**
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