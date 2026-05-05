// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DBACommonEnums.generated.h"

/**
 * 元素类型枚举
 * 定义游戏中的自然元素
 */
UENUM(BlueprintType)
enum class EDBAElement : uint8
{
	None UMETA(DisplayName = "无"),
	Fire UMETA(DisplayName = "火"),
	Water UMETA(DisplayName = "水"),
	Wood UMETA(DisplayName = "木"),
	Gold UMETA(DisplayName = "金"),
	Earth UMETA(DisplayName = "土")
};

/**
 * 五大阵营枚举
 * 定义游戏中的阵营系统
 */
UENUM(BlueprintType)
enum class EDBAFiveCamp : uint8
{
	None UMETA(DisplayName = "无"),
	East UMETA(DisplayName = "东方"),
	West UMETA(DisplayName = "西方"),
	South UMETA(DisplayName = "南方"),
	North UMETA(DisplayName = "北方"),
	Center UMETA(DisplayName = "中央")
};

/**
 * 十二生肖枚举
 * 定义游戏中的生肖系统
 */
UENUM(BlueprintType)
enum class EDBAZodiac : uint8
{
	None UMETA(DisplayName = "无"),
	Rat UMETA(DisplayName = "鼠"),
	Ox UMETA(DisplayName = "牛"),
	Tiger UMETA(DisplayName = "虎"),
	Rabbit UMETA(DisplayName = "兔"),
	Dragon UMETA(DisplayName = "龙"),
	Snake UMETA(DisplayName = "蛇"),
	Horse UMETA(DisplayName = "马"),
	Goat UMETA(DisplayName = "羊"),
	Monkey UMETA(DisplayName = "猴"),
	Rooster UMETA(DisplayName = "鸡"),
	Dog UMETA(DisplayName = "狗"),
	Pig UMETA(DisplayName = "猪")
};

/**
 * 技能槽位枚举
 */
UENUM(BlueprintType)
enum class EDBASkillSlot : uint8
{
	None UMETA(DisplayName = "无"),
	Passive UMETA(DisplayName = "被动技能"),
	Active1 UMETA(DisplayName = "主动技能1"),
	Active2 UMETA(DisplayName = "主动技能2"),
	Active3 UMETA(DisplayName = "主动技能3"),
	Active4 UMETA(DisplayName = "主动技能4"),
	Ultimate UMETA(DisplayName = "终极技能")
};

/**
 * 被动技能类型枚举
 */
UENUM(BlueprintType)
enum class EDBAPassiveType : uint8
{
	None UMETA(DisplayName = "无"),
	ElementResonance UMETA(DisplayName = "元素共鸣"),
	ZodiacBonus UMETA(DisplayName = "生肖加成"),
	FiveCampBonus UMETA(DisplayName = "阵营加成")
};

/**
 * 游戏模式状态枚举
 */
UENUM(BlueprintType)
enum class EDBAGameModeState : uint8
{
	None UMETA(DisplayName = "无"),
	Waiting UMETA(DisplayName = "等待中"),
	Playing UMETA(DisplayName = "游戏中"),
	Paused UMETA(DisplayName = "已暂停"),
	GameOver UMETA(DisplayName = "游戏结束")
};
