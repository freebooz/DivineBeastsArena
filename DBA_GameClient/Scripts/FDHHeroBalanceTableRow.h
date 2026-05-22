// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

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
