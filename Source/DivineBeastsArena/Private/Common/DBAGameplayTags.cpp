// Copyright Epic Games, Inc. All Rights Reserved.
// 神兽竞技场 - GameplayTag 根命名实现

#include "Common/DBAGameplayTags.h"
#include "GameplayTagsManager.h"
#include "Common/DBALogChannels.h"

const FDBAGameplayTags& FDBAGameplayTags::Get()
{
	static FDBAGameplayTags GameplayTags;
	return GameplayTags;
}

void FDBAGameplayTags::InitializeNativeTags()
{
	UE_LOG(LogDBACore, Log, TEXT("FDBAGameplayTags::InitializeNativeTags - 开始初始化 GameplayTag"));

	AddAllTags();

	UE_LOG(LogDBACore, Log, TEXT("FDBAGameplayTags::InitializeNativeTags - GameplayTag 初始化完成"));
}

FGameplayTag FDBAGameplayTags::AddTag(const FString& TagName, const FString& Comment)
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	// 请求添加原生 Tag
	FGameplayTag Tag = Manager.AddNativeGameplayTag(FName(*TagName), FString::Printf(TEXT("%s"), *Comment));

	return Tag;
}

void FDBAGameplayTags::AddAllTags()
{
	// 使用静态局部变量确保线程安全的单次初始化
	static FDBAGameplayTags GameplayTags;
	FDBAGameplayTags& TagsToInit = GameplayTags;

	// ========================================
	// State.* - 状态标签
	// ========================================

	TagsToInit.State_Dead = AddTag(
		TEXT("State.Dead"),
		TEXT("死亡状态，角色无法行动"));

	TagsToInit.State_Stunned = AddTag(
		TEXT("State.Stunned"),
		TEXT("眩晕状态，无法移动和施法"));

	TagsToInit.State_Rooted = AddTag(
		TEXT("State.Rooted"),
		TEXT("定身状态，无法移动但可施法"));

	TagsToInit.State_Silenced = AddTag(
		TEXT("State.Silenced"),
		TEXT("沉默状态，无法施法但可移动"));

	TagsToInit.State_Invulnerable = AddTag(
		TEXT("State.Invulnerable"),
		TEXT("无敌状态，免疫所有伤害"));

	TagsToInit.State_Untargetable = AddTag(
		TEXT("State.Untargetable"),
		TEXT("不可选中状态，无法被选为目标"));

	TagsToInit.State_Casting = AddTag(
		TEXT("State.Casting"),
		TEXT("施法状态，正在施放技能"));

	TagsToInit.State_Channeling = AddTag(
		TEXT("State.Channeling"),
		TEXT("引导状态，正在引导技能"));

	// ========================================
	// Status.* - 状态效果标签
	// ========================================

	TagsToInit.Status_Buff = AddTag(
		TEXT("Status.Buff"),
		TEXT("Buff 效果，增益状态"));

	TagsToInit.Status_Debuff = AddTag(
		TEXT("Status.Debuff"),
		TEXT("Debuff 效果，减益状态"));

	TagsToInit.Status_CC = AddTag(
		TEXT("Status.CC"),
		TEXT("控制效果，限制行动"));

	TagsToInit.Status_Damage = AddTag(
		TEXT("Status.Damage"),
		TEXT("伤害效果"));

	TagsToInit.Status_Heal = AddTag(
		TEXT("Status.Heal"),
		TEXT("治疗效果"));

	TagsToInit.Status_Shield = AddTag(
		TEXT("Status.Shield"),
		TEXT("护盾效果"));

	// ========================================
	// Ability.* - 技能标签
	// ========================================

	TagsToInit.Ability_Active = AddTag(
		TEXT("Ability.Active"),
		TEXT("主动技能"));

	TagsToInit.Ability_Passive = AddTag(
		TEXT("Ability.Passive"),
		TEXT("被动技能"));

	TagsToInit.Ability_Ultimate = AddTag(
		TEXT("Ability.Ultimate"),
		TEXT("生肖大招"));

	TagsToInit.Ability_BasicAttack = AddTag(
		TEXT("Ability.BasicAttack"),
		TEXT("普通攻击"));

	TagsToInit.Ability_Resonance = AddTag(
		TEXT("Ability.Resonance"),
		TEXT("元素共鸣"));

	// ========================================
	// Cooldown.* - 冷却标签
	// ========================================

	TagsToInit.Cooldown_Skill01 = AddTag(
		TEXT("Cooldown.Skill01"),
		TEXT("技能1冷却"));

	TagsToInit.Cooldown_Skill02 = AddTag(
		TEXT("Cooldown.Skill02"),
		TEXT("技能2冷却"));

	TagsToInit.Cooldown_Skill03 = AddTag(
		TEXT("Cooldown.Skill03"),
		TEXT("技能3冷却"));

	TagsToInit.Cooldown_Skill04 = AddTag(
		TEXT("Cooldown.Skill04"),
		TEXT("技能4冷却"));

	TagsToInit.Cooldown_Ultimate = AddTag(
		TEXT("Cooldown.Ultimate"),
		TEXT("生肖大招冷却"));

	// ========================================
	// Input.* - 输入标签
	// ========================================

	TagsToInit.Input_BasicAttack = AddTag(
		TEXT("Input.BasicAttack"),
		TEXT("普通攻击输入"));

	TagsToInit.Input_Skill01 = AddTag(
		TEXT("Input.Skill01"),
		TEXT("技能1输入"));

	TagsToInit.Input_Skill02 = AddTag(
		TEXT("Input.Skill02"),
		TEXT("技能2输入"));

	TagsToInit.Input_Skill03 = AddTag(
		TEXT("Input.Skill03"),
		TEXT("技能3输入"));

	TagsToInit.Input_Skill04 = AddTag(
		TEXT("Input.Skill04"),
		TEXT("技能4输入"));

	TagsToInit.Input_Ultimate = AddTag(
		TEXT("Input.Ultimate"),
		TEXT("生肖大招输入"));

	TagsToInit.Input_LockTarget = AddTag(
		TEXT("Input.LockTarget"),
		TEXT("锁定目标输入"));

	TagsToInit.Input_CancelCast = AddTag(
		TEXT("Input.CancelCast"),
		TEXT("取消施法输入"));

	// ========================================
	// Event.* - 事件标签
	// ========================================

	TagsToInit.Event_Damage = AddTag(
		TEXT("Event.Damage"),
		TEXT("伤害事件"));

	TagsToInit.Event_Heal = AddTag(
		TEXT("Event.Heal"),
		TEXT("治疗事件"));

	TagsToInit.Event_Death = AddTag(
		TEXT("Event.Death"),
		TEXT("死亡事件"));

	TagsToInit.Event_Respawn = AddTag(
		TEXT("Event.Respawn"),
		TEXT("重生事件"));

	TagsToInit.Event_LevelUp = AddTag(
		TEXT("Event.LevelUp"),
		TEXT("升级事件"));

	TagsToInit.Event_SkillHit = AddTag(
		TEXT("Event.SkillHit"),
		TEXT("技能命中事件"));

	TagsToInit.Event_ChainFinisher = AddTag(
		TEXT("Event.ChainFinisher"),
		TEXT("连锁终结事件"));

	// ========================================
	// UI.* - UI 标签
	// ========================================

	TagsToInit.UI_HUD = AddTag(
		TEXT("UI.HUD"),
		TEXT("HUD 层"));

	TagsToInit.UI_Menu = AddTag(
		TEXT("UI.Menu"),
		TEXT("菜单层"));

	TagsToInit.UI_Popup = AddTag(
		TEXT("UI.Popup"),
		TEXT("弹窗层"));

	TagsToInit.UI_Notification = AddTag(
		TEXT("UI.Notification"),
		TEXT("通知层"));

	TagsToInit.UI_CombatText = AddTag(
		TEXT("UI.CombatText"),
		TEXT("战斗文字层"));

	// ========================================
	// Match.* - 对局标签
	// ========================================

	TagsToInit.Match_Preparation = AddTag(
		TEXT("Match.Preparation"),
		TEXT("准备阶段"));

	TagsToInit.Match_InProgress = AddTag(
		TEXT("Match.InProgress"),
		TEXT("对局进行中"));

	TagsToInit.Match_Ending = AddTag(
		TEXT("Match.Ending"),
		TEXT("对局结束中"));

	TagsToInit.Match_Ended = AddTag(
		TEXT("Match.Ended"),
		TEXT("对局已结束"));

	// ========================================
	// Build.* - 构建标签
	// ========================================

	TagsToInit.Build_ZodiacSelect = AddTag(
		TEXT("Build.ZodiacSelect"),
		TEXT("生肖选择"));

	TagsToInit.Build_ElementSelect = AddTag(
		TEXT("Build.ElementSelect"),
		TEXT("元素选择"));

	TagsToInit.Build_FiveCampSelect = AddTag(
		TEXT("Build.FiveCampSelect"),
		TEXT("阵营选择"));

	TagsToInit.Build_Confirmed = AddTag(
		TEXT("Build.Confirmed"),
		TEXT("构建确认"));

	// ========================================
	// Element.* - 自然元素之力标签
	// ========================================

	TagsToInit.Element_Metal = AddTag(
		TEXT("Element.Metal"),
		TEXT("金元素"));

	TagsToInit.Element_Wood = AddTag(
		TEXT("Element.Wood"),
		TEXT("木元素"));

	TagsToInit.Element_Water = AddTag(
		TEXT("Element.Water"),
		TEXT("水元素"));

	TagsToInit.Element_Fire = AddTag(
		TEXT("Element.Fire"),
		TEXT("火元素"));

	TagsToInit.Element_Earth = AddTag(
		TEXT("Element.Earth"),
		TEXT("土元素"));

	// ========================================
	// FiveCamp.* - 五大阵营标签
	// ========================================

	TagsToInit.FiveCamp_Byakko = AddTag(
		TEXT("FiveCamp.Byakko"),
		TEXT("白虎阵营"));

	TagsToInit.FiveCamp_Qinglong = AddTag(
		TEXT("FiveCamp.Qinglong"),
		TEXT("青龙阵营"));

	TagsToInit.FiveCamp_Xuanwu = AddTag(
		TEXT("FiveCamp.Xuanwu"),
		TEXT("玄武阵营"));

	TagsToInit.FiveCamp_Zhuque = AddTag(
		TEXT("FiveCamp.Zhuque"),
		TEXT("朱雀阵营"));

	TagsToInit.FiveCamp_Kirin = AddTag(
		TEXT("FiveCamp.Kirin"),
		TEXT("麒麟阵营"));

	// ========================================
	// Zodiac.* - 十二生肖标签
	// ========================================

	TagsToInit.Zodiac_Rat = AddTag(
		TEXT("Zodiac.Rat"),
		TEXT("鼠"));

	TagsToInit.Zodiac_Ox = AddTag(
		TEXT("Zodiac.Ox"),
		TEXT("牛"));

	TagsToInit.Zodiac_Tiger = AddTag(
		TEXT("Zodiac.Tiger"),
		TEXT("虎"));

	TagsToInit.Zodiac_Rabbit = AddTag(
		TEXT("Zodiac.Rabbit"),
		TEXT("兔"));

	TagsToInit.Zodiac_Dragon = AddTag(
		TEXT("Zodiac.Dragon"),
		TEXT("龙"));

	TagsToInit.Zodiac_Snake = AddTag(
		TEXT("Zodiac.Snake"),
		TEXT("蛇"));

	TagsToInit.Zodiac_Horse = AddTag(
		TEXT("Zodiac.Horse"),
		TEXT("马"));

	TagsToInit.Zodiac_Goat = AddTag(
		TEXT("Zodiac.Goat"),
		TEXT("羊"));

	TagsToInit.Zodiac_Monkey = AddTag(
		TEXT("Zodiac.Monkey"),
		TEXT("猴"));

	TagsToInit.Zodiac_Rooster = AddTag(
		TEXT("Zodiac.Rooster"),
		TEXT("鸡"));

	TagsToInit.Zodiac_Dog = AddTag(
		TEXT("Zodiac.Dog"),
		TEXT("狗"));

	TagsToInit.Zodiac_Pig = AddTag(
		TEXT("Zodiac.Pig"),
		TEXT("猪"));

	// ========================================
	// Team.* - 队伍标签
	// ========================================

	TagsToInit.Team_Team1 = AddTag(
		TEXT("Team.Team1"),
		TEXT("队伍1"));

	TagsToInit.Team_Team2 = AddTag(
		TEXT("Team.Team2"),
		TEXT("队伍2"));

	TagsToInit.Team_Neutral = AddTag(
		TEXT("Team.Neutral"),
		TEXT("中立"));

	// ========================================
	// AI.* - AI 标签
	// ========================================

	TagsToInit.AI_Minion = AddTag(
		TEXT("AI.Minion"),
		TEXT("小兵 AI"));

	TagsToInit.AI_Monster = AddTag(
		TEXT("AI.Monster"),
		TEXT("野怪 AI"));

	TagsToInit.AI_Turret = AddTag(
		TEXT("AI.Turret"),
		TEXT("防御塔 AI"));

	TagsToInit.AI_Core = AddTag(
		TEXT("AI.Core"),
		TEXT("核心 AI"));

	TagsToInit.AI_NPC = AddTag(
		TEXT("AI.NPC"),
		TEXT("NPC AI"));

	// ========================================
	// Telemetry.* - 遥测标签（可选）
	// ========================================

	TagsToInit.Telemetry_Performance = AddTag(
		TEXT("Telemetry.Performance"),
		TEXT("性能遥测"));

	TagsToInit.Telemetry_Gameplay = AddTag(
		TEXT("Telemetry.Gameplay"),
		TEXT("游戏玩法遥测"));

	TagsToInit.Telemetry_Network = AddTag(
		TEXT("Telemetry.Network"),
		TEXT("网络遥测"));

	TagsToInit.Telemetry_Error = AddTag(
		TEXT("Telemetry.Error"),
		TEXT("错误遥测"));

	// ========================================
	// GameOps.* - 运维标签（可选）
	// ========================================

	TagsToInit.GameOps_ServerStatus = AddTag(
		TEXT("GameOps.ServerStatus"),
		TEXT("服务器状态"));

	TagsToInit.GameOps_MatchStatus = AddTag(
		TEXT("GameOps.MatchStatus"),
		TEXT("对局状态"));

	TagsToInit.GameOps_PlayerStatus = AddTag(
		TEXT("GameOps.PlayerStatus"),
		TEXT("玩家状态"));
}