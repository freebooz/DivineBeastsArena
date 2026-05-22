# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

# Copyright Freebooz Games, Inc. All Rights Reserved.
# -*- coding: utf-8 -*-
"""
十二生肖数值平衡配置生成脚本
用于 UE5 编辑器内运行 (py.exe 或在 Editor Python Console 中执行)

使用方法:
1. 在 UE5 编辑器中打开项目
2. 打开 Python Console (Window -> Developer Tools -> Python Console)
3. 执行: exec(open('Scripts/GenerateHeroBalance.py', encoding='utf-8').read())
"""

import os
import json

# 注：unreal 模块仅在 UE5 编辑器内可用
# 独立运行时会自动跳过需要 unreal 的函数

# ========================================
# 数值平衡数据结构定义
# ========================================

class HeroBalanceData:
    def __init__(self, zodiac_type, character_name, short_name, core_role,
                 survivability, damage, control, mobility, support, difficulty, team_fight_impact,
                 recommended_lane, team_role, advantages, weaknesses, best_partners):
        self.zodiac_type = zodiac_type
        self.character_name = character_name
        self.short_name = short_name
        self.core_role = core_role
        self.survivability = survivability
        self.damage = damage
        self.control = control
        self.mobility = mobility
        self.support = support
        self.difficulty = difficulty
        self.team_fight_impact = team_fight_impact
        self.recommended_lane = recommended_lane
        self.team_role = team_role
        self.advantages = advantages
        self.weaknesses = weaknesses
        self.best_partners = best_partners

# ========================================
# 十二生肖数值平衡数据
# ========================================

HERO_BALANCE_DATA = [
    # 子鼠·夜影灵牙｜影牙
    HeroBalanceData(
        zodiac_type="Rat",
        character_name="子鼠·夜影灵牙",
        short_name="影牙",
        core_role="潜行刺客 / 侦察收割",
        survivability=2, damage=5, control=2, mobility=5, support=2, difficulty=5, team_fight_impact=3,
        recommended_lane="打野 / 游走",
        team_role="探穴侦察、绕后切入、残血收尾",
        advantages="爆发高、侦察灵活、收尾强",
        weaknesses="身板脆、依赖时机",
        best_partners="金翎、天犬、铁角"
    ),
    # 丑牛·撼山铁角｜铁角
    HeroBalanceData(
        zodiac_type="Ox",
        character_name="丑牛·撼山铁角",
        short_name="铁角",
        core_role="重装坦克 / 开团先锋",
        survivability=5, damage=2, control=5, mobility=2, support=4, difficulty=3, team_fight_impact=5,
        recommended_lane="上路 / 辅助前排",
        team_role="正面开团、举盾护队、反身保护",
        advantages="开团强、承伤高、保护稳",
        weaknesses="机动低、输出低",
        best_partners="苍龙、玉灵、玉角"
    ),
    # 寅虎·啸山白虎｜白虎
    HeroBalanceData(
        zodiac_type="Tiger",
        character_name="寅虎·啸山白虎",
        short_name="白虎",
        core_role="爆发战士 / 目标压制",
        survivability=3, damage=5, control=3, mobility=4, support=1, difficulty=4, team_fight_impact=4,
        recommended_lane="上路 / 打野",
        team_role="侧翼突进、单点压制、追击收割",
        advantages="单点爆发强、追击强",
        weaknesses="怕被集火控制",
        best_partners="金翎、雷蹄、玉角"
    ),
    # 卯兔·踏月玉灵｜玉灵
    HeroBalanceData(
        zodiac_type="Rabbit",
        character_name="卯兔·踏月玉灵",
        short_name="玉灵",
        core_role="机动输出 / 月影拉扯",
        survivability=2, damage=4, control=2, mobility=5, support=2, difficulty=5, team_fight_impact=3,
        recommended_lane="中路 / 游走",
        team_role="位移拉扯、月影迷惑、持续消耗",
        advantages="灵活、拉扯强、操作上限高",
        weaknesses="容错低、怕硬控",
        best_partners="天犬、玉角、铁角"
    ),
    # 辰龙·御雷苍龙｜苍龙
    HeroBalanceData(
        zodiac_type="Dragon",
        character_name="辰龙·御雷苍龙",
        short_name="苍龙",
        core_role="法师核心 / 雷云控场",
        survivability=3, damage=5, control=4, mobility=2, support=3, difficulty=4, team_fight_impact=5,
        recommended_lane="中路",
        team_role="雷云控场、团战法核、雷门辅助",
        advantages="团战输出强、控场强",
        weaknesses="依赖站位和预判",
        best_partners="铁角、幽鳞、玉角"
    ),
    # 巳蛇·幽毒灵蛇｜幽鳞
    HeroBalanceData(
        zodiac_type="Snake",
        character_name="巳蛇·幽毒灵蛇",
        short_name="幽鳞",
        core_role="灵动控场 / 区域节奏",
        survivability=3, damage=3, control=5, mobility=4, support=2, difficulty=4, team_fight_impact=5,
        recommended_lane="中路 / 辅助控制",
        team_role="区域控场、蛇纹减速、优雅脱身",
        advantages="区域控制强、节奏压制强",
        weaknesses="爆发一般",
        best_partners="苍龙、铁角、金翎"
    ),
    # 午马·赤焰雷蹄｜雷蹄
    HeroBalanceData(
        zodiac_type="Horse",
        character_name="午马·赤焰雷蹄",
        short_name="雷蹄",
        core_role="高机动先锋 / 跑图支援",
        survivability=3, damage=4, control=3, mobility=5, support=4, difficulty=3, team_fight_impact=4,
        recommended_lane="打野 / 上路",
        team_role="快速支援、路径铺设、远程开团",
        advantages="支援快、开团好、节奏强",
        weaknesses="持续站场一般",
        best_partners="白虎、玉角、天犬"
    ),
    # 未羊·玉角灵铃｜玉角
    HeroBalanceData(
        zodiac_type="Goat",
        character_name="未羊·玉角灵铃",
        short_name="玉角",
        core_role="治疗辅助 / 团队保护",
        survivability=3, damage=1, control=2, mobility=3, support=5, difficulty=3, team_fight_impact=5,
        recommended_lane="辅助",
        team_role="治疗、护盾、净化、团队祝福",
        advantages="团队续航强、保护强",
        weaknesses="输出低、依赖队友",
        best_partners="铁角、獠牙、白虎"
    ),
    # 申猴·百戏灵猴｜灵猴
    HeroBalanceData(
        zodiac_type="Monkey",
        character_name="申猴·百戏灵猴",
        short_name="灵猴",
        core_role="高机动扰乱 / 假身换位",
        survivability=2, damage=4, control=3, mobility=5, support=1, difficulty=5, team_fight_impact=4,
        recommended_lane="打野 / 游走",
        team_role="假身换位、连跳扰乱、后排干扰",
        advantages="操作秀、扰乱强、机动高",
        weaknesses="容错低、怕稳定控制",
        best_partners="金翎、天犬、玉角"
    ),
    # 酉鸡·破晓金翎｜金翎
    HeroBalanceData(
        zodiac_type="Rooster",
        character_name="酉鸡·破晓金翎",
        short_name="金翎",
        core_role="侦测辅助 / 视野控制",
        survivability=3, damage=2, control=3, mobility=3, support=5, difficulty=3, team_fight_impact=4,
        recommended_lane="辅助",
        team_role="视野预警、显形照场、反埋伏",
        advantages="反隐强、视野强、团队价值高",
        weaknesses="正面伤害不足",
        best_partners="影牙、白虎、灵猴"
    ),
    # 戌狗·守门天犬｜天犬
    HeroBalanceData(
        zodiac_type="Dog",
        character_name="戌狗·守门天犬",
        short_name="天犬",
        core_role="守护辅助 / 反突进",
        survivability=4, damage=2, control=4, mobility=3, support=5, difficulty=3, team_fight_impact=5,
        recommended_lane="辅助 / 上路",
        team_role="护主救援、反突进、守门结界",
        advantages="保护强、反突进强",
        weaknesses="开团不如铁角",
        best_partners="玉灵、苍龙、金翎"
    ),
    # 亥猪·岩甲獠牙｜獠牙
    HeroBalanceData(
        zodiac_type="Pig",
        character_name="亥猪·岩甲獠牙",
        short_name="獠牙",
        core_role="站场坦克 / 稳定承伤",
        survivability=5, damage=3, control=4, mobility=2, support=3, difficulty=2, team_fight_impact=5,
        recommended_lane="上路 / 前排辅助",
        team_role="稳定站场、岩甲承伤、福印稳阵",
        advantages="站场强、耐打、团战稳定",
        weaknesses="机动低、手短",
        best_partners="玉角、苍龙、金翎"
    ),
]

# ========================================
# 数据表列定义
# ========================================

HERO_BALANCE_COLUMNS = [
    "ZodiacType",
    "CharacterName",
    "ShortName",
    "CoreRole",
    "Survivability",
    "Damage",
    "Control",
    "Mobility",
    "Support",
    "Difficulty",
    "TeamFightImpact",
    "RecommendedLane",
    "TeamRole",
    "Advantages",
    "Weaknesses",
    "BestPartners",
]

# ========================================
# 生成函数
# ========================================

def generate_hero_balance_csv():
    """生成数值平衡 CSV 文件 (可导入 UE5)"""

    output_file = "Scripts/DT_HeroBalance_V15.csv"

    with open(output_file, 'w', encoding='utf-8') as f:
        # 写入表头
        f.write(','.join(HERO_BALANCE_COLUMNS) + '\n')

        # 写入每个英雄的数据
        for hero in HERO_BALANCE_DATA:
            row = [
                hero.zodiac_type,
                hero.character_name,
                hero.short_name,
                hero.core_role,
                str(hero.survivability),
                str(hero.damage),
                str(hero.control),
                str(hero.mobility),
                str(hero.support),
                str(hero.difficulty),
                str(hero.team_fight_impact),
                hero.recommended_lane,
                hero.team_role,
                hero.advantages,
                hero.weaknesses,
                hero.best_partners,
            ]
            f.write(','.join(row) + '\n')

    print(f"CSV 文件已生成: {output_file}")
    print("请在 UE5 中使用 File -> Import CSV 导入")


def generate_hero_balance_struct_code():
    """生成数值平衡结构体代码"""

    output_file = "Scripts/FDHHeroBalanceTableRow.h"

    code = '''// Copyright Freebooz Games, Inc. All Rights Reserved.
// 英雄数值平衡数据表结构体 (由 Python 脚本自动生成)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FDHHeroBalanceTableRow.generated.h"

/**
 * FDHHeroBalanceTableRow
 * 英雄数值平衡数据表行结构
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDHHeroBalanceTableRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    // 基本信息
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString ZodiacType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString CharacterName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString ShortName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString CoreRole;

    // 能力评分 (1-5)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 Survivability;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 Damage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 Control;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 Mobility;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 Support;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 Difficulty;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 TeamFightImpact;

    // 分路信息
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString RecommendedLane;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString TeamRole;

    // 优劣势
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString Advantages;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString Weaknesses;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString BestPartners;
};
'''

    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(code)

    print(f"结构体头文件已生成: {output_file}")


def generate_json_config():
    """生成 JSON 格式配置文件 (用于外部工具或配置系统)"""

    import json

    output_file = "Scripts/HeroBalanceConfig.json"

    config = {
        "version": "V15",
        "last_updated": "2026-05-08",
        "heroes": []
    }

    for hero in HERO_BALANCE_DATA:
        hero_data = {
            "zodiac_type": hero.zodiac_type,
            "character_name": hero.character_name,
            "short_name": hero.short_name,
            "core_role": hero.core_role,
            "core_stats": {
                "survivability": hero.survivability,
                "damage": hero.damage,
                "control": hero.control,
                "mobility": hero.mobility,
                "support": hero.support,
                "difficulty": hero.difficulty,
                "team_fight_impact": hero.team_fight_impact,
            },
            "position_info": {
                "recommended_lane": hero.recommended_lane,
                "team_role": hero.team_role,
            },
            "strengths_weaknesses": {
                "advantages": hero.advantages,
                "weaknesses": hero.weaknesses,
                "best_partners": hero.best_partners,
            }
        }
        config["heroes"].append(hero_data)

    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(config, f, ensure_ascii=False, indent=2)

    print(f"JSON 配置文件已生成: {output_file}")


# ========================================
# 主执行
# ========================================

if __name__ == "__main__":
    print("=" * 60)
    print("十二生肖数值平衡配置生成脚本")
    print("=" * 60)

    # 生成 CSV
    generate_hero_balance_csv()

    # 生成结构体代码
    generate_hero_balance_struct_code()

    # 生成 JSON
    generate_json_config()

    print("\n完成！请查看 Scripts/ 目录下的输出文件")