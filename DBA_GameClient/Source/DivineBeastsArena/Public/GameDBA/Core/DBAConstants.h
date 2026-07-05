// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

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

	/** 自然元素数量（Metal/Wood/Water/Fire/Earth） */
	constexpr int32 ElementCount = 5;

	/** 十二生肖英雄数量 */
	constexpr int32 ZodiacCount = 12;

	// ========================================
	// 技能系统
	// ========================================

	/** 核心战斗输入数量（BasicAttack + Skill01~04 + Ultimate） */
	constexpr int32 CoreCombatInputCount = 6;

	/** 主动技能数量（Skill01~04） */
	constexpr int32 ActiveSkillCount = 4;

	/** 竞技场战斗技能槽数量（Skill01~04 + Ultimate） */
	constexpr int32 ArenaCombatSkillSlotCount = DBAConstants::ActiveSkillCount + 1;

	/** 可玩技能槽数量（BasicAttack + Skill01~04 + Ultimate） */
	constexpr int32 PlayableSkillSlotCount = DBAConstants::CoreCombatInputCount;

	/** 可玩技能数组容量（保留 0 号位，技能槽使用 1-based 索引） */
	constexpr int32 PlayableSkillArraySize = DBAConstants::PlayableSkillSlotCount + 1;

	/** 最大技能等级 */
	constexpr int32 MaxSkillLevel = 5;

	/** 元素主动技能表每元素位置数量（占位 + Skill01~04） */
	constexpr int32 ElementAbilityPositionCount = DBAConstants::ActiveSkillCount + 1;

	/** 元素主动技能表期望行数（5 元素 × 5 位置） */
	constexpr int32 ElementActiveAbilityRowCount = DBAConstants::ElementCount * DBAConstants::ElementAbilityPositionCount;

	/** 固定技能组表期望行数（12 生肖 × 5 元素） */
	constexpr int32 FixedSkillGroupRowCount = DBAConstants::ZodiacCount * DBAConstants::ElementCount;

	/** 最大英雄等级 */
	constexpr int32 MaxHeroLevel = 18;

	// ========================================
	// 十二生肖技能名称定稿 (V15)
	// ========================================

	namespace DBASkillNames
	{
		// 子鼠·夜影灵牙｜影牙
		constexpr const TCHAR* Rat_Passive = TEXT("灵鼠印");
		constexpr const TCHAR* Rat_Skill01 = TEXT("钻影");
		constexpr const TCHAR* Rat_Skill02 = TEXT("飞牙");
		constexpr const TCHAR* Rat_Skill03 = TEXT("鼠遁");
		constexpr const TCHAR* Rat_Skill04 = TEXT("探穴");
		constexpr const TCHAR* Rat_Ultimate = TEXT("子夜现身");

		// 丑牛·撼山铁角｜铁角
		constexpr const TCHAR* Ox_Passive = TEXT("牛劲");
		constexpr const TCHAR* Ox_Skill01 = TEXT("角挑");
		constexpr const TCHAR* Ox_Skill02 = TEXT("铁蹄震");
		constexpr const TCHAR* Ox_Skill03 = TEXT("巨盾阵");
		constexpr const TCHAR* Ox_Skill04 = TEXT("回身顶");
		constexpr const TCHAR* Ox_Ultimate = TEXT("蛮牛开山");

		// 寅虎·啸山白虎｜白虎
		constexpr const TCHAR* Tiger_Passive = TEXT("虎威");
		constexpr const TCHAR* Tiger_Skill01 = TEXT("虎跃");
		constexpr const TCHAR* Tiger_Skill02 = TEXT("三裂爪");
		constexpr const TCHAR* Tiger_Skill03 = TEXT("虎啸提气");
		constexpr const TCHAR* Tiger_Skill04 = TEXT("追风爪");
		constexpr const TCHAR* Tiger_Ultimate = TEXT("白虎点将");

		// 卯兔·踏月玉灵｜玉灵
		constexpr const TCHAR* Rabbit_Passive = TEXT("轻月");
		constexpr const TCHAR* Rabbit_Skill01 = TEXT("踏月返");
		constexpr const TCHAR* Rabbit_Skill02 = TEXT("月牙轮");
		constexpr const TCHAR* Rabbit_Skill03 = TEXT("月闪");
		constexpr const TCHAR* Rabbit_Skill04 = TEXT("留月影");
		constexpr const TCHAR* Rabbit_Ultimate = TEXT("玉兔拜月");

		// 辰龙·御雷苍龙｜苍龙
		constexpr const TCHAR* Dragon_Passive = TEXT("龙雷印");
		constexpr const TCHAR* Dragon_Skill01 = TEXT("雷龙");
		constexpr const TCHAR* Dragon_Skill02 = TEXT("云雷阵");
		constexpr const TCHAR* Dragon_Skill03 = TEXT("龙鳞护");
		constexpr const TCHAR* Dragon_Skill04 = TEXT("雷门");
		constexpr const TCHAR* Dragon_Ultimate = TEXT("苍龙唤雷");

		// 巳蛇·幽毒灵蛇｜幽鳞
		constexpr const TCHAR* Snake_Passive = TEXT("蛇纹");
		constexpr const TCHAR* Snake_Skill01 = TEXT("蛇探");
		constexpr const TCHAR* Snake_Skill02 = TEXT("蛇环");
		constexpr const TCHAR* Snake_Skill03 = TEXT("蜕影步");
		constexpr const TCHAR* Snake_Skill04 = TEXT("花步");
		constexpr const TCHAR* Snake_Ultimate = TEXT("百花蛇舞");

		// 午马·赤焰雷蹄｜雷蹄
		constexpr const TCHAR* Horse_Passive = TEXT("奔势");
		constexpr const TCHAR* Horse_Skill01 = TEXT("雷蹄冲");
		constexpr const TCHAR* Horse_Skill02 = TEXT("赤焰旋");
		constexpr const TCHAR* Horse_Skill03 = TEXT("驰援");
		constexpr const TCHAR* Horse_Skill04 = TEXT("踏火印");
		constexpr const TCHAR* Horse_Ultimate = TEXT("奔雷入阵");

		// 未羊·玉角灵铃｜玉角
		constexpr const TCHAR* Goat_Passive = TEXT("铃愿");
		constexpr const TCHAR* Goat_Skill01 = TEXT("回春铃");
		constexpr const TCHAR* Goat_Skill02 = TEXT("暖玉盾");
		constexpr const TCHAR* Goat_Skill03 = TEXT("清铃音");
		constexpr const TCHAR* Goat_Skill04 = TEXT("愿光环");
		constexpr const TCHAR* Goat_Ultimate = TEXT("灵铃赐福");

		// 申猴·百戏灵猴｜灵猴
		constexpr const TCHAR* Monkey_Passive = TEXT("猴戏");
		constexpr const TCHAR* Monkey_Skill01 = TEXT("翻跃");
		constexpr const TCHAR* Monkey_Skill02 = TEXT("猴影");
		constexpr const TCHAR* Monkey_Skill03 = TEXT("云跳");
		constexpr const TCHAR* Monkey_Skill04 = TEXT("摘星手");
		constexpr const TCHAR* Monkey_Ultimate = TEXT("百猴闹场");

		// 酉鸡·破晓金翎｜金翎
		constexpr const TCHAR* Rooster_Passive = TEXT("晨鸣");
		constexpr const TCHAR* Rooster_Skill01 = TEXT("金鸡鸣");
		constexpr const TCHAR* Rooster_Skill02 = TEXT("金羽标");
		constexpr const TCHAR* Rooster_Skill03 = TEXT("明照");
		constexpr const TCHAR* Rooster_Skill04 = TEXT("晨羽阵");
		constexpr const TCHAR* Rooster_Ultimate = TEXT("破晓照天");

		// 戌狗·守门天犬｜天犬
		constexpr const TCHAR* Dog_Passive = TEXT("犬护");
		constexpr const TCHAR* Dog_Skill01 = TEXT("扑援");
		constexpr const TCHAR* Dog_Skill02 = TEXT("犬盾拍");
		constexpr const TCHAR* Dog_Skill03 = TEXT("灵鼻踪");
		constexpr const TCHAR* Dog_Skill04 = TEXT("护心圈");
		constexpr const TCHAR* Dog_Ultimate = TEXT("天犬守门");

		// 亥猪·岩甲獠牙｜獠牙
		constexpr const TCHAR* Pig_Passive = TEXT("厚甲");
		constexpr const TCHAR* Pig_Skill01 = TEXT("獠拱");
		constexpr const TCHAR* Pig_Skill02 = TEXT("岩甲蓄");
		constexpr const TCHAR* Pig_Skill03 = TEXT("锤震");
		constexpr const TCHAR* Pig_Skill04 = TEXT("福印");
		constexpr const TCHAR* Pig_Ultimate = TEXT("福山不动");
	}

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

	/** 元素共鸣表期望行数（5 元素 × 0~4 共鸣等级） */
	constexpr int32 ElementResonanceRowCount = DBAConstants::ElementCount * (DBAConstants::MaxResonanceLevel + 1);

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

	/** 共鸣1级伤害加成 */
	constexpr float ResonanceLevel1_DamageBonus = 0.05f;

	/** 共鸣2级控制时间加成（秒） */
	constexpr float ResonanceLevel2_CCDuration = 0.50f;

	/** 共鸣2级护盾加成 */
	constexpr float ResonanceLevel2_ShieldBonus = 0.10f;

	/** 共鸣2级伤害加成 */
	constexpr float ResonanceLevel2_DamageBonus = 0.10f;

	/** 共鸣3级控制时间加成（秒） */
	constexpr float ResonanceLevel3_CCDuration = 0.75f;

	/** 共鸣3级护盾加成 */
	constexpr float ResonanceLevel3_ShieldBonus = 0.15f;

	/** 共鸣3级伤害加成 */
	constexpr float ResonanceLevel3_DamageBonus = 0.15f;

	/** 共鸣4级控制时间加成（秒） */
	constexpr float ResonanceLevel4_CCDuration = 1.0f;

	/** 共鸣4级护盾加成 */
	constexpr float ResonanceLevel4_ShieldBonus = 0.20f;

	/** 共鸣4级伤害加成 */
	constexpr float ResonanceLevel4_DamageBonus = 0.20f;

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
	// 地图边界配置
	// ========================================

	/** 地图边界 - X轴最小值 */
	constexpr float MapBoundary_MinX = -10000.0f;

	/** 地图边界 - X轴最大值 */
	constexpr float MapBoundary_MaxX = 10000.0f;

	/** 地图边界 - Y轴最小值 */
	constexpr float MapBoundary_MinY = -10000.0f;

	/** 地图边界 - Y轴最大值 */
	constexpr float MapBoundary_MaxY = 10000.0f;

	/** 地图边界 - Z轴最小值 */
	constexpr float MapBoundary_MinZ = -100.0f;

	/** 地图边界 - Z轴最大值 */
	constexpr float MapBoundary_MaxZ = 10000.0f;

	/** 地图边界检测阈值（超过此距离进行校正） */
	constexpr float MapBoundary_CorrectionThreshold = 500.0f;

	// ========================================
	// 战斗配置
	// ========================================

	/** 默认攻击范围 */
	constexpr float DefaultAttackRange = 500.0f;

	/** 基础伤害百分比（目标最大生命值） */
	constexpr float BaseDamagePercentOfMaxHealth = 0.1f;

	/** 暴击概率 */
	constexpr float CriticalChance = 0.1f;

	/** 暴击伤害倍率 */
	constexpr float CriticalDamageMultiplier = 2.0f;

	/** 默认基础伤害 */
	constexpr float DefaultBaseDamage = 50.0f;

	/** 动画速度基数 */
	constexpr float AnimationSpeedBase = 600.0f;

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
