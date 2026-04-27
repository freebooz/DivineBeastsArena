// Copyright Freebooz Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DBADataAssetBase.generated.h"

/**
 * DBA 通用 DataAsset 基类
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBADataAssetBase : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UDBADataAssetBase();

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