// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明十二生肖占位外观染色 DataTable 行结构。
- 阅读重点：行名使用生肖枚举名（Rat、Ox…），BodyTint 驱动 M_DBA_RuntimeTint 染色。
- 修改提示：新增生肖时同步更新 DT_ZodiacPlaceholderTints 源表并重新导入。
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "DBAZodiacPlaceholderTintRow.generated.h"

/**
 * 十二生肖占位外观染色行
 * 在启用「共用占位模型 + 材质染色」策略时，按生肖区分 BodyTint。
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAZodiacPlaceholderTintRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Placeholder")
	EDBAZodiac Zodiac = EDBAZodiac::None;

	/** 主体染色（写入 RuntimeTint 材质的 Tint/BaseColor 等参数） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Placeholder")
	FLinearColor BodyTint = FLinearColor::White;

	/** 辅助高光色（可选，用于 UI 灯光或第二材质槽） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Zodiac|Placeholder")
	FLinearColor AccentTint = FLinearColor::White;
};
