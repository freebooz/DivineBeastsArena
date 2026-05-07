// Copyright Freebooz Games, Inc. All Rights Reserved.
// 英雄数值平衡数据表结构体 (由 Python 脚本自动生成)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FDHHeroBalanceTableRow.generated.h"

/**
 * FDHHeroBalanceTableRow
 * 英雄数值平衡数据表行结构
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDHHeroBalanceTableRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    // 基本信息
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString ZodiacType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString CharacterName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString ShortName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString CoreRole;

    // 能力评分 (1-5)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 Survivability;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 Damage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 Control;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 Mobility;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 Support;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 Difficulty;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 TeamFightImpact;

    // 分路信息
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString RecommendedLane;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString TeamRole;

    // 优劣势
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString Advantages;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString Weaknesses;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString BestPartners;
};
