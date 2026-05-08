# Copyright Freebooz Games, Inc. All Rights Reserved.
# -*- coding: utf-8 -*-
"""
UE5 DataTable 资产创建脚本
在 UE5 编辑器中运行 (Python Console)

使用方法:
1. 在 UE5 编辑器中打开项目
2. 打开 Python Console (Window -> Developer Tools -> Python Console)
3. 执行: exec(open('Scripts/CreateMissingDataTables.py', encoding='utf-8').read())
"""

import unreal
import os

# ========================================
# 路径常量
# ========================================

ELEMENT_RESONANCE_TABLE_PATH = "/Game/Data/Elements/ElementResonanceTable"
SKILL_DATA_TABLE_PATH = "/Game/Data/Skills/SkillDataTable"

# ========================================
# 检查函数
# ========================================

def check_datatable(table_path):
    """检查 DataTable 是否存在"""
    if unreal.EditorAssetLibrary.does_asset_exist(table_path):
        print(f"[已存在] {table_path}")
        return True
    else:
        print(f"[不存在] {table_path}")
        return False

def check_struct(struct_path):
    """检查结构体是否存在"""
    if unreal.load_object(None, struct_path):
        print(f"[结构体存在] {struct_path}")
        return True
    else:
        print(f"[结构体不存在] {struct_path}")
        return False

def list_csv_files():
    """列出可用的 CSV 文件"""
    print("\n可用的 CSV 文件:")
    csv_dir = "Scripts"
    if os.path.exists(csv_dir):
        for f in os.listdir(csv_dir):
            if f.endswith(".csv"):
                filepath = os.path.join(csv_dir, f)
                size = os.path.getsize(filepath)
                print(f"  - {filepath} ({size} bytes)")

def manual_instructions():
    """显示手动创建说明"""
    print("\n" + "=" * 60)
    print("手动创建 DataTable 步骤:")
    print("=" * 60)
    print("\n1. ElementResonanceTable:")
    print("   - 路径: /Game/Data/Elements/")
    print("   - 右键 -> Miscellaneous -> Data Table")
    print("   - Row Structure: DBAElementResonanceRow")
    print("   - 保存为 ElementResonanceTable")
    print("   - 右键 -> Import CSV -> Scripts/DT_ElementResonance_V15.csv")
    print("\n2. SkillDataTable:")
    print("   - 路径: /Game/Data/Skills/")
    print("   - 右键 -> Miscellaneous -> Data Table")
    print("   - Row Structure: FDBASkillDataRow")
    print("   - 保存为 SkillDataTable")
    print("   - 右键 -> Import CSV -> Scripts/DT_Skills_V15.csv")
    print("\n3. 保存所有资产: Ctrl+Shift+S")

# ========================================
# 主函数
# ========================================

def main():
    print("=" * 60)
    print("检查缺失的 DataTable 资产")
    print("=" * 60)

    # 检查结构体
    print("\n[1/3] 检查结构体...")
    check_struct("/Script/DivineBeastsArena.DBAElementResonanceRow")
    check_struct("/Script/DivineBeastsArena.FDBASkillDataRow")

    # 检查 DataTable
    print("\n[2/3] 检查 DataTable...")
    check_datatable(ELEMENT_RESONANCE_TABLE_PATH)
    check_datatable(SKILL_DATA_TABLE_PATH)

    # 列出 CSV 文件
    print("\n[3/3] CSV 文件列表...")
    list_csv_files()

    manual_instructions()

if __name__ == "__main__":
    main()
