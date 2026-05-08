# Copyright Freebooz Games, Inc. All Rights Reserved.
# -*- coding: utf-8 -*-
"""
元素共鸣数据表生成脚本
"""

import os

class ElementResonanceData:
    def __init__(self, row_name, element_type, resonance_level, display_name, description, icon_path,
                 damage_bonus, defense_bonus, health_bonus, energy_regen_bonus,
                 move_speed_bonus, control_time_bonus, shield_bonus,
                 resonance_vfx_name, resonance_sfx_name):
        self.row_name = row_name
        self.element_type = element_type
        self.resonance_level = resonance_level
        self.display_name = display_name
        self.description = description
        self.icon_path = icon_path
        self.damage_bonus = damage_bonus
        self.defense_bonus = defense_bonus
        self.health_bonus = health_bonus
        self.energy_regen_bonus = energy_regen_bonus
        self.move_speed_bonus = move_speed_bonus
        self.control_time_bonus = control_time_bonus
        self.shield_bonus = shield_bonus
        self.resonance_vfx_name = resonance_vfx_name
        self.resonance_sfx_name = resonance_sfx_name

ELEMENT_RESONANCE_DATA = [
    # Gold (金属金)
    ElementResonanceData("Gold_0", "Gold", 0, "无共鸣", "无共鸣效果", "None", 0, 0, 0, 0, 0, 0, 0, "None", "None"),
    ElementResonanceData("Gold_1", "Gold", 1, "初级共鸣·金", "获得少量金属性加成", "None", 5, 3, 2, 5, 2, 0.1, 5, "VFX_Resonance_Gold_L1", "SFX_Resonance_Gold"),
    ElementResonanceData("Gold_2", "Gold", 2, "中级共鸣·金", "获得中量金属性加成", "None", 10, 6, 4, 10, 4, 0.2, 10, "VFX_Resonance_Gold_L2", "SFX_Resonance_Gold"),
    ElementResonanceData("Gold_3", "Gold", 3, "高级共鸣·金", "获得大量金属性加成", "None", 15, 9, 6, 15, 6, 0.3, 15, "VFX_Resonance_Gold_L3", "SFX_Resonance_Gold"),
    ElementResonanceData("Gold_4", "Gold", 4, "超级共鸣·金", "获得最大金属性加成", "None", 20, 12, 8, 20, 8, 0.4, 20, "VFX_Resonance_Gold_L4", "SFX_Resonance_Gold_Ultra"),
    # Wood (木属木)
    ElementResonanceData("Wood_0", "Wood", 0, "无共鸣", "无共鸣效果", "None", 0, 0, 0, 0, 0, 0, 0, "None", "None"),
    ElementResonanceData("Wood_1", "Wood", 1, "初级共鸣·木", "获得少量木属性加成", "None", 3, 5, 5, 3, 4, 0.15, 3, "VFX_Resonance_Wood_L1", "SFX_Resonance_Wood"),
    ElementResonanceData("Wood_2", "Wood", 2, "中级共鸣·木", "获得中量木属性加成", "None", 6, 10, 10, 6, 8, 0.3, 6, "VFX_Resonance_Wood_L2", "SFX_Resonance_Wood"),
    ElementResonanceData("Wood_3", "Wood", 3, "高级共鸣·木", "获得大量木属性加成", "None", 9, 15, 15, 9, 12, 0.45, 9, "VFX_Resonance_Wood_L3", "SFX_Resonance_Wood"),
    ElementResonanceData("Wood_4", "Wood", 4, "超级共鸣·木", "获得最大木属性加成", "None", 12, 20, 20, 12, 16, 0.6, 12, "VFX_Resonance_Wood_L4", "SFX_Resonance_Wood_Ultra"),
    # Water (水属水)
    ElementResonanceData("Water_0", "Water", 0, "无共鸣", "无共鸣效果", "None", 0, 0, 0, 0, 0, 0, 0, "None", "None"),
    ElementResonanceData("Water_1", "Water", 1, "初级共鸣·水", "获得少量水属性加成", "None", 4, 4, 6, 4, 3, 0.2, 4, "VFX_Resonance_Water_L1", "SFX_Resonance_Water"),
    ElementResonanceData("Water_2", "Water", 2, "中级共鸣·水", "获得中量水属性加成", "None", 8, 8, 12, 8, 6, 0.4, 8, "VFX_Resonance_Water_L2", "SFX_Resonance_Water"),
    ElementResonanceData("Water_3", "Water", 3, "高级共鸣·水", "获得大量水属性加成", "None", 12, 12, 18, 12, 9, 0.6, 12, "VFX_Resonance_Water_L3", "SFX_Resonance_Water"),
    ElementResonanceData("Water_4", "Water", 4, "超级共鸣·水", "获得最大水属性加成", "None", 16, 16, 24, 16, 12, 0.8, 16, "VFX_Resonance_Water_L4", "SFX_Resonance_Water_Ultra"),
    # Fire (火属火)
    ElementResonanceData("Fire_0", "Fire", 0, "无共鸣", "无共鸣效果", "None", 0, 0, 0, 0, 0, 0, 0, "None", "None"),
    ElementResonanceData("Fire_1", "Fire", 1, "初级共鸣·火", "获得少量火属性加成", "None", 6, 2, 3, 6, 5, 0.1, 3, "VFX_Resonance_Fire_L1", "SFX_Resonance_Fire"),
    ElementResonanceData("Fire_2", "Fire", 2, "中级共鸣·火", "获得中量火属性加成", "None", 12, 4, 6, 12, 10, 0.2, 6, "VFX_Resonance_Fire_L2", "SFX_Resonance_Fire"),
    ElementResonanceData("Fire_3", "Fire", 3, "高级共鸣·火", "获得大量火属性加成", "None", 18, 6, 9, 18, 15, 0.3, 9, "VFX_Resonance_Fire_L3", "SFX_Resonance_Fire"),
    ElementResonanceData("Fire_4", "Fire", 4, "超级共鸣·火", "获得最大火属性加成", "None", 24, 8, 12, 24, 20, 0.4, 12, "VFX_Resonance_Fire_L4", "SFX_Resonance_Fire_Ultra"),
    # Earth (土属土)
    ElementResonanceData("Earth_0", "Earth", 0, "无共鸣", "无共鸣效果", "None", 0, 0, 0, 0, 0, 0, 0, "None", "None"),
    ElementResonanceData("Earth_1", "Earth", 1, "初级共鸣·土", "获得少量土属性加成", "None", 2, 6, 4, 2, 3, 0.15, 5, "VFX_Resonance_Earth_L1", "SFX_Resonance_Earth"),
    ElementResonanceData("Earth_2", "Earth", 2, "中级共鸣·土", "获得中量土属性加成", "None", 4, 12, 8, 4, 6, 0.3, 10, "VFX_Resonance_Earth_L2", "SFX_Resonance_Earth"),
    ElementResonanceData("Earth_3", "Earth", 3, "高级共鸣·土", "获得大量土属性加成", "None", 6, 18, 12, 6, 9, 0.45, 15, "VFX_Resonance_Earth_L3", "SFX_Resonance_Earth"),
    ElementResonanceData("Earth_4", "Earth", 4, "超级共鸣·土", "获得最大土属性加成", "None", 8, 24, 16, 8, 12, 0.6, 20, "VFX_Resonance_Earth_L4", "SFX_Resonance_Earth_Ultra"),
]

# CSV 第一列是行名称（Name），后面是数据列
ELEMENT_RESONANCE_COLUMNS = [
    "ElementType", "ResonanceLevel", "DisplayName", "Description", "Icon",
    "DamageBonus", "DefenseBonus", "HealthBonus", "EnergyRegenBonus",
    "MoveSpeedBonus", "ControlTimeBonus", "ShieldBonus",
    "ResonanceVFXName", "ResonanceSFXName",
]

def generate_element_resonance_csv():
    """生成元素共鸣 CSV 文件"""
    output_file = "Scripts/DT_ElementResonance_V15.csv"

    with open(output_file, 'w', encoding='utf-8') as f:
        # 写入 Name 列 + 数据列
        header = ["Name"] + ELEMENT_RESONANCE_COLUMNS
        f.write(','.join(header) + '\n')

        for data in ELEMENT_RESONANCE_DATA:
            row = [
                data.row_name,
                data.element_type,
                str(data.resonance_level),
                data.display_name,
                data.description,
                data.icon_path,
                str(data.damage_bonus),
                str(data.defense_bonus),
                str(data.health_bonus),
                str(data.energy_regen_bonus),
                str(data.move_speed_bonus),
                str(data.control_time_bonus),
                str(data.shield_bonus),
                data.resonance_vfx_name,
                data.resonance_sfx_name,
            ]
            f.write(','.join(row) + '\n')

    print(f"CSV 文件已生成: {output_file}")

if __name__ == "__main__":
    print("=" * 60)
    print("元素共鸣数据表生成脚本")
    print("=" * 60)
    generate_element_resonance_csv()
    print("\n完成!")
