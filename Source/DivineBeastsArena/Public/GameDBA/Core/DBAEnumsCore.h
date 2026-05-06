// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "DBAEnumsCore.generated.h"

UENUM(BlueprintType)
enum class EDBAZodiacType : uint8
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
	Rooster UMETA(DisplayName = "曜鸣神鸡"),
	Dog UMETA(DisplayName = "狗"),
	Pig UMETA(DisplayName = "福岳灵猪"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBAElementType : uint8
{
	None UMETA(DisplayName = "无"),
	Metal UMETA(DisplayName = "金"),
	Wood UMETA(DisplayName = "木"),
	Water UMETA(DisplayName = "水"),
	Fire UMETA(DisplayName = "火"),
	Earth UMETA(DisplayName = "土"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBAFiveCampType : uint8
{
	None UMETA(DisplayName = "无"),
	BaiHu UMETA(DisplayName = "白虎阵营"),
	QingLong UMETA(DisplayName = "青龙阵营"),
	XuanWu UMETA(DisplayName = "玄武阵营"),
	ZhuQue UMETA(DisplayName = "朱雀阵营"),
	QiLin UMETA(DisplayName = "麒麟阵营"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBATeamId : uint8
{
	None UMETA(DisplayName = "无队伍"),
	Team1 UMETA(DisplayName = "队伍1"),
	Team2 UMETA(DisplayName = "队伍2"),
	Team3 UMETA(DisplayName = "队伍3"),
	Team4 UMETA(DisplayName = "队伍4"),
	Team5 UMETA(DisplayName = "队伍5"),
	Neutral UMETA(DisplayName = "中立"),
	Spectator UMETA(DisplayName = "旁观者"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBASkillInputSlot : uint8
{
	None UMETA(DisplayName = "无"),
	BasicAttack UMETA(DisplayName = "普通攻击"),
	Skill01 UMETA(DisplayName = "技能1"),
	Skill02 UMETA(DisplayName = "技能2"),
	Skill03 UMETA(DisplayName = "技能3"),
	Skill04 UMETA(DisplayName = "技能4"),
	Ultimate UMETA(DisplayName = "生肖大招"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBACombatInputSlot : uint8
{
	None UMETA(DisplayName = "无"),
	LockTarget UMETA(DisplayName = "锁定目标"),
	CancelCast UMETA(DisplayName = "取消施法"),
	Ping UMETA(DisplayName = "信号"),
	Scoreboard UMETA(DisplayName = "计分板"),
	Menu UMETA(DisplayName = "菜单"),
	Chat UMETA(DisplayName = "聊天"),
	Map UMETA(DisplayName = "地图"),
	Interact UMETA(DisplayName = "交互"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBASkillType : uint8
{
	None UMETA(DisplayName = "无"),
	Passive UMETA(DisplayName = "被动技能"),
	Active UMETA(DisplayName = "主动技能"),
	Ultimate UMETA(DisplayName = "生肖大招"),
	Resonance UMETA(DisplayName = "元素共鸣"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBAPlatformFamily : uint8
{
	Unknown UMETA(DisplayName = "未知"),
	PC UMETA(DisplayName = "PC"),
	Android UMETA(DisplayName = "Android"),
	iOS UMETA(DisplayName = "iOS"),
	Linux UMETA(DisplayName = "Linux"),
	DedicatedServer UMETA(DisplayName = "Dedicated Server"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBAGameFlowDomain : uint8
{
	None UMETA(DisplayName = "无"),
	Startup UMETA(DisplayName = "启动"),
	Frontend UMETA(DisplayName = "前台"),
	Loading UMETA(DisplayName = "加载"),
	Arena UMETA(DisplayName = "对局"),
	Practice UMETA(DisplayName = "练习"),
	Result UMETA(DisplayName = "结算"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBAFrontendStep : uint8
{
	None UMETA(DisplayName = "无"),
	Login UMETA(DisplayName = "登录"),
	CharacterSelect UMETA(DisplayName = "角色选择"),
	MainLobby UMETA(DisplayName = "主大厅"),
	NewbieVillage UMETA(DisplayName = "新手村"),
	Party UMETA(DisplayName = "组队"),
	Queue UMETA(DisplayName = "匹配队列"),
	MatchFound UMETA(DisplayName = "匹配成功"),
	ReadyCheck UMETA(DisplayName = "准备检查"),
	HeroSelect UMETA(DisplayName = "英雄选择"),
	ElementSelect UMETA(DisplayName = "元素选择"),
	FiveCampSelect UMETA(DisplayName = "阵营选择"),
	LoadingArena UMETA(DisplayName = "加载对局"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBAExternalServiceState : uint8
{
	Disabled UMETA(DisplayName = "已禁用"),
	Idle UMETA(DisplayName = "空闲"),
	Connecting UMETA(DisplayName = "连接中"),
	Connected UMETA(DisplayName = "已连接"),
	Disconnected UMETA(DisplayName = "已断开"),
	Error UMETA(DisplayName = "错误"),
	CircuitOpen UMETA(DisplayName = "熔断开启"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBATelemetrySourceType : uint8
{
	None UMETA(DisplayName = "无"),
	Client UMETA(DisplayName = "客户端"),
	DedicatedServer UMETA(DisplayName = "Dedicated Server"),
	Editor UMETA(DisplayName = "编辑器"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBAElementCounterResult : uint8
{
	None UMETA(DisplayName = "无关系"),
	Counter UMETA(DisplayName = "克制"),
	Countered UMETA(DisplayName = "被克制"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBAResonanceLevel : uint8
{
	None UMETA(DisplayName = "无共鸣"),
	Level1 UMETA(DisplayName = "共鸣1级"),
	Level2 UMETA(DisplayName = "共鸣2级"),
	Level3 UMETA(DisplayName = "共鸣3级"),
	Level4 UMETA(DisplayName = "共鸣4级"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBAChainTier : uint8
{
	None UMETA(DisplayName = "无连锁"),
	Tier1 UMETA(DisplayName = "连锁1阶"),
	Tier2 UMETA(DisplayName = "连锁2阶"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBAMatchMode : uint8
{
	None UMETA(DisplayName = "无"),
	Arena_5v5 UMETA(DisplayName = "5v5 对局"),
	Arena_3v3 UMETA(DisplayName = "3v3 对局"),
	Arena_1v1 UMETA(DisplayName = "1v1 对局"),
	Practice UMETA(DisplayName = "练习模式"),
	Custom UMETA(DisplayName = "自定义"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBAMatchResult : uint8
{
	None UMETA(DisplayName = "无"),
	Victory UMETA(DisplayName = "胜利"),
	Defeat UMETA(DisplayName = "失败"),
	Draw UMETA(DisplayName = "平局"),
	Surrender UMETA(DisplayName = "投降"),
	Disconnect UMETA(DisplayName = "断线"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBAAbilityActivationPolicy : uint8
{
	OnInputTriggered UMETA(DisplayName = "输入触发时"),
	WhileInputActive UMETA(DisplayName = "输入持续时"),
	OnSpawn UMETA(DisplayName = "生成时"),
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDBAAbilityInputID : uint8
{
	None UMETA(DisplayName = "无"),
	Confirm UMETA(DisplayName = "确认"),
	Cancel UMETA(DisplayName = "取消"),
	BasicAttack UMETA(DisplayName = "普通攻击"),
	Skill01 UMETA(DisplayName = "技能1"),
	Skill02 UMETA(DisplayName = "技能2"),
	Skill03 UMETA(DisplayName = "技能3"),
	Skill04 UMETA(DisplayName = "技能4"),
	Ultimate UMETA(DisplayName = "生肖大招"),
	MAX UMETA(Hidden)
};
