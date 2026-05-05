// Copyright Freebooz Games, Inc. All Rights Reserved.
// 神兽竞技场 - GameplayTag 根命名定义

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * DBA GameplayTag 管理器
 * 负责注册和管理项目使用的 GameplayTag
 *
 * GameplayTag 命名规范：
 * - 使用点号分隔层级：Category.Subcategory.Specific
 * - 使用 PascalCase 命名
 * - 避免使用下划线
 *
 * 什么状态适合 GameplayTag：
 * - 需要网络复制的状态标签
 * - 需要在 GAS 中查询的状态
 * - 需要触发 GameplayCue 的事件
 * - 需要在蓝图中使用的标签
 *
 * 什么状态适合枚举：
 * - 固定数量的状态（如十二生肖、自然元素之力）
 * - 需要 switch 语句处理的状态
 * - 不需要网络复制的本地状态
 *
 * 什么状态适合 Attribute / GE Stack / Runtime DTO：
 * - 需要数值计算的状态（如生命值、能量）
 * - 需要堆叠的状态（如 Buff 层数）
 * - 临时运行时数据（如伤害计算中间结果）
 */
struct DIVINEBEASTSARENA_API FDBAGameplayTags
{
public:
	/**
	 * 获取单例实例
	 */
	static const FDBAGameplayTags& Get();

	/**
	 * 初始化所有 GameplayTag
	 * 在模块启动时调用
	 */
	static void InitializeNativeTags();

	// ========================================
	// State.* - 状态标签
	// ========================================

	/** State.Dead - 死亡状态 */
	mutable FGameplayTag State_Dead;

	/** State.Stunned - 眩晕状态 */
	mutable FGameplayTag State_Stunned;

	/** State.Rooted - 定身状态 */
	mutable FGameplayTag State_Rooted;

	/** State.Silenced - 沉默状态 */
	mutable FGameplayTag State_Silenced;

	/** State.Invulnerable - 无敌状态 */
	mutable FGameplayTag State_Invulnerable;

	/** State.Untargetable - 不可选中状态 */
	mutable FGameplayTag State_Untargetable;

	/** State.Casting - 施法状态 */
	mutable FGameplayTag State_Casting;

	/** State.Channeling - 引导状态 */
	mutable FGameplayTag State_Channeling;

	// ========================================
	// Status.* - 状态效果标签
	// ========================================

	/** Status.Buff - Buff 效果 */
	mutable FGameplayTag Status_Buff;

	/** Status.Debuff - Debuff 效果 */
	mutable FGameplayTag Status_Debuff;

	/** Status.CC - 控制效果 */
	mutable FGameplayTag Status_CC;

	/** Status.Damage - 伤害效果 */
	mutable FGameplayTag Status_Damage;

	/** Status.Heal - 治疗效果 */
	mutable FGameplayTag Status_Heal;

	/** Status.Shield - 护盾效果 */
	mutable FGameplayTag Status_Shield;

	// ========================================
	// Ability.* - 技能标签
	// ========================================

	/** Ability.Active - 主动技能 */
	mutable FGameplayTag Ability_Active;

	/** Ability.Passive - 被动技能 */
	mutable FGameplayTag Ability_Passive;

	/** Ability.Ultimate - 生肖大招 */
	mutable FGameplayTag Ability_Ultimate;

	/** Ability.BasicAttack - 普通攻击 */
	mutable FGameplayTag Ability_BasicAttack;

	/** Ability.Resonance - 元素共鸣 */
	mutable FGameplayTag Ability_Resonance;

	// ========================================
	// Cooldown.* - 冷却标签
	// ========================================

	/** Cooldown.Skill01 - 技能1冷却 */
	mutable FGameplayTag Cooldown_Skill01;

	/** Cooldown.Skill02 - 技能2冷却 */
	mutable FGameplayTag Cooldown_Skill02;

	/** Cooldown.Skill03 - 技能3冷却 */
	mutable FGameplayTag Cooldown_Skill03;

	/** Cooldown.Skill04 - 技能4冷却 */
	mutable FGameplayTag Cooldown_Skill04;

	/** Cooldown.Ultimate - 生肖大招冷却 */
	mutable FGameplayTag Cooldown_Ultimate;

	// ========================================
	// Input.* - 输入标签
	// ========================================

	/** Input.BasicAttack - 普通攻击输入 */
	mutable FGameplayTag Input_BasicAttack;

	/** Input.Skill01 - 技能1输入 */
	mutable FGameplayTag Input_Skill01;

	/** Input.Skill02 - 技能2输入 */
	mutable FGameplayTag Input_Skill02;

	/** Input.Skill03 - 技能3输入 */
	mutable FGameplayTag Input_Skill03;

	/** Input.Skill04 - 技能4输入 */
	mutable FGameplayTag Input_Skill04;

	/** Input.Ultimate - 生肖大招输入 */
	mutable FGameplayTag Input_Ultimate;

	/** Input.LockTarget - 锁定目标输入 */
	mutable FGameplayTag Input_LockTarget;

	/** Input.CancelCast - 取消施法输入 */
	mutable FGameplayTag Input_CancelCast;

	// ========================================
	// Event.* - 事件标签
	// ========================================

	/** Event.Damage - 伤害事件 */
	mutable FGameplayTag Event_Damage;

	/** Event.Heal - 治疗事件 */
	mutable FGameplayTag Event_Heal;

	/** Event.Death - 死亡事件 */
	mutable FGameplayTag Event_Death;

	/** Event.Respawn - 重生事件 */
	mutable FGameplayTag Event_Respawn;

	/** Event.LevelUp - 升级事件 */
	mutable FGameplayTag Event_LevelUp;

	/** Event.SkillHit - 技能命中事件 */
	mutable FGameplayTag Event_SkillHit;

	/** Event.ChainFinisher - 连锁终结事件 */
	mutable FGameplayTag Event_ChainFinisher;

	// ========================================
	// UI.* - UI 标签
	// ========================================

	/** UI.HUD - HUD 层 */
	mutable FGameplayTag UI_HUD;

	/** UI.Menu - 菜单层 */
	mutable FGameplayTag UI_Menu;

	/** UI.Popup - 弹窗层 */
	mutable FGameplayTag UI_Popup;

	/** UI.Notification - 通知层 */
	mutable FGameplayTag UI_Notification;

	/** UI.CombatText - 战斗文字层 */
	mutable FGameplayTag UI_CombatText;

	// ========================================
	// Match.* - 对局标签
	// ========================================

	/** Match.Preparation - 准备阶段 */
	mutable FGameplayTag Match_Preparation;

	/** Match.InProgress - 对局进行中 */
	mutable FGameplayTag Match_InProgress;

	/** Match.Ending - 对局结束中 */
	mutable FGameplayTag Match_Ending;

	/** Match.Ended - 对局已结束 */
	mutable FGameplayTag Match_Ended;

	// ========================================
	// Build.* - 构建标签（英雄选择流程）
	// ========================================

	/** Build.ZodiacSelect - 生肖选择 */
	mutable FGameplayTag Build_ZodiacSelect;

	/** Build.ElementSelect - 元素选择 */
	mutable FGameplayTag Build_ElementSelect;

	/** Build.FiveCampSelect - 阵营选择 */
	mutable FGameplayTag Build_FiveCampSelect;

	/** Build.Confirmed - 构建确认 */
	mutable FGameplayTag Build_Confirmed;

	// ========================================
	// Element.* - 自然元素之力标签
	// ========================================

	/** Element.Metal - 金元素 */
	mutable FGameplayTag Element_Metal;

	/** Element.Wood - 木元素 */
	mutable FGameplayTag Element_Wood;

	/** Element.Water - 水元素 */
	mutable FGameplayTag Element_Water;

	/** Element.Fire - 火元素 */
	mutable FGameplayTag Element_Fire;

	/** Element.Earth - 土元素 */
	mutable FGameplayTag Element_Earth;

	// ========================================
	// FiveCamp.* - 五大阵营标签
	// ========================================

	/** FiveCamp.Byakko - 白虎阵营 */
	mutable FGameplayTag FiveCamp_Byakko;

	/** FiveCamp.Qinglong - 青龙阵营 */
	mutable FGameplayTag FiveCamp_Qinglong;

	/** FiveCamp.Xuanwu - 玄武阵营 */
	mutable FGameplayTag FiveCamp_Xuanwu;

	/** FiveCamp.Zhuque - 朱雀阵营 */
	mutable FGameplayTag FiveCamp_Zhuque;

	/** FiveCamp.Kirin - 麒麟阵营 */
	mutable FGameplayTag FiveCamp_Kirin;

	// ========================================
	// Zodiac.* - 十二生肖标签
	// ========================================

	/** Zodiac.Rat - 鼠 */
	mutable FGameplayTag Zodiac_Rat;

	/** Zodiac.Ox - 牛 */
	mutable FGameplayTag Zodiac_Ox;

	/** Zodiac.Tiger - 虎 */
	mutable FGameplayTag Zodiac_Tiger;

	/** Zodiac.Rabbit - 兔 */
	mutable FGameplayTag Zodiac_Rabbit;

	/** Zodiac.Dragon - 龙 */
	mutable FGameplayTag Zodiac_Dragon;

	/** Zodiac.Snake - 蛇 */
	mutable FGameplayTag Zodiac_Snake;

	/** Zodiac.Horse - 马 */
	mutable FGameplayTag Zodiac_Horse;

	/** Zodiac.Goat - 羊 */
	mutable FGameplayTag Zodiac_Goat;

	/** Zodiac.Monkey - 猴 */
	mutable FGameplayTag Zodiac_Monkey;

	/** Zodiac.Rooster - 鸡 */
	mutable FGameplayTag Zodiac_Rooster;

	/** Zodiac.Dog - 狗 */
	mutable FGameplayTag Zodiac_Dog;

	/** Zodiac.Pig - 猪 */
	mutable FGameplayTag Zodiac_Pig;

	// ========================================
	// Team.* - 队伍标签
	// ========================================

	/** Team.Team1 - 队伍1 */
	mutable FGameplayTag Team_Team1;

	/** Team.Team2 - 队伍2 */
	mutable FGameplayTag Team_Team2;

	/** Team.Neutral - 中立 */
	mutable FGameplayTag Team_Neutral;

	// ========================================
	// AI.* - AI 标签
	// ========================================

	/** AI.Minion - 小兵 AI */
	mutable FGameplayTag AI_Minion;

	/** AI.Monster - 野怪 AI */
	mutable FGameplayTag AI_Monster;

	/** AI.Turret - 防御塔 AI */
	mutable FGameplayTag AI_Turret;

	/** AI.Core - 核心 AI */
	mutable FGameplayTag AI_Core;

	/** AI.NPC - NPC AI */
	mutable FGameplayTag AI_NPC;

	// ========================================
	// Telemetry.* - 遥测标签（可选）
	// ========================================

	/** Telemetry.Performance - 性能遥测 */
	mutable FGameplayTag Telemetry_Performance;

	/** Telemetry.Gameplay - 游戏玩法遥测 */
	mutable FGameplayTag Telemetry_Gameplay;

	/** Telemetry.Network - 网络遥测 */
	mutable FGameplayTag Telemetry_Network;

	/** Telemetry.Error - 错误遥测 */
	mutable FGameplayTag Telemetry_Error;

	// ========================================
	// GameOps.* - 运维标签（可选）
	// ========================================

	/** GameOps.ServerStatus - 服务器状态 */
	mutable FGameplayTag GameOps_ServerStatus;

	/** GameOps.MatchStatus - 对局状态 */
	mutable FGameplayTag GameOps_MatchStatus;

	/** GameOps.PlayerStatus - 玩家状态 */
	mutable FGameplayTag GameOps_PlayerStatus;

private:
	/**
	 * 添加 GameplayTag
	 * @param TagName Tag 名称
	 * @param Comment 注释
	 * @return 添加的 Tag
	 */
	static FGameplayTag AddTag(const FString& TagName, const FString& Comment);

	/**
	 * 添加所有原生 Tag
	 */
	static void AddAllTags();
};