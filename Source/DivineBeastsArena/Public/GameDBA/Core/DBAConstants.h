// Copyright Freebooz Games, Inc. All Rights Reserved.
// 神兽竞技场 - 核心常量定义

#pragma once

#include "CoreMinimal.h"
#include "DBAEnumsCore.h"

/**
 * DBA 核心常量命名空间
 * 包含全局常量、魔法数字、配置默认值
 */
namespace DBAConstants
{
	// ========================================
	// 游戏版本
	// ========================================

	/** 游戏版本字符串 */
	constexpr const TCHAR* GameVersion = TEXT("0.1.0");

	/** 最小客户端版本 */
	constexpr const TCHAR* MinClientVersion = TEXT("0.1.0");

	/** 数据模式版本 */
	constexpr int32 DataSchemaVersion = 1;

	// ========================================
	// 网络配置
	// ========================================

	/** 默认服务器端口 */
	constexpr int32 DefaultServerPort = 7777;

	/** 默认查询端口 */
	constexpr int32 DefaultQueryPort = 27015;

	/** 最大玩家数 */
	constexpr int32 MaxPlayers = 10;

	/** 最大旁观者数 */
	constexpr int32 MaxSpectators = 2;

	/** 网络更新频率（Hz） */
	constexpr float NetworkUpdateFrequency = 30.0f;

	/** 客户端预测窗口（秒） */
	constexpr float ClientPredictionWindow = 0.1f;

	// ========================================
	// 队伍配置
	// ========================================

	/** 默认队伍大小 */
	constexpr int32 DefaultTeamSize = 5;

	/** 最大队伍数量 */
	constexpr int32 MaxTeamCount = 5;

	// ========================================
	// 技能系统
	// ========================================

	/** 核心战斗输入数量（BasicAttack + Skill01~04 + Ultimate） */
	constexpr int32 CoreCombatInputCount = 6;

	/** 主动技能数量（Skill01~04） */
	constexpr int32 ActiveSkillCount = 4;

	/** 最大技能等级 */
	constexpr int32 MaxSkillLevel = 5;

	/** 最大英雄等级 */
	constexpr int32 MaxHeroLevel = 18;

	// ========================================
	// 元素克制倍率
	// ========================================

	/** 普通技能克制倍率 */
	constexpr float ElementCounter_Normal = 1.25f;

	/** 普通技能被克制倍率 */
	constexpr float ElementCountered_Normal = 0.80f;

	/** 生肖大招克制倍率 */
	constexpr float ElementCounter_Ultimate = 1.35f;

	/** 生肖大招被克制倍率 */
	constexpr float ElementCountered_Ultimate = 0.65f;

	/** 无克制关系倍率 */
	constexpr float ElementNeutral = 1.0f;

	// ========================================
	// 连锁系统
	// ========================================

	/** 最大连锁等级 */
	constexpr int32 MaxChainLevel = 10;

	/** 连锁1阶门槛 */
	constexpr int32 ChainTier1Threshold = 6;

	/** 连锁2阶门槛 */
	constexpr int32 ChainTier2Threshold = 10;

	/** 连锁1阶伤害加成 */
	constexpr float ChainTier1DamageBonus = 1.20f;

	/** 连锁2阶伤害加成 */
	constexpr float ChainTier2DamageBonus = 1.35f;

	/** 连锁终结已损生命值伤害百分比 */
	constexpr float ChainFinisher_HealthPercentDamage = 0.20f;

	/** 连锁终结多元素额外伤害 */
	constexpr float ChainFinisher_MultiElementBonus = 0.10f;

	/** 连锁终结多元素门槛 */
	constexpr int32 ChainFinisher_MultiElementThreshold = 3;

	/** 连锁超时时间（秒） */
	constexpr float ChainTimeout = 6.0f;

	// ========================================
	// 共鸣系统
	// ========================================

	/** 最大共鸣等级 */
	constexpr int32 MaxResonanceLevel = 4;

	/** 共鸣1级技能数量要求 */
	constexpr int32 ResonanceLevel1_SkillCount = 2;

	/** 共鸣2级技能数量要求 */
	constexpr int32 ResonanceLevel2_SkillCount = 3;

	/** 共鸣3级技能数量要求 */
	constexpr int32 ResonanceLevel3_SkillCount = 4;

	/** 共鸣4级技能数量要求 */
	constexpr int32 ResonanceLevel4_SkillCount = 5;

	/** 共鸣1级控制时间加成（秒） */
	constexpr float ResonanceLevel1_CCDuration = 0.25f;

	/** 共鸣1级护盾加成 */
	constexpr float ResonanceLevel1_ShieldBonus = 0.05f;

	/** 共鸣2级控制时间加成（秒） */
	constexpr float ResonanceLevel2_CCDuration = 0.50f;

	/** 共鸣2级护盾加成 */
	constexpr float ResonanceLevel2_ShieldBonus = 0.10f;

	/** 共鸣3级控制时间加成（秒） */
	constexpr float ResonanceLevel3_CCDuration = 0.75f;

	/** 共鸣3级护盾加成 */
	constexpr float ResonanceLevel3_ShieldBonus = 0.15f;

	/** 共鸣4级控制时间加成（秒） */
	constexpr float ResonanceLevel4_CCDuration = 1.0f;

	/** 共鸣4级护盾加成 */
	constexpr float ResonanceLevel4_ShieldBonus = 0.20f;

	// ========================================
	// 终极能量系统
	// ========================================

	/** 最大终极能量 */
	constexpr float MaxUltimateEnergy = 100.0f;

	/** 技能命中获得能量 */
	constexpr float UltimateEnergy_SkillHit = 3.0f;

	/** 击杀英雄获得能量 */
	constexpr float UltimateEnergy_HeroKill = 20.0f;

	/** 助攻获得能量 */
	constexpr float UltimateEnergy_Assist = 10.0f;

	/** 被动回复能量（每秒） */
	constexpr float UltimateEnergy_PassiveRegen = 1.0f;

	// ========================================
	// 属性系统
	// ========================================

	/** 核心平衡属性数量 */
	constexpr int32 CoreAttributeCount = 8;

	/** 防御减伤公式常量 */
	constexpr float DefenseReductionConstant = 100.0f;

	// ========================================
	// 对局配置
	// ========================================

	/** 默认对局时长限制（秒，0 表示无限制） */
	constexpr float DefaultMatchDurationLimit = 1800.0f;

	/** 准备阶段时长（秒） */
	constexpr float PreparationDuration = 60.0f;

	/** 英雄选择时长（秒） */
	constexpr float HeroSelectDuration = 30.0f;

	/** 元素选择时长（秒） */
	constexpr float ElementSelectDuration = 20.0f;

	/** 阵营选择时长（秒） */
	constexpr float FiveCampSelectDuration = 15.0f;

	/** 重生时间基础值（秒） */
	constexpr float RespawnTimeBase = 5.0f;

	/** 重生时间每级增加（秒） */
	constexpr float RespawnTimePerLevel = 2.0f;

	/** 最大重生时间（秒） */
	constexpr float RespawnTimeMax = 60.0f;

	// ========================================
	// UI 配置
	// ========================================

	/** 最大 Buff 显示数量 */
	constexpr int32 MaxBuffDisplayCount = 16;

	/** 最大 Debuff 显示数量 */
	constexpr int32 MaxDebuffDisplayCount = 16;

	/** 战斗文字显示时长（秒） */
	constexpr float CombatTextDuration = 2.0f;

	/** 通知显示时长（秒） */
	constexpr float NotificationDuration = 5.0f;

	// ========================================
	// 外部服务配置（可选）
	// ========================================

	/** 外部服务请求超时时间（秒） */
	constexpr float ExternalService_RequestTimeout = 5.0f;

	/** 外部服务最大重试次数 */
	constexpr int32 ExternalService_MaxRetryCount = 3;

	/** 外部服务熔断阈值（连续失败次数） */
	constexpr int32 ExternalService_CircuitBreakerThreshold = 5;

	/** 外部服务本地缓冲区限制（条） */
	constexpr int32 ExternalService_LocalBufferLimit = 1000;

	/** 外部服务上报间隔（秒） */
	constexpr float ExternalService_ReportInterval = 60.0f;

	// ========================================
	// 平台配置
	// ========================================

	/** Android 低配设备内存阈值（MB） */
	constexpr int32 Android_LowEndMemoryThreshold = 2048;

	/** Android 触控按钮最小尺寸（像素） */
	constexpr float Android_MinTouchButtonSize = 64.0f;

	/** Android 安全区边距（像素） */
	constexpr float Android_SafeAreaMargin = 20.0f;

	// ========================================
	// 调试配置
	// ========================================

	/** 是否启用调试绘制 */
	constexpr bool bEnableDebugDraw = false;

	/** 调试绘制持续时间（秒） */
	constexpr float DebugDrawDuration = 5.0f;

	/** 调试绘制厚度 */
	constexpr float DebugDrawThickness = 2.0f;
}

/**
 * DBA 元素克制关系命名空间
 * 定义五行相克关系
 */
namespace DBAElementCounter
{
	/**
	 * 获取元素克制的目标元素
	 * @param Element 攻击方元素
	 * @return 被克制的元素
	 */
	inline EDBAElementType GetCounteredElement(EDBAElementType Element)
	{
		switch (Element)
		{
		case EDBAElementType::Metal:  return EDBAElementType::Wood;   // 金克木
		case EDBAElementType::Wood:   return EDBAElementType::Earth;  // 木克土
		case EDBAElementType::Water:  return EDBAElementType::Fire;   // 水克火
		case EDBAElementType::Fire:   return EDBAElementType::Metal;  // 火克金
		case EDBAElementType::Earth:  return EDBAElementType::Water;  // 土克水
		default:                      return EDBAElementType::None;
		}
	}

	/**
	 * 获取克制当前元素的元素
	 * @param Element 防御方元素
	 * @return 克制该元素的元素
	 */
	inline EDBAElementType GetCounterElement(EDBAElementType Element)
	{
		switch (Element)
		{
		case EDBAElementType::Metal:  return EDBAElementType::Fire;   // 火克金
		case EDBAElementType::Wood:   return EDBAElementType::Metal;  // 金克木
		case EDBAElementType::Water:  return EDBAElementType::Earth;  // 土克水
		case EDBAElementType::Fire:   return EDBAElementType::Water;  // 水克火
		case EDBAElementType::Earth:  return EDBAElementType::Wood;   // 木克土
		default:                      return EDBAElementType::None;
		}
	}

	/**
	 * 判断元素克制关系
	 * @param AttackerElement 攻击方元素
	 * @param DefenderElement 防御方元素
	 * @return 克制结果
	 */
	inline EDBAElementCounterResult GetCounterResult(EDBAElementType AttackerElement, EDBAElementType DefenderElement)
	{
		if (AttackerElement == EDBAElementType::None || DefenderElement == EDBAElementType::None)
		{
			return EDBAElementCounterResult::None;
		}

		if (GetCounteredElement(AttackerElement) == DefenderElement)
		{
			return EDBAElementCounterResult::Counter;  // 克制
		}

		if (GetCounterElement(AttackerElement) == DefenderElement)
		{
			return EDBAElementCounterResult::Countered;  // 被克制
		}

		return EDBAElementCounterResult::None;  // 无关系
	}

	/**
	 * 获取元素克制倍率
	 * @param CounterResult 克制结果
	 * @param bIsUltimate 是否为生肖大招
	 * @return 伤害倍率
	 */
	inline float GetCounterMultiplier(EDBAElementCounterResult CounterResult, bool bIsUltimate)
	{
		if (CounterResult == EDBAElementCounterResult::Counter)
		{
			return bIsUltimate ? DBAConstants::ElementCounter_Ultimate : DBAConstants::ElementCounter_Normal;
		}
		else if (CounterResult == EDBAElementCounterResult::Countered)
		{
			return bIsUltimate ? DBAConstants::ElementCountered_Ultimate : DBAConstants::ElementCountered_Normal;
		}
		return DBAConstants::ElementNeutral;
	}
}

/**
 * DBA 路径常量命名空间
 * 定义常用资源路径
 */
namespace DBAPaths
{
	// ========================================
	// 内容路径
	// ========================================

	/** Core 资源根路径 */
	constexpr const TCHAR* CoreRoot = TEXT("/Game/Core");

	/** MobaBase 资源根路径 */
	constexpr const TCHAR* MobaBaseRoot = TEXT("/Game/MobaBase");

	/** DBA 项目资源根路径 */
	constexpr const TCHAR* DBARoot = TEXT("/Game/DBA");

	/** Lobby 资源根路径 */
	constexpr const TCHAR* LobbyRoot = TEXT("/Game/Lobby");

	/** Arena 资源根路径 */
	constexpr const TCHAR* ArenaRoot = TEXT("/Game/Arena");

	/** Practice 资源根路径 */
	constexpr const TCHAR* PracticeRoot = TEXT("/Game/Practice");

	/** UI 资源根路径 */
	constexpr const TCHAR* UIRoot = TEXT("/Game/UI");

	/** Developer 资源根路径 */
	constexpr const TCHAR* DeveloperRoot = TEXT("/Game/Developer");

	// ========================================
	// 数据表路径
	// ========================================

	/** 十二生肖数据表 */
	constexpr const TCHAR* DT_Zodiacs = TEXT("/Game/DBA/Data/Tables/DT_Zodiacs");

	/** 自然元素之力数据表 */
	constexpr const TCHAR* DT_Elements = TEXT("/Game/DBA/Data/Tables/DT_Elements");

	/** 五大阵营数据表 */
	constexpr const TCHAR* DT_FiveCamps = TEXT("/Game/DBA/Data/Tables/DT_FiveCamps");

	/** 技能数据表 */
	constexpr const TCHAR* DT_Skills = TEXT("/Game/DBA/Data/Tables/DT_Skills");

	/** 英雄数据表 */
	constexpr const TCHAR* DT_Heroes = TEXT("/Game/DBA/Data/Tables/DT_Heroes");

	/** 固定技能组数据表 */
	constexpr const TCHAR* DT_FixedSkillGroups = TEXT("/Game/DBA/Data/Tables/DT_FixedSkillGroups");

	// ========================================
	// 地图路径
	// ========================================

	/** 测试地图 */
	constexpr const TCHAR* Map_ArenaTest = TEXT("/Game/Maps/Arena_Test");

	/** 5v5 对局地图 */
	constexpr const TCHAR* Map_Arena5v5 = TEXT("/Game/Arena/Maps/Arena_5v5");

	/** 主大厅地图 */
	constexpr const TCHAR* Map_MainLobby = TEXT("/Game/Lobby/Maps/MainLobby");

	/** 新手村地图 */
	constexpr const TCHAR* Map_NewbieVillage = TEXT("/Game/Lobby/Maps/NewbieVillage");

	/** 练习地图 */
	constexpr const TCHAR* Map_Practice = TEXT("/Game/Practice/Maps/Practice_Training");
}