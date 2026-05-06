// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DBACommonEnums.generated.h"

UENUM(BlueprintType)
enum class EDBAElement : uint8
{
	None UMETA(DisplayName = "无"),
	Fire UMETA(DisplayName = "火"),
	Water UMETA(DisplayName = "水"),
	Wood UMETA(DisplayName = "木"),
	Gold UMETA(DisplayName = "金"),
	Earth UMETA(DisplayName = "土"),

	// Legacy GameDBA name kept for source compatibility.
	Metal = Gold UMETA(DisplayName = "金"),

	MAX = 6 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBAFiveCamp : uint8
{
	None UMETA(DisplayName = "无"),
	East UMETA(DisplayName = "东方"),
	West UMETA(DisplayName = "西方"),
	South UMETA(DisplayName = "南方"),
	North UMETA(DisplayName = "北方"),
	Center UMETA(DisplayName = "中央"),

	// Legacy GameDBA names kept for source compatibility.
	QingLong = East UMETA(DisplayName = "青龙阵营"),
	BaiHu = West UMETA(DisplayName = "白虎阵营"),
	ZhuQue = South UMETA(DisplayName = "朱雀阵营"),
	XuanWu = North UMETA(DisplayName = "玄武阵营"),
	QiLin = Center UMETA(DisplayName = "麒麟阵营"),

	MAX = 6 UMETA(Hidden)
};

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
	Pig UMETA(DisplayName = "猪"),
	MAX = 13 UMETA(Hidden)
};

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

UENUM(BlueprintType)
enum class EDBAPassiveType : uint8
{
	None UMETA(DisplayName = "无"),
	ElementResonance UMETA(DisplayName = "元素共鸣"),
	ZodiacBonus UMETA(DisplayName = "生肖加成"),
	FiveCampBonus UMETA(DisplayName = "阵营加成")
};

UENUM(BlueprintType)
enum class EDBAGameModeState : uint8
{
	None UMETA(DisplayName = "无"),
	Waiting UMETA(DisplayName = "等待中"),
	Playing UMETA(DisplayName = "游戏中"),
	Paused UMETA(DisplayName = "已暂停"),
	GameOver UMETA(DisplayName = "游戏结束")
};
