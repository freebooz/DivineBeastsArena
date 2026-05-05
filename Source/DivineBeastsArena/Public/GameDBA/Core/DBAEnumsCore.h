// Copyright Freebooz Games, Inc. All Rights Reserved.
// 神兽竞技场 - 核心枚举定义

#pragma once

#include "CoreMinimal.h"
#include "DBAEnumsCore.generated.h"

/**
 * EDBAZodiacType
 * 十二生肖类型
 * 决定英雄身份、外观剪影、动画基调、生肖大招来源
 */
UENUM(BlueprintType)
enum class EDBAZodiacType : uint8
{
	None        UMETA(DisplayName = "无"),

	Rat         UMETA(DisplayName = "鼠"),
	Ox          UMETA(DisplayName = "牛"),
	Tiger       UMETA(DisplayName = "虎"),
	Rabbit      UMETA(DisplayName = "兔"),
	Dragon      UMETA(DisplayName = "龙"),
	Snake       UMETA(DisplayName = "蛇"),
	Horse       UMETA(DisplayName = "马"),
	Goat        UMETA(DisplayName = "羊"),
	Monkey      UMETA(DisplayName = "猴"),
	Rooster     UMETA(DisplayName = "曜鸣神鸡"),
	Dog         UMETA(DisplayName = "狗"),
	Pig         UMETA(DisplayName = "福岳灵猪"),

	MAX         UMETA(Hidden)
};

/**
 * EDBAElementType
 * 自然元素之力类型
 * 决定克制关系、属性加成、技能元素
 * 五行相克：Metal 克 Wood，Wood 克 Earth，Earth 克 Water，Water 克 Fire，Fire 克 Metal
 */
UENUM(BlueprintType)
enum class EDBAElementType : uint8
{
	None        UMETA(DisplayName = "无"),

	Metal       UMETA(DisplayName = "金"),
	Wood        UMETA(DisplayName = "木"),
	Water       UMETA(DisplayName = "水"),
	Fire        UMETA(DisplayName = "火"),
	Earth       UMETA(DisplayName = "土"),

	MAX         UMETA(Hidden)
};

/**
 * EDBAFiveCampType
 * 五大阵营类型
 * 决定外观、法相、阵营徽记、特效主题、UI 图标风格、声音风格
 * 与 TeamId 完全解耦，纯表现层
 */
UENUM(BlueprintType)
enum class EDBAFiveCampType : uint8
{
	None        UMETA(DisplayName = "无"),

	BaiHu       UMETA(DisplayName = "白虎阵营"),  // 西方、金属、锋利、白色
	QingLong    UMETA(DisplayName = "青龙阵营"),  // 东方、木、生长、青色
	XuanWu      UMETA(DisplayName = "玄武阵营"),  // 北方、水、防御、黑色
	ZhuQue      UMETA(DisplayName = "朱雀阵营"),  // 南方、火、爆发、红色
	QiLin      UMETA(DisplayName = "麒麟阵营"),  // 中央、土、稳固、黄色

	MAX         UMETA(Hidden)
};

/**
 * EDBATeamId
 * 队伍 ID
 * 用于战斗逻辑、敌我判定、复制分组
 * 与 Zodiac / Element / FiveCamp 完全解耦
 */
UENUM(BlueprintType)
enum class EDBATeamId : uint8
{
	None        UMETA(DisplayName = "无队伍"),

	Team1       UMETA(DisplayName = "队伍1"),
	Team2       UMETA(DisplayName = "队伍2"),
	Team3       UMETA(DisplayName = "队伍3"),
	Team4       UMETA(DisplayName = "队伍4"),
	Team5       UMETA(DisplayName = "队伍5"),

	Neutral     UMETA(DisplayName = "中立"),      // 野怪、中立单位
	Spectator   UMETA(DisplayName = "旁观者"),

	MAX         UMETA(Hidden)
};

/**
 * EDBASkillInputSlot
 * 技能输入槽位
 * 核心战斗输入最多 6 个：BasicAttack + Skill01~04 + Ultimate
 */
UENUM(BlueprintType)
enum class EDBASkillInputSlot : uint8
{
	None            UMETA(DisplayName = "无"),

	BasicAttack     UMETA(DisplayName = "普通攻击"),
	Skill01         UMETA(DisplayName = "技能1"),
	Skill02         UMETA(DisplayName = "技能2"),
	Skill03         UMETA(DisplayName = "技能3"),
	Skill04         UMETA(DisplayName = "技能4"),
	Ultimate        UMETA(DisplayName = "生肖大招"),

	MAX             UMETA(Hidden)
};

/**
 * EDBACombatInputSlot
 * 战斗相关输入槽位（非技能）
 * 系统输入不计入 6 个核心战斗输入
 */
UENUM(BlueprintType)
enum class EDBACombatInputSlot : uint8
{
	None            UMETA(DisplayName = "无"),

	LockTarget      UMETA(DisplayName = "锁定目标"),
	CancelCast      UMETA(DisplayName = "取消施法"),
	Ping            UMETA(DisplayName = "信号"),
	Scoreboard      UMETA(DisplayName = "计分板"),
	Menu            UMETA(DisplayName = "菜单"),
	Chat            UMETA(DisplayName = "聊天"),
	Map             UMETA(DisplayName = "地图"),
	Interact        UMETA(DisplayName = "交互"),

	MAX             UMETA(Hidden)
};

/**
 * EDBASkillType
 * 技能类型
 * 用于技能分类、UI 显示、数据统计
 */
UENUM(BlueprintType)
enum class EDBASkillType : uint8
{
	None            UMETA(DisplayName = "无"),

	Passive         UMETA(DisplayName = "被动技能"),      // 自动生效，不占输入
	Active          UMETA(DisplayName = "主动技能"),      // Skill01~04
	Ultimate        UMETA(DisplayName = "生肖大招"),      // ZodiacUltimate
	Resonance       UMETA(DisplayName = "元素共鸣"),      // 自动生效，不占输入

	MAX             UMETA(Hidden)
};

/**
 * EDBAPlatformFamily
 * 平台类型
 * 用于平台特定逻辑、输入适配、资源加载
 */
UENUM(BlueprintType)
enum class EDBAPlatformFamily : uint8
{
	Unknown         UMETA(DisplayName = "未知"),

	PC              UMETA(DisplayName = "PC"),
	Android         UMETA(DisplayName = "Android"),
	iOS             UMETA(DisplayName = "iOS"),
	Linux           UMETA(DisplayName = "Linux"),

	DedicatedServer UMETA(DisplayName = "Dedicated Server"),

	MAX             UMETA(Hidden)
};

/**
 * EDBAGameFlowDomain
 * 游戏流程域
 * 用于全局流程状态机、资源加载、UI 层级管理
 */
UENUM(BlueprintType)
enum class EDBAGameFlowDomain : uint8
{
	None            UMETA(DisplayName = "无"),

	Startup         UMETA(DisplayName = "启动"),          // 启动画面、初始化
	Frontend        UMETA(DisplayName = "前台"),          // 登录、大厅、组队、匹配
	Loading         UMETA(DisplayName = "加载"),          // 加载对局资源
	Arena           UMETA(DisplayName = "对局"),          // 对局内
	Practice        UMETA(DisplayName = "练习"),          // 练习模式
	Result          UMETA(DisplayName = "结算"),          // 对局结算

	MAX             UMETA(Hidden)
};

/**
 * EDBAFrontendStep
 * 前台流程步骤
 * 用于前台子系统状态管理
 */
UENUM(BlueprintType)
enum class EDBAFrontendStep : uint8
{
	None                UMETA(DisplayName = "无"),

	Login               UMETA(DisplayName = "登录"),
	CharacterSelect     UMETA(DisplayName = "角色选择"),
	MainLobby           UMETA(DisplayName = "主大厅"),
	NewbieVillage       UMETA(DisplayName = "新手村"),
	Party               UMETA(DisplayName = "组队"),
	Queue               UMETA(DisplayName = "匹配队列"),
	MatchFound          UMETA(DisplayName = "匹配成功"),
	ReadyCheck          UMETA(DisplayName = "准备检查"),
	HeroSelect          UMETA(DisplayName = "英雄选择"),
	ElementSelect       UMETA(DisplayName = "元素选择"),
	FiveCampSelect      UMETA(DisplayName = "阵营选择"),
	LoadingArena        UMETA(DisplayName = "加载对局"),

	MAX                 UMETA(Hidden)
};

/**
 * EDBAExternalServiceState
 * 外部服务状态
 * 用于可选的 Monitoring / GameOps 客户端状态管理
 */
UENUM(BlueprintType)
enum class EDBAExternalServiceState : uint8
{
	Disabled        UMETA(DisplayName = "已禁用"),        // 配置关闭
	Idle            UMETA(DisplayName = "空闲"),          // 未连接
	Connecting      UMETA(DisplayName = "连接中"),
	Connected       UMETA(DisplayName = "已连接"),
	Disconnected    UMETA(DisplayName = "已断开"),
	Error           UMETA(DisplayName = "错误"),
	CircuitOpen     UMETA(DisplayName = "熔断开启"),      // 连续失败，停止请求

	MAX             UMETA(Hidden)
};

/**
 * EDBATelemetrySourceType
 * 遥测数据来源类型
 * 用于可选的监控上报数据分类
 */
UENUM(BlueprintType)
enum class EDBATelemetrySourceType : uint8
{
	None            UMETA(DisplayName = "无"),

	Client          UMETA(DisplayName = "客户端"),
	DedicatedServer UMETA(DisplayName = "Dedicated Server"),
	Editor          UMETA(DisplayName = "编辑器"),

	MAX             UMETA(Hidden)
};

/**
 * EDBAElementCounterResult
 * 元素克制结果
 * 用于伤害计算、UI 提示
 */
UENUM(BlueprintType)
enum class EDBAElementCounterResult : uint8
{
	None            UMETA(DisplayName = "无关系"),        // 倍率 1.0
	Counter         UMETA(DisplayName = "克制"),          // 倍率 1.25（普通）/ 1.35（大招）
	Countered       UMETA(DisplayName = "被克制"),        // 倍率 0.80（普通）/ 0.65（大招）

	MAX             UMETA(Hidden)
};

/**
 * EDBAResonanceLevel
 * 元素共鸣等级
 * 根据同元素技能数量决定
 */
UENUM(BlueprintType)
enum class EDBAResonanceLevel : uint8
{
	None            UMETA(DisplayName = "无共鸣"),        // 0-1 个同元素技能
	Level1          UMETA(DisplayName = "共鸣1级"),       // 2 个同元素技能
	Level2          UMETA(DisplayName = "共鸣2级"),       // 3 个同元素技能
	Level3          UMETA(DisplayName = "共鸣3级"),       // 4 个同元素技能
	Level4          UMETA(DisplayName = "共鸣4级"),       // 5 个同元素技能

	MAX             UMETA(Hidden)
};

/**
 * EDBAChainTier
 * 连锁等级
 * 用于连锁系统伤害加成判定
 */
UENUM(BlueprintType)
enum class EDBAChainTier : uint8
{
	None            UMETA(DisplayName = "无连锁"),        // ChainLevel 0-5
	Tier1           UMETA(DisplayName = "连锁1阶"),       // ChainLevel 6-9，伤害 +20%
	Tier2           UMETA(DisplayName = "连锁2阶"),       // ChainLevel 10，伤害 +35%，触发终结

	MAX             UMETA(Hidden)
};

/**
 * EDBAMatchMode
 * 对局模式
 * 用于匹配、地图选择、规则配置
 */
UENUM(BlueprintType)
enum class EDBAMatchMode : uint8
{
	None            UMETA(DisplayName = "无"),

	Arena_5v5       UMETA(DisplayName = "5v5 对局"),
	Arena_3v3       UMETA(DisplayName = "3v3 对局"),
	Arena_1v1       UMETA(DisplayName = "1v1 对局"),
	Practice        UMETA(DisplayName = "练习模式"),
	Custom          UMETA(DisplayName = "自定义"),

	MAX             UMETA(Hidden)
};

/**
 * EDBAMatchResult
 * 对局结果
 * 用于结算、统计
 */
UENUM(BlueprintType)
enum class EDBAMatchResult : uint8
{
	None            UMETA(DisplayName = "无"),

	Victory         UMETA(DisplayName = "胜利"),
	Defeat          UMETA(DisplayName = "失败"),
	Draw            UMETA(DisplayName = "平局"),
	Surrender       UMETA(DisplayName = "投降"),
	Disconnect      UMETA(DisplayName = "断线"),

	MAX             UMETA(Hidden)
};

/**
 * EDBAAbilityActivationPolicy
 * 技能激活策略
 * 用于 GAS Ability 激活时机控制
 */
UENUM(BlueprintType)
enum class EDBAAbilityActivationPolicy : uint8
{
	OnInputTriggered    UMETA(DisplayName = "输入触发时"),     // 按下输入时激活
	WhileInputActive    UMETA(DisplayName = "输入持续时"),     // 输入持续期间激活
	OnSpawn             UMETA(DisplayName = "生成时"),         // Actor 生成时激活（被动技能）

	MAX                 UMETA(Hidden)
};

/**
 * EDBAAbilityInputID
 * 技能输入 ID
 * 用于 GAS Ability 与输入绑定
 * 与 EDBASkillInputSlot 对应
 */
UENUM(BlueprintType)
enum class EDBAAbilityInputID : uint8
{
	None            UMETA(DisplayName = "无"),
	Confirm         UMETA(DisplayName = "确认"),
	Cancel          UMETA(DisplayName = "取消"),

	BasicAttack     UMETA(DisplayName = "普通攻击"),
	Skill01         UMETA(DisplayName = "技能1"),
	Skill02         UMETA(DisplayName = "技能2"),
	Skill03         UMETA(DisplayName = "技能3"),
	Skill04         UMETA(DisplayName = "技能4"),
	Ultimate        UMETA(DisplayName = "生肖大招"),

	MAX             UMETA(Hidden)
};