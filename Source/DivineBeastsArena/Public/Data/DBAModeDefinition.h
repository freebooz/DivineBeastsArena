// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DBAModeDefinition.generated.h"

/**
 * 游戏模式定义行
 * 定义游戏模式的规则、显示名称、描述、支持地图、队伍配置
 * 用于前台模式选择、匹配队列、对局规则初始化
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAModeDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 游戏模式唯一标识符（例如 "5v5", "3v3", "Practice"） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode", meta = (DisplayName = "模式标识符"))
	FName ModeId;

	/** 游戏模式中文显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode", meta = (DisplayName = "中文名称"))
	FText DisplayNameCN;

	/** 游戏模式英文显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode", meta = (DisplayName = "英文名称"))
	FText DisplayNameEN;

	/** 游戏模式描述 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode", meta = (DisplayName = "描述", MultiLine = true))
	FText Description;

	/** 游戏模式图标路径 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode", meta = (DisplayName = "图标路径"))
	TSoftObjectPtr<UTexture2D> IconTexture;

	/** 队伍数量（例如 2 表示两队对抗） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode|Team", meta = (DisplayName = "队伍数量"))
	int32 TeamCount = 2;

	/** 每队玩家数量（例如 5 表示 5v5） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode|Team", meta = (DisplayName = "每队玩家数"))
	int32 PlayersPerTeam = 5;

	/** 是否允许 AI 填充（用于 Practice 模式） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode|Team", meta = (DisplayName = "允许AI填充"))
	bool bAllowAIFill = false;

	/** 对局时长限制（秒，0 表示无限制） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode|Rule", meta = (DisplayName = "对局时长限制(秒)"))
	int32 MatchDurationSeconds = 1800;

	/** 胜利条件类型（例如 "DestroyCore", "KillCount", "TimeLimit"） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode|Rule", meta = (DisplayName = "胜利条件类型"))
	FName VictoryConditionType;

	/** 胜利条件参数（例如击杀数量、占领点数量） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode|Rule", meta = (DisplayName = "胜利条件参数"))
	int32 VictoryConditionValue = 0;

	/** 是否启用重生机制 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode|Rule", meta = (DisplayName = "启用重生"))
	bool bEnableRespawn = true;

	/** 重生时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode|Rule", meta = (DisplayName = "重生时间(秒)"))
	float RespawnTimeSeconds = 10.0f;

	/** 是否启用野怪系统 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode|Rule", meta = (DisplayName = "启用野怪"))
	bool bEnableJungle = true;

	/** 是否启用防御塔系统 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode|Rule", meta = (DisplayName = "启用防御塔"))
	bool bEnableTurrets = true;

	/** 是否启用小兵系统 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode|Rule", meta = (DisplayName = "启用小兵"))
	bool bEnableMinions = true;

	/** 支持的地图列表 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode", meta = (DisplayName = "支持地图列表"))
	TArray<FName> SupportedMaps;

	/** 是否在当前版本可用 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode", meta = (DisplayName = "是否可用"))
	bool bIsAvailable = true;

	/** 是否需要排位匹配 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode", meta = (DisplayName = "需要排位匹配"))
	bool bRequiresRankedMatch = false;

	/** 最低等级要求 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mode", meta = (DisplayName = "最低等级要求"))
	int32 MinLevelRequired = 1;

	FDBAModeDefinitionRow()
		: ModeId(NAME_None)
		, TeamCount(2)
		, PlayersPerTeam(5)
		, bAllowAIFill(false)
		, MatchDurationSeconds(1800)
		, VictoryConditionType(NAME_None)
		, VictoryConditionValue(0)
		, bEnableRespawn(true)
		, RespawnTimeSeconds(10.0f)
		, bEnableJungle(true)
		, bEnableTurrets(true)
		, bEnableMinions(true)
		, bIsAvailable(true)
		, bRequiresRankedMatch(false)
		, MinLevelRequired(1)
	{
	}
};
