# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

# Copyright Freebooz Games, Inc. All Rights Reserved.
# -*- coding: utf-8 -*-
"""
技能数据表生成脚本
生成与 FDBASkillDataRow 对齐的 CSV
"""

import os

class SkillData:
    def __init__(self, row_name, skill_id, display_name, description,
                 cooldown, energy_cost, cast_range, skill_type, skill_slot,
                 zodiac_type, element_type):
        self.row_name = row_name
        self.skill_id = skill_id
        self.display_name = display_name
        self.description = description
        self.cooldown = cooldown
        self.energy_cost = energy_cost
        self.cast_range = cast_range
        self.skill_type = skill_type          # Passive/Active/Ultimate
        self.skill_slot = skill_slot          # None/Passive/Active1-4/Ultimate
        self.zodiac_type = zodiac_type        # Rat/Ox/Tiger/etc.
        self.element_type = element_type      # None/Fire/Water/Wood/Gold/Earth

HERO_SKILL_DATA = [
    # 子鼠·夜影灵牙
    SkillData("Rat_Passive", "Rat_Passive", "灵鼠印", "命中时叠加印记，叠满后触发额外效果", 0, 0, 0, "Passive", "Passive", "Rat", "None"),
    SkillData("Rat_Skill01", "Rat_Skill01", "钻影", "向目标方向钻入阴影，快速位移", 8, 30, 500, "Active", "Active1", "Rat", "None"),
    SkillData("Rat_Skill02", "Rat_Skill02", "飞牙", "发射多枚飞牙，追踪目标", 6, 25, 600, "Active", "Active2", "Rat", "None"),
    SkillData("Rat_Skill03", "Rat_Skill03", "鼠遁", "钻地脱身，短暂无敌并生成假穴", 12, 40, 0, "Active", "Active3", "Rat", "None"),
    SkillData("Rat_Skill04", "Rat_Skill04", "探穴", "灵鼠探路，暴露草丛视野", 10, 35, 800, "Active", "Active4", "Rat", "None"),
    SkillData("Rat_Ultimate", "Rat_Ultimate", "子夜现身", "瞬移至目标背后，背后伤害加成", 60, 100, 300, "Ultimate", "Ultimate", "Rat", "None"),

    # 丑牛·撼山铁角
    SkillData("Ox_Passive", "Ox_Passive", "牛劲", "血量越低，双抗越高", 0, 0, 0, "Passive", "Passive", "Ox", "None"),
    SkillData("Ox_Skill01", "Ox_Skill01", "角挑", "牛角上挑，击飞目标", 10, 35, 200, "Active", "Active1", "Ox", "None"),
    SkillData("Ox_Skill02", "Ox_Skill02", "铁蹄震", "践踏地面，造成范围伤害和减速", 14, 50, 350, "Active", "Active2", "Ox", "None"),
    SkillData("Ox_Skill03", "Ox_Skill03", "巨盾阵", "举起巨盾，为周围队友提供护盾", 16, 60, 0, "Active", "Active3", "Ox", "None"),
    SkillData("Ox_Skill04", "Ox_Skill04", "回身顶", "快速转身，反向顶撞", 8, 30, 250, "Active", "Active4", "Ox", "None"),
    SkillData("Ox_Ultimate", "Ox_Ultimate", "蛮牛开山", "冲锋开团，击退路径上所有敌人", 50, 100, 600, "Ultimate", "Ultimate", "Ox", "None"),

    # 寅虎·啸山白虎
    SkillData("Tiger_Passive", "Tiger_Passive", "虎威", "攻击附带真伤，低血量时伤害提升", 0, 0, 0, "Passive", "Passive", "Tiger", "None"),
    SkillData("Tiger_Skill01", "Tiger_Skill01", "虎跃", "向目标跃击，落地产生范围伤害", 6, 25, 400, "Active", "Active1", "Tiger", "None"),
    SkillData("Tiger_Skill02", "Tiger_Skill02", "三裂爪", "三段伤害，递进式增强", 8, 40, 350, "Active", "Active2", "Tiger", "None"),
    SkillData("Tiger_Skill03", "Tiger_Skill03", "虎啸提气", "提气减伤，低血量时反扑", 14, 50, 0, "Active", "Active3", "Tiger", "None"),
    SkillData("Tiger_Skill04", "Tiger_Skill04", "追风爪", "远程爪击，带追踪效果", 10, 35, 550, "Active", "Active4", "Tiger", "None"),
    SkillData("Tiger_Ultimate", "Tiger_Ultimate", "白虎点将", "锁定目标高速突进，单点爆发", 45, 100, 700, "Ultimate", "Ultimate", "Tiger", "None"),

    # 卯兔·踏月玉灵
    SkillData("Rabbit_Passive", "Rabbit_Passive", "轻月", "移速加成，叠满月层获得月盾", 0, 0, 0, "Passive", "Passive", "Rabbit", "None"),
    SkillData("Rabbit_Skill01", "Rabbit_Skill01", "踏月返", "位移技能，可返回原位置", 7, 30, 500, "Active", "Active1", "Rabbit", "None"),
    SkillData("Rabbit_Skill02", "Rabbit_Skill02", "月牙轮", "月牙飞刃，来回两段伤害", 9, 35, 450, "Active", "Active2", "Rabbit", "None"),
    SkillData("Rabbit_Skill03", "Rabbit_Skill03", "月闪", "月影闪避，躲避关键技能", 11, 40, 0, "Active", "Active3", "Rabbit", "None"),
    SkillData("Rabbit_Skill04", "Rabbit_Skill04", "留月影", "留下假身迷惑敌人", 13, 45, 0, "Active", "Active4", "Rabbit", "None"),
    SkillData("Rabbit_Ultimate", "Rabbit_Ultimate", "玉兔拜月", "圆月落下，范围月轮切割", 55, 100, 500, "Ultimate", "Ultimate", "Rabbit", "None"),

    # 辰龙·御雷苍龙
    SkillData("Dragon_Passive", "Dragon_Passive", "龙雷印", "技能命中叠加印记，满层落雷", 0, 0, 0, "Passive", "Passive", "Dragon", "None"),
    SkillData("Dragon_Skill01", "Dragon_Skill01", "雷龙", "雷电化龙，直线穿透", 10, 45, 550, "Active", "Active1", "Dragon", "None"),
    SkillData("Dragon_Skill02", "Dragon_Skill02", "云雷阵", "区域雷云，持续落雷", 16, 70, 400, "Active", "Active2", "Dragon", "None"),
    SkillData("Dragon_Skill03", "Dragon_Skill03", "龙鳞护", "抵挡一次伤害或控制", 12, 50, 0, "Active", "Active3", "Dragon", "None"),
    SkillData("Dragon_Skill04", "Dragon_Skill04", "雷门", "生成雷门，穿过获得加速", 8, 35, 300, "Active", "Active4", "Dragon", "None"),
    SkillData("Dragon_Ultimate", "Dragon_Ultimate", "苍龙唤雷", "大范围雷云汇聚，主雷爆发", 60, 100, 700, "Ultimate", "Ultimate", "Dragon", "None"),

    # 巳蛇·幽毒灵蛇
    SkillData("Snake_Passive", "Snake_Passive", "蛇纹", "技能命中叠加蛇纹，减速敌人", 0, 0, 0, "Passive", "Passive", "Snake", "None"),
    SkillData("Snake_Skill01", "Snake_Skill01", "蛇探", "小蛇灵光探路，暴露视野", 7, 30, 500, "Active", "Active1", "Snake", "None"),
    SkillData("Snake_Skill02", "Snake_Skill02", "蛇环", "蛇影环绕，区域控制", 12, 55, 350, "Active", "Active2", "Snake", "None"),
    SkillData("Snake_Skill03", "Snake_Skill03", "蜕影步", "留影滑步，柔性脱身", 10, 40, 0, "Active", "Active3", "Snake", "None"),
    SkillData("Snake_Skill04", "Snake_Skill04", "花步", "花瓣光步，快速位移", 6, 25, 400, "Active", "Active4", "Snake", "None"),
    SkillData("Snake_Ultimate", "Snake_Ultimate", "百花蛇舞", "花瓣与灵蛇共舞，大范围控场", 50, 100, 500, "Ultimate", "Ultimate", "Snake", "None"),

    # 午马·赤焰雷蹄
    SkillData("Horse_Passive", "Horse_Passive", "奔势", "跑动蓄力，速度越快伤害越高", 0, 0, 0, "Passive", "Passive", "Horse", "None"),
    SkillData("Horse_Skill01", "Horse_Skill01", "雷蹄冲", "火雷冲锋，快速接近", 8, 35, 500, "Active", "Active1", "Horse", "None"),
    SkillData("Horse_Skill02", "Horse_Skill02", "赤焰旋", "长枪旋转，范围伤害", 10, 45, 300, "Active", "Active2", "Horse", "None"),
    SkillData("Horse_Skill03", "Horse_Skill03", "驰援", "冲向队友，双方获得护盾", 14, 55, 600, "Active", "Active3", "Horse", "None"),
    SkillData("Horse_Skill04", "Horse_Skill04", "踏火印", "火雷蹄印铺路", 6, 25, 400, "Active", "Active4", "Horse", "None"),
    SkillData("Horse_Ultimate", "Horse_Ultimate", "奔雷入阵", "远程开团，大范围冲击", 55, 100, 800, "Ultimate", "Ultimate", "Horse", "None"),

    # 未羊·玉角灵铃
    SkillData("Goat_Passive", "Goat_Passive", "铃愿", "附近队友获得回复增益", 0, 0, 0, "Passive", "Passive", "Goat", "None"),
    SkillData("Goat_Skill01", "Goat_Skill01", "回春铃", "大范围治疗", 12, 50, 450, "Active", "Active1", "Goat", "None"),
    SkillData("Goat_Skill02", "Goat_Skill02", "暖玉盾", "为目标提供护盾", 14, 60, 400, "Active", "Active2", "Goat", "None"),
    SkillData("Goat_Skill03", "Goat_Skill03", "清铃音", "净化队友，解除控制", 10, 40, 350, "Active", "Active3", "Goat", "None"),
    SkillData("Goat_Skill04", "Goat_Skill04", "愿光环", "地面祝福圈，站入获得增益", 8, 35, 300, "Active", "Active4", "Goat", "None"),
    SkillData("Goat_Ultimate", "Goat_Ultimate", "灵铃赐福", "团队保护，紧急护盾", 60, 100, 500, "Ultimate", "Ultimate", "Goat", "None"),

    # 申猴·百戏灵猴
    SkillData("Monkey_Passive", "Monkey_Passive", "猴戏", "连招成功获得攻速加成", 0, 0, 0, "Passive", "Passive", "Monkey", "None"),
    SkillData("Monkey_Skill01", "Monkey_Skill01", "翻跃", "灵活翻跃，穿墙能力", 6, 25, 450, "Active", "Active1", "Monkey", "None"),
    SkillData("Monkey_Skill02", "Monkey_Skill02", "猴影", "生成假身，本体换位", 11, 45, 0, "Active", "Active2", "Monkey", "None"),
    SkillData("Monkey_Skill03", "Monkey_Skill03", "云跳", "连续跳跃，快速追击", 5, 20, 350, "Active", "Active3", "Monkey", "None"),
    SkillData("Monkey_Skill04", "Monkey_Skill04", "摘星手", "远程拉扯，拉回目标", 9, 38, 500, "Active", "Active4", "Monkey", "None"),
    SkillData("Monkey_Ultimate", "Monkey_Ultimate", "百猴闹场", "多个猴影扰乱，群体控制", 50, 100, 600, "Ultimate", "Ultimate", "Monkey", "None"),

    # 酉鸡·破晓金翎
    SkillData("Rooster_Passive", "Rooster_Passive", "晨鸣", "附近有隐藏敌人时预警", 0, 0, 0, "Passive", "Passive", "Rooster", "None"),
    SkillData("Rooster_Skill01", "Rooster_Skill01", "金鸡鸣", "扇形声波，暴露隐身目标", 10, 40, 450, "Active", "Active1", "Rooster", "None"),
    SkillData("Rooster_Skill02", "Rooster_Skill02", "金羽标", "标记目标，持续暴露位置", 8, 30, 550, "Active", "Active2", "Rooster", "None"),
    SkillData("Rooster_Skill03", "Rooster_Skill03", "明照", "照亮区域，探测草丛", 13, 50, 400, "Active", "Active3", "Rooster", "None"),
    SkillData("Rooster_Skill04", "Rooster_Skill04", "晨羽阵", "羽阵形成，区域内可见隐身", 16, 65, 350, "Active", "Active4", "Rooster", "None"),
    SkillData("Rooster_Ultimate", "Rooster_Ultimate", "破晓照天", "全图照亮，解除隐身", 55, 100, 700, "Ultimate", "Ultimate", "Rooster", "None"),

    # 戌狗·守门天犬
    SkillData("Dog_Passive", "Dog_Passive", "犬护", "保护附近队友，减伤", 0, 0, 0, "Passive", "Passive", "Dog", "None"),
    SkillData("Dog_Skill01", "Dog_Skill01", "扑援", "扑向队友，提供护盾", 10, 40, 500, "Active", "Active1", "Dog", "None"),
    SkillData("Dog_Skill02", "Dog_Skill02", "犬盾拍", "盾击前方，眩晕控制", 8, 35, 250, "Active", "Active2", "Dog", "None"),
    SkillData("Dog_Skill03", "Dog_Skill03", "灵鼻踪", "追踪目标，显示足迹", 12, 45, 600, "Active", "Active3", "Dog", "None"),
    SkillData("Dog_Skill04", "Dog_Skill04", "护心圈", "队友脚下护圈，抵挡一次", 14, 55, 400, "Active", "Active4", "Dog", "None"),
    SkillData("Dog_Ultimate", "Dog_Ultimate", "天犬守门", "神门结界，区域保护", 50, 100, 500, "Ultimate", "Ultimate", "Dog", "None"),

    # 亥猪·岩甲獠牙
    SkillData("Pig_Passive", "Pig_Passive", "厚甲", "血量越低，双抗越高", 0, 0, 0, "Passive", "Passive", "Pig", "None"),
    SkillData("Pig_Skill01", "Pig_Skill01", "獠拱", "獠牙前拱，击退目标", 9, 35, 300, "Active", "Active1", "Pig", "None"),
    SkillData("Pig_Skill02", "Pig_Skill02", "岩甲蓄", "吸收伤害，结束时反击", 14, 60, 0, "Active", "Active2", "Pig", "None"),
    SkillData("Pig_Skill03", "Pig_Skill03", "锤震", "大锤砸地，范围伤害减速", 11, 45, 350, "Active", "Active3", "Pig", "None"),
    SkillData("Pig_Skill04", "Pig_Skill04", "福印", "地面福印，队友站入减伤", 8, 30, 400, "Active", "Active4", "Pig", "None"),
    SkillData("Pig_Ultimate", "Pig_Ultimate", "福山不动", "站场强化，周身岩甲发亮", 55, 100, 450, "Ultimate", "Ultimate", "Pig", "None"),
]

# CSV 列 - 必须与 FDBASkillDataRow 对齐
SKILL_CSV_COLUMNS = [
    "SkillId", "DisplayName", "Description", "Icon", "SkillType", "SkillSlot",
    "ElementType", "ZodiacType", "Cooldown", "EnergyCost", "UltimateEnergyCost",
    "CastTime", "CastRange", "EffectRadius", "BaseDamage", "DamageScaling",
    "HealAmount", "HealScaling", "ShieldValue", "ControlTime",
    "VFXType", "VFXAsset", "SFXType", "SFXAsset",
    "AnimBlueprintClass", "SkillTags", "TacticalRole", "bIsDefensiveSkill",
    "IconDesignName", "IconDesignDescription", "VFXDesignDescription",
    "UpgradeConfigJson", "MaxSkillLevel", "UpgradeProficiencyRequired",
    "bEnabled", "bIsInDevelopment"
]

def generate_skill_csv():
    """生成技能数据 CSV"""
    output_file = "Scripts/DT_Skills_V15.csv"

    with open(output_file, 'w', encoding='utf-8') as f:
        header = ["Name"] + SKILL_CSV_COLUMNS
        f.write(','.join(header) + '\n')

        for skill in HERO_SKILL_DATA:
            row = [
                skill.row_name,           # Name
                skill.skill_id,           # SkillId
                skill.display_name,       # DisplayName
                skill.description,        # Description
                "None",                   # Icon
                skill.skill_type,         # SkillType: Passive/Active/Ultimate
                skill.skill_slot,         # SkillSlot: Passive/Active1-4/Ultimate
                skill.element_type,       # ElementType: None/Fire/Water/Wood/Gold/Earth
                skill.zodiac_type,        # ZodiacType: Rat/Ox/Tiger/etc.
                str(skill.cooldown),      # Cooldown
                str(skill.energy_cost),   # EnergyCost
                "100",                    # UltimateEnergyCost
                "0",                      # CastTime
                str(skill.cast_range),   # CastRange
                "0",                      # EffectRadius
                "100",                    # BaseDamage
                "0",                      # DamageScaling
                "0",                      # HealAmount
                "0",                      # HealScaling
                "0",                      # ShieldValue
                "0",                      # ControlTime
                "None",                   # VFXType
                "None",                   # VFXAsset
                "None",                   # SFXType
                "None",                   # SFXAsset
                "None",                   # AnimBlueprintClass
                "None",                   # SkillTags
                "None",                   # TacticalRole
                "false",                  # bIsDefensiveSkill
                "None",                   # IconDesignName
                "None",                   # IconDesignDescription
                "None",                   # VFXDesignDescription
                "None",                   # UpgradeConfigJson
                "1",                      # MaxSkillLevel
                "1000",                   # UpgradeProficiencyRequired
                "true",                   # bEnabled
                "false",                  # bIsInDevelopment
            ]
            f.write(','.join(row) + '\n')

    print(f"CSV 已生成: {output_file}")
    print(f"共 {len(HERO_SKILL_DATA)} 条技能数据")

if __name__ == "__main__":
    generate_skill_csv()
