# Copyright Freebooz Games, Inc. All Rights Reserved.
# -*- coding: utf-8 -*-
"""
十二生肖技能数据表生成脚本
用于 UE5 编辑器内运行 (py.exe 或在 Editor Python Console 中执行)

使用方法:
1. 在 UE5 编辑器中打开项目
2. 打开 Python Console (Window -> Developer Tools -> Python Console)
3. 执行: exec(open('Scripts/GenerateSkillDataTable.py', encoding='utf-8').read())
"""

import os

# 注：unreal 模块仅在 UE5 编辑器内可用
# 独立运行时会自动跳过需要 unreal 的函数

# ========================================
# 技能数据结构定义
# ========================================

class SkillData:
    def __init__(self, skill_id, name, description, cooldown, energy_cost, cast_range, effect_type):
        self.skill_id = skill_id
        self.name = name
        self.description = description
        self.cooldown = cooldown
        self.energy_cost = energy_cost
        self.cast_range = cast_range
        self.effect_type = effect_type

class HeroSkillSet:
    def __init__(self, zodiac_type, character_name, short_name, skills):
        self.zodiac_type = zodiac_type
        self.character_name = character_name
        self.short_name = short_name
        self.skills = skills  # [passive, skill01, skill02, skill03, skill04, ultimate]

# ========================================
# 十二生肖技能数据定义
# ========================================

HERO_SKILL_DATA = [
    # 子鼠·夜影灵牙｜影牙
    HeroSkillSet(
        zodiac_type="Rat",
        character_name="子鼠·夜影灵牙",
        short_name="影牙",
        skills=[
            SkillData("Rat_Passive", "灵鼠印", "命中时叠加印记，叠满后触发额外效果", 0, 0, 0, "Passive"),
            SkillData("Rat_Skill01", "钻影", "向目标方向钻入阴影，快速位移", 8, 30, 500, "Active"),
            SkillData("Rat_Skill02", "飞牙", "发射多枚飞牙，追踪目标", 6, 25, 600, "Active"),
            SkillData("Rat_Skill03", "鼠遁", "钻地脱身，短暂无敌并生成假穴", 12, 40, 0, "Active"),
            SkillData("Rat_Skill04", "探穴", "灵鼠探路，暴露草丛视野", 10, 35, 800, "Active"),
            SkillData("Rat_Ultimate", "子夜现身", "瞬移至目标背后，背后伤害加成", 60, 100, 300, "Ultimate"),
        ]
    ),
    # 丑牛·撼山铁角｜铁角
    HeroSkillSet(
        zodiac_type="Ox",
        character_name="丑牛·撼山铁角",
        short_name="铁角",
        skills=[
            SkillData("Ox_Passive", "牛劲", "血量越低，双抗越高", 0, 0, 0, "Passive"),
            SkillData("Ox_Skill01", "角挑", "牛角上挑，击飞目标", 10, 35, 200, "Active"),
            SkillData("Ox_Skill02", "铁蹄震", "践踏地面，造成范围伤害和减速", 14, 50, 350, "Active"),
            SkillData("Ox_Skill03", "巨盾阵", "举起巨盾，为周围队友提供护盾", 16, 60, 0, "Active"),
            SkillData("Ox_Skill04", "回身顶", "快速转身，反向顶撞", 8, 30, 250, "Active"),
            SkillData("Ox_Ultimate", "蛮牛开山", "冲锋开团，击退路径上所有敌人", 50, 100, 600, "Ultimate"),
        ]
    ),
    # 寅虎·啸山白虎｜白虎
    HeroSkillSet(
        zodiac_type="Tiger",
        character_name="寅虎·啸山白虎",
        short_name="白虎",
        skills=[
            SkillData("Tiger_Passive", "虎威", "攻击附带真伤，低血量时伤害提升", 0, 0, 0, "Passive"),
            SkillData("Tiger_Skill01", "虎跃", "向目标跃击，落地产生范围伤害", 6, 25, 400, "Active"),
            SkillData("Tiger_Skill02", "三裂爪", "三段伤害，递进式增强", 8, 40, 350, "Active"),
            SkillData("Tiger_Skill03", "虎啸提气", "提气减伤，低血量时反扑", 14, 50, 0, "Active"),
            SkillData("Tiger_Skill04", "追风爪", "远程爪击，带追踪效果", 10, 35, 550, "Active"),
            SkillData("Tiger_Ultimate", "白虎点将", "锁定目标高速突进，单点爆发", 45, 100, 700, "Ultimate"),
        ]
    ),
    # 卯兔·踏月玉灵｜玉灵
    HeroSkillSet(
        zodiac_type="Rabbit",
        character_name="卯兔·踏月玉灵",
        short_name="玉灵",
        skills=[
            SkillData("Rabbit_Passive", "轻月", "移速加成，叠满月层获得月盾", 0, 0, 0, "Passive"),
            SkillData("Rabbit_Skill01", "踏月返", "位移技能，可返回原位置", 7, 30, 500, "Active"),
            SkillData("Rabbit_Skill02", "月牙轮", "月牙飞刃，来回两段伤害", 9, 35, 450, "Active"),
            SkillData("Rabbit_Skill03", "月闪", "月影闪避，躲避关键技能", 11, 40, 0, "Active"),
            SkillData("Rabbit_Skill04", "留月影", "留下假身迷惑敌人", 13, 45, 0, "Active"),
            SkillData("Rabbit_Ultimate", "玉兔拜月", "圆月落下，范围月轮切割", 55, 100, 500, "Ultimate"),
        ]
    ),
    # 辰龙·御雷苍龙｜苍龙
    HeroSkillSet(
        zodiac_type="Dragon",
        character_name="辰龙·御雷苍龙",
        short_name="苍龙",
        skills=[
            SkillData("Dragon_Passive", "龙雷印", "技能命中叠加印记，满层落雷", 0, 0, 0, "Passive"),
            SkillData("Dragon_Skill01", "雷龙", "雷电化龙，直线穿透", 10, 45, 550, "Active"),
            SkillData("Dragon_Skill02", "云雷阵", "区域雷云，持续落雷", 16, 70, 400, "Active"),
            SkillData("Dragon_Skill03", "龙鳞护", "抵挡一次伤害或控制", 12, 50, 0, "Active"),
            SkillData("Dragon_Skill04", "雷门", "生成雷门，穿过获得加速", 8, 35, 300, "Active"),
            SkillData("Dragon_Ultimate", "苍龙唤雷", "大范围雷云汇聚，主雷爆发", 60, 100, 700, "Ultimate"),
        ]
    ),
    # 巳蛇·幽毒灵蛇｜幽鳞
    HeroSkillSet(
        zodiac_type="Snake",
        character_name="巳蛇·幽毒灵蛇",
        short_name="幽鳞",
        skills=[
            SkillData("Snake_Passive", "蛇纹", "技能命中叠加蛇纹，减速敌人", 0, 0, 0, "Passive"),
            SkillData("Snake_Skill01", "蛇探", "小蛇灵光探路，暴露视野", 7, 30, 500, "Active"),
            SkillData("Snake_Skill02", "蛇环", "蛇影环绕，区域控制", 12, 55, 350, "Active"),
            SkillData("Snake_Skill03", "蜕影步", "留影滑步，柔性脱身", 10, 40, 0, "Active"),
            SkillData("Snake_Skill04", "花步", "花瓣光步，快速位移", 6, 25, 400, "Active"),
            SkillData("Snake_Ultimate", "百花蛇舞", "花瓣与灵蛇共舞，大范围控场", 50, 100, 500, "Ultimate"),
        ]
    ),
    # 午马·赤焰雷蹄｜雷蹄
    HeroSkillSet(
        zodiac_type="Horse",
        character_name="午马·赤焰雷蹄",
        short_name="雷蹄",
        skills=[
            SkillData("Horse_Passive", "奔势", "跑动蓄力，速度越快伤害越高", 0, 0, 0, "Passive"),
            SkillData("Horse_Skill01", "雷蹄冲", "火雷冲锋，快速接近", 8, 35, 500, "Active"),
            SkillData("Horse_Skill02", "赤焰旋", "长枪旋转，范围伤害", 10, 45, 300, "Active"),
            SkillData("Horse_Skill03", "驰援", "冲向队友，双方获得护盾", 14, 55, 600, "Active"),
            SkillData("Horse_Skill04", "踏火印", "火雷蹄印铺路", 6, 25, 400, "Active"),
            SkillData("Horse_Ultimate", "奔雷入阵", "远程开团，大范围冲击", 55, 100, 800, "Ultimate"),
        ]
    ),
    # 未羊·玉角灵铃｜玉角
    HeroSkillSet(
        zodiac_type="Goat",
        character_name="未羊·玉角灵铃",
        short_name="玉角",
        skills=[
            SkillData("Goat_Passive", "铃愿", "附近队友获得回复增益", 0, 0, 0, "Passive"),
            SkillData("Goat_Skill01", "回春铃", "大范围治疗", 12, 50, 450, "Active"),
            SkillData("Goat_Skill02", "暖玉盾", "为目标提供护盾", 14, 60, 400, "Active"),
            SkillData("Goat_Skill03", "清铃音", "净化队友，解除控制", 10, 40, 350, "Active"),
            SkillData("Goat_Skill04", "愿光环", "地面祝福圈，站入获得增益", 8, 35, 300, "Active"),
            SkillData("Goat_Ultimate", "灵铃赐福", "团队保护，紧急护盾", 60, 100, 500, "Ultimate"),
        ]
    ),
    # 申猴·百戏灵猴｜灵猴
    HeroSkillSet(
        zodiac_type="Monkey",
        character_name="申猴·百戏灵猴",
        short_name="灵猴",
        skills=[
            SkillData("Monkey_Passive", "猴戏", "连招成功获得攻速加成", 0, 0, 0, "Passive"),
            SkillData("Monkey_Skill01", "翻跃", "灵活翻跃，穿墙能力", 6, 25, 450, "Active"),
            SkillData("Monkey_Skill02", "猴影", "生成假身，本体换位", 11, 45, 0, "Active"),
            SkillData("Monkey_Skill03", "云跳", "连续跳跃，快速追击", 5, 20, 350, "Active"),
            SkillData("Monkey_Skill04", "摘星手", "远程拉扯，拉回目标", 9, 38, 500, "Active"),
            SkillData("Monkey_Ultimate", "百猴闹场", "多个猴影扰乱，群体控制", 50, 100, 600, "Ultimate"),
        ]
    ),
    # 酉鸡·破晓金翎｜金翎
    HeroSkillSet(
        zodiac_type="Rooster",
        character_name="酉鸡·破晓金翎",
        short_name="金翎",
        skills=[
            SkillData("Rooster_Passive", "晨鸣", "附近有隐藏敌人时预警", 0, 0, 0, "Passive"),
            SkillData("Rooster_Skill01", "金鸡鸣", "扇形声波，暴露隐身目标", 10, 40, 450, "Active"),
            SkillData("Rooster_Skill02", "金羽标", "标记目标，持续暴露位置", 8, 30, 550, "Active"),
            SkillData("Rooster_Skill03", "明照", "照亮区域，探测草丛", 13, 50, 400, "Active"),
            SkillData("Rooster_Skill04", "晨羽阵", "羽阵形成，区域内可见隐身", 16, 65, 350, "Active"),
            SkillData("Rooster_Ultimate", "破晓照天", "全图照亮，解除隐身", 55, 100, 700, "Ultimate"),
        ]
    ),
    # 戌狗·守门天犬｜天犬
    HeroSkillSet(
        zodiac_type="Dog",
        character_name="戌狗·守门天犬",
        short_name="天犬",
        skills=[
            SkillData("Dog_Passive", "犬护", "保护附近队友，减伤", 0, 0, 0, "Passive"),
            SkillData("Dog_Skill01", "扑援", "扑向队友，提供护盾", 10, 40, 500, "Active"),
            SkillData("Dog_Skill02", "犬盾拍", "盾击前方，眩晕控制", 8, 35, 250, "Active"),
            SkillData("Dog_Skill03", "灵鼻踪", "追踪目标，显示足迹", 12, 45, 600, "Active"),
            SkillData("Dog_Skill04", "护心圈", "队友脚下护圈，抵挡一次", 14, 55, 400, "Active"),
            SkillData("Dog_Ultimate", "天犬守门", "神门结界，区域保护", 50, 100, 500, "Ultimate"),
        ]
    ),
    # 亥猪·岩甲獠牙｜獠牙
    HeroSkillSet(
        zodiac_type="Pig",
        character_name="亥猪·岩甲獠牙",
        short_name="獠牙",
        skills=[
            SkillData("Pig_Passive", "厚甲", "血量越低，双抗越高", 0, 0, 0, "Passive"),
            SkillData("Pig_Skill01", "獠拱", "獠牙前拱，击退目标", 9, 35, 300, "Active"),
            SkillData("Pig_Skill02", "岩甲蓄", "吸收伤害，结束时反击", 14, 60, 0, "Active"),
            SkillData("Pig_Skill03", "锤震", "大锤砸地，范围伤害减速", 11, 45, 350, "Active"),
            SkillData("Pig_Skill04", "福印", "地面福印，队友站入减伤", 8, 30, 400, "Active"),
            SkillData("Pig_Ultimate", "福山不动", "站场强化，周身岩甲发亮", 55, 100, 450, "Ultimate"),
        ]
    ),
]

# ========================================
# 数据表列定义
# ========================================

SKILL_DATA_TABLE_COLUMNS = [
    "SkillID",
    "SkillName",
    "Description",
    "Cooldown",
    "EnergyCost",
    "CastRange",
    "EffectType",
    "ZodiacType",
    "CharacterName",
]

# ========================================
# 生成函数
# ========================================

def create_skill_data_table():
    """创建技能数据表"""

    # 数据表路径
    output_path = "/Game/DBA/Data/Tables/DT_Skills"

    # 创建数据表资产
    table = unreal.DataTableFactory.create_empty(unreal.Struct)
    table.set_editor_property("row_struct", None)  # 稍后设置具体结构

    print(f"技能数据表创建完成，请手动配置结构体")
    print(f"建议路径: {output_path}")
    print(f"\n请在 UE5 编辑器中执行以下步骤:")
    print("1. 创建 Struct: FDBSkillTableRow")
    print("2. 包含字段:", SKILL_DATA_TABLE_COLUMNS)

    return table


def generate_skill_csv():
    """生成技能数据 CSV 文件 (可导入 UE5)"""

    output_file = "Scripts/DT_Skills_V15.csv"

    with open(output_file, 'w', encoding='utf-8') as f:
        # 写入表头
        f.write(','.join(SKILL_DATA_TABLE_COLUMNS) + '\n')

        # 写入每个英雄的每个技能
        for hero in HERO_SKILL_DATA:
            for skill in hero.skills:
                row = [
                    skill.skill_id,
                    skill.name,
                    skill.description.replace(',', ';'),
                    str(skill.cooldown),
                    str(skill.energy_cost),
                    str(skill.cast_range),
                    skill.effect_type,
                    hero.zodiac_type,
                    hero.character_name,
                ]
                f.write(','.join(row) + '\n')

    print(f"CSV 文件已生成: {output_file}")
    print("请在 UE5 中使用 File -> Import CSV 导入")


def generate_skill_data_code():
    """生成技能数据结构体代码"""

    output_file = "Scripts/FDBSkillTableRow.h"

    code = '''// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能数据表结构体 (由 Python 脚本自动生成)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FDBSkillTableRow.generated.h"

/**
 * FDBSkillTableRow
 * 技能数据表行结构
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBSkillTableRow : public FTableRowBase
{
    GENERATED_BODY()

public:
'''

    # 添加每个列的字段
    for col in SKILL_DATA_TABLE_COLUMNS:
        prop_type = "FString"
        if col == "Cooldown" or col == "EnergyCost" or col == "CastRange":
            prop_type = "float"
        code += f'\n\tUPROPERTY(EditDefaultsOnly, BlueprintReadOnly)\n\t{prop_type} {col};\n'

    code += '''};

// 在 C++ 中使用:
// #include "GameDBA/GAS/DataTables/FDBSkillTableRow.h"
// UDataTable* SkillTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/DBA/Data/Tables/DT_Skills"));
// FDBSkillTableRow* Row = SkillTable->FindRow<FDBSkillTableRow>(FName("Rat_Passive"), TEXT(""));
'''

    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(code)

    print(f"结构体头文件已生成: {output_file}")


# ========================================
# 主执行
# ========================================

if __name__ == "__main__":
    print("=" * 60)
    print("十二生肖技能数据表生成脚本")
    print("=" * 60)

    # 生成 CSV
    generate_skill_csv()

    # 生成代码
    generate_skill_data_code()

    print("\n完成！请查看 Scripts/ 目录下的输出文件")