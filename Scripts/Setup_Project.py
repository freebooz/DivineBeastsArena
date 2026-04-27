#!/usr/bin/env python3
# Copyright FreeboozStudio. All Rights Reserved.
# 项目设置和 GAS 系统初始化脚本

import os
import sys
import json
import csv
import shutil
from pathlib import Path

# ============ 配置 ============
ENGINE_ROOT = r"E:\UnrealEngine-5.7.1-release"
PROJECT_ROOT = Path(__file__).parent.parent.absolute()

# 共鸣等级配置
RESONANCE_CONFIG = {
    0: {"DamageBonus": 0.0, "DefenseBonus": 0.0, "HealthBonus": 0.0,
        "EnergyRegenBonus": 0.0, "MoveSpeedBonus": 0.0, "ControlTime": 0.0, "ShieldBonus": 0.0},
    1: {"DamageBonus": 5.0, "DefenseBonus": 2.0, "HealthBonus": 3.0,
        "EnergyRegenBonus": 0.0, "MoveSpeedBonus": 0.0, "ControlTime": 0.25, "ShieldBonus": 5.0},
    2: {"DamageBonus": 10.0, "DefenseBonus": 4.0, "HealthBonus": 6.0,
        "EnergyRegenBonus": 0.0, "MoveSpeedBonus": 0.0, "ControlTime": 0.50, "ShieldBonus": 10.0},
    3: {"DamageBonus": 15.0, "DefenseBonus": 6.0, "HealthBonus": 9.0,
        "EnergyRegenBonus": 0.0, "MoveSpeedBonus": 0.0, "ControlTime": 0.75, "ShieldBonus": 15.0},
    4: {"DamageBonus": 20.0, "DefenseBonus": 8.0, "HealthBonus": 12.0,
        "EnergyRegenBonus": 0.0, "MoveSpeedBonus": 0.0, "ControlTime": 1.00, "ShieldBonus": 20.0},
}

ELEMENTS = ["Metal", "Wood", "Water", "Fire", "Earth"]

# ============ 功能函数 ============

def print_header(title):
    print("\n" + "=" * 60)
    print(f"  {title}")
    print("=" * 60)

def print_step(step, total, message):
    print(f"\n[{step}/{total}] {message}")

def check_engine():
    """检查引擎路径"""
    print_step(1, 3, "检查引擎配置...")

    engine_build = Path(ENGINE_ROOT) / "Engine" / "Build" / "BatchFiles" / "Build.bat"

    if engine_build.exists():
        print(f"  [OK] 找到引擎 Build.bat: {engine_build}")
        return True
    else:
        print(f"  [ERROR] 未找到引擎 Build.bat: {engine_build}")
        print(f"  请确认 ENGINE_ROOT 配置正确: {ENGINE_ROOT}")
        return False

def create_directories():
    """创建必要的目录结构"""
    print_step(2, 3, "创建目录结构...")

    dirs_to_create = [
        PROJECT_ROOT / "Scripts",
        PROJECT_ROOT / "Exports" / "DataTables",
    ]

    for dir_path in dirs_to_create:
        dir_path.mkdir(parents=True, exist_ok=True)
        print(f"  [OK] {dir_path.relative_to(PROJECT_ROOT)}")

def generate_resonance_csv():
    """生成共鸣加成数据表 CSV"""
    print_step(3, 3, "生成共鸣数据表...")

    output_path = PROJECT_ROOT / "Exports" / "DataTables" / "DBA_ResonanceBonusDataTable.csv"

    with open(output_path, 'w', newline='', encoding='utf-8') as f:
        writer = csv.writer(f)

        # 表头
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

        # 数据行
        for element in ELEMENTS:
            for level in range(5):
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
                    config["ControlTime"],
                    config["ShieldBonus"],
                ])

    print(f"  [OK] CSV 已生成: {output_path.relative_to(PROJECT_ROOT)}")
    return output_path

def print_summary():
    """打印设置完成摘要"""
    print_header("设置完成")

    print("""
下一步操作：

1. 编译项目
   运行 Scripts/Build_Development.bat

2. UE 编辑器中创建数据表
   - 打开项目
   - 内容浏览器右键 → 杂项 → DataTable
   - 选择行类型: FDBAResonanceBonusDataRow
   - 导入 Exports/DataTables/DBA_ResonanceBonusDataTable.csv

3. 创建 GameplayEffect 资产（蓝图）
   - 内容浏览器右键 → 杂项 → Gameplay Effect
   - 命名为 GE_Resonance_Buff_Lv1 ~ Lv4
   - 配置 Duration: HasDuration
   - 配置 Modifiers: AttackPower, Defense, MaxHealth

4. 更新技能配置
   - 在 DBAElementAbilityBase 的 CostGameplayEffect
     设置为 DBEEnergyCostEffect
    """)

# ============ 主流程 ============

def main():
    print_header("神兽竞技场 - GAS 系统初始化")

    # 检查引擎
    if not check_engine():
        sys.exit(1)

    # 创建目录
    create_directories()

    # 生成数据
    generate_resonance_csv()

    # 打印摘要
    print_summary()

if __name__ == "__main__":
    main()
