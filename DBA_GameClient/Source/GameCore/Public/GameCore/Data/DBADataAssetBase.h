// Copyright Freebooz Studio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DBADataAssetBase.generated.h"

/**
 * DBA 通用 DataAsset 基类
 */
UCLASS(BlueprintType)
class GAMECORE_API UDBADataAssetBase : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    /** 数据 ID */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Base")
    FName DataId;

    /** 显示名称 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Base")
    FText DisplayName;

    /** 英文名称 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Base")
    FText EnglishName;

    /** 描述 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Base", meta = (MultiLine = true))
    FText Description;

    /** 主标签 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Base")
    FGameplayTag PrimaryTag;

    /** 图标 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Base")
    TSoftObjectPtr<UTexture2D> Icon;

    /** 主题色 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Base")
    FLinearColor ThemeColor = FLinearColor::White;
};