# Copyright FreeboozStudio. All Rights Reserved.
# 生成共鸣加成数据表 CSV 文件

import csv
import os

# 共鸣等级配置
RESONANCE_CONFIG = {
    0: {"DamageBonus": 0.0, "DefenseBonus": 0.0, "HealthBonus": 0.0, "EnergyRegenBonus": 0.0, "MoveSpeedBonus": 0.0, "ControlTimeBonus": 0.0, "ShieldBonus": 0.0},
    1: {"DamageBonus": 5.0, "DefenseBonus": 2.0, "HealthBonus": 3.0, "EnergyRegenBonus": 0.0, "MoveSpeedBonus": 0.0, "ControlTimeBonus": 0.25, "ShieldBonus": 5.0},
    2: {"DamageBonus": 10.0, "DefenseBonus": 4.0, "HealthBonus": 6.0, "EnergyRegenBonus": 0.0, "MoveSpeedBonus": 0.0, "ControlTimeBonus": 0.50, "ShieldBonus": 10.0},
    3: {"DamageBonus": 15.0, "DefenseBonus": 6.0, "HealthBonus": 9.0, "EnergyRegenBonus": 0.0, "MoveSpeedBonus": 0.0, "ControlTimeBonus": 0.75, "ShieldBonus": 15.0},
    4: {"DamageBonus": 20.0, "DefenseBonus": 8.0, "HealthBonus": 12.0, "EnergyRegenBonus": 0.0, "MoveSpeedBonus": 0.0, "ControlTimeBonus": 1.00, "ShieldBonus": 20.0},
}

# 元素类型
ELEMENTS = ["Metal", "Wood", "Water", "Fire", "Earth"]

def generate_csv():
    """生成共鸣加成数据表 CSV"""
    output_dir = os.path.join(os.path.dirname(os.path.dirname(__file__)), "Exports", "DataTables")

    # 确保输出目录存在
    os.makedirs(output_dir, exist_ok=True)

    csv_path = os.path.join(output_dir, "DBA_ResonanceBonusDataTable.csv")

    with open(csv_path, 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)

        # 写入表头
        writer.writerow([
            "RowName",
            "ResonanceLevel",
            "DamageBonusPercent",
            "DefenseBonusPercent",
            "HealthBonusPercent",
            "EnergyRegenBonusPercent",
            "MoveSpeedBonusPercent",
            "ControlTimeBonusSeconds",
            "ShieldBonusPercent",
        ])

        # 写入数据行
        for element in ELEMENTS:
            for level in range(5):  # 0-4
                config = RESONANCE_CONFIG[level]
                row_name = f"{element}_{level}"

                writer.writerow([
                    row_name,
                    level,
                    config["DamageBonus"],
                    config["DefenseBonus"],
                    config["HealthBonus"],
                    config["EnergyRegenBonus"],
                    config["MoveSpeedBonus"],
                    config["ControlTimeBonus"],
                    config["ShieldBonus"],
                ])

    print(f"CSV 已生成: {csv_path}")
    return csv_path

if __name__ == "__main__":
    generate_csv()
