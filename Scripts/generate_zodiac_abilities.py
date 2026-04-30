#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
批量生成 GA (GameplayAbility) 技能类的 Python 脚本
根据十二生肖和五行阵营数据自动生成技能类代码

生成规则:
- 每个生肖有 5 个技能: Passive, Q, W, E, Ultimate(R)
- Passive 继承自 DBAZodiacAbilityBase
- Q/W/E 继承自 DBAElementAbilityBase
- R 继承自 DBAZodiacUltimateAbilityBase

用法:
    python generate_zodiac_abilities.py [--output OUTPUT_DIR]
"""

import os
import argparse
from pathlib import Path
from typing import Dict, List

# ============== 配置 ==============
# 十二生肖列表
ZODIAC_TYPES = [
    ("Rat", "鼠"),
    ("Ox", "牛"),
    ("Tiger", "虎"),
    ("Rabbit", "兔"),
    ("Dragon", "龙"),
    ("Snake", "蛇"),
    ("Horse", "马"),
    ("Goat", "羊"),
    ("Monkey", "猴"),
    ("Rooster", "鸡"),
    ("Dog", "狗"),
    ("Pig", "猪"),
]

# 五行元素
ELEMENTS = [
    ("Gold", "金"),
    ("Wood", "木"),
    ("Water", "水"),
    ("Fire", "火"),
    ("Earth", "土"),
]

# 技能类型
# Passive -> DBAZodiacAbilityBase (有 ZodiacType)
# Q/W/E -> DBAElementAbilityBase (有 ElementType，无 ZodiacType)
# R -> DBAZodiacUltimateAbilityBase (继承自 DBAZodiacAbilityBase，有 ZodiacType)
SKILL_TYPES = [
    ("Passive", "被动技能", "DBAZodiacAbilityBase", "zodiac"),
    ("Q", "Q技能", "DBAElementAbilityBase", "element"),
    ("W", "W技能", "DBAElementAbilityBase", "element"),
    ("E", "E技能", "DBAElementAbilityBase", "element"),
    ("R", "终极技能", "DBAZodiacUltimateAbilityBase", "zodiac"),
]

# ============== 模板 ==============

HEADER_TEMPLATE = """// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 {zodiac_cn}{skill_name} 技能类

#pragma once

#include "CoreMinimal.h"
#include "DBA/GAS/Abilities/{base_class}.h"
#include "DBAGameplayAbility_{zodiac}_{skill}.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API UDBAGameplayAbility_{zodiac}_{skill} : public U{base_class}
{{
	GENERATED_BODY()

public:
	UDBAGameplayAbility_{zodiac}_{skill}();
}};
"""

CPP_TEMPLATE = """// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的 {zodiac_cn}{skill_name} 技能类实现

#include "DBAGameplayAbility_{zodiac}_{skill}.h"

UDBAGameplayAbility_{zodiac}_{skill}::UDBAGameplayAbility_{zodiac}_{skill}()
{{
	// 技能配置
	AbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.{zodiac}.{skill}"), false);
	ActivationPolicy = EDBAMobaAbilityActivationPolicy::OnInputTriggered;

	// 生肖类型 - 仅 ZodiacAbilityBase 和其子类需要设置
{zodiac_config}
}}
"""

# ============== 代码生成 ==============

def get_property_config(skill_type: str, zodiac: str) -> str:
    """获取属性配置
    - zodiac 类型: Passive/R -> 设置 ZodiacType = EDBAZodiacType::{zodiac}
    - element 类型: Q/W/E -> 设置 ElementType = EDBAElement::None (需要外部设置)
    """
    if skill_type == "zodiac":
        return f"\tZodiacType = EDBAZodiacType::{zodiac};"
    else:
        # element 类型不设置 ZodiacType，只在注释中说明需要外部配置
        return "\t// ElementType 由英雄实例在 SpawnAbility 时设置"

def generate_ability_class(zodiac: str, zodiac_cn: str, skill: str, skill_name: str, base_class: str, property_type: str) -> Dict[str, str]:
    """生成单个技能类的头文件和cpp文件内容"""
    property_config = get_property_config(property_type, zodiac)

    header = HEADER_TEMPLATE.format(
        zodiac_cn=zodiac_cn,
        skill_name=skill_name,
        zodiac=zodiac,
        skill=skill,
        base_class=base_class,
        zodiac_config=property_config,
    )

    cpp = CPP_TEMPLATE.format(
        zodiac_cn=zodiac_cn,
        skill_name=skill_name,
        zodiac=zodiac,
        skill=skill,
        zodiac_config=property_config,
    )

    return {
        "header": header,
        "cpp": cpp,
    }

def main():
    parser = argparse.ArgumentParser(description="批量生成 GA 技能类")
    parser.add_argument("--output", "-o", default="Generated/Abilities",
                        help="输出目录路径 (相对于项目根目录)")
    args = parser.parse_args()

    # 项目根目录
    project_root = Path(__file__).parent.parent / "Source" / "DivineBeastsArena"

    # 输出目录
    output_dir = project_root / args.output
    output_dir.mkdir(parents=True, exist_ok=True)

    # 创建子目录
    private_dir = output_dir / "Private"
    public_dir = output_dir / "Public"
    private_dir.mkdir(parents=True, exist_ok=True)
    public_dir.mkdir(parents=True, exist_ok=True)

    generated_count = 0

    # 遍历所有生肖生成技能类
    for zodiac_id, zodiac_cn in ZODIAC_TYPES:
        for skill, skill_name, base_class, property_type in SKILL_TYPES:
            # 生成类名
            class_name = f"DBAGameplayAbility_{zodiac_id}_{skill}"

            # 生成代码
            code = generate_ability_class(zodiac_id, zodiac_cn, skill, skill_name, base_class, property_type)

            # 写入头文件
            header_path = public_dir / f"{class_name}.h"
            with open(header_path, "w", encoding="utf-8") as f:
                f.write(code["header"])

            # 写入cpp文件
            cpp_path = private_dir / f"{class_name}.cpp"
            with open(cpp_path, "w", encoding="utf-8") as f:
                f.write(code["cpp"])

            generated_count += 1
            print(f"  生成: {class_name}")

    # 生成汇总头文件
    generate_summary_header(public_dir, generated_count)

    print(f"\n完成! 共生成 {generated_count} 个技能类")
    print(f"输出目录: {output_dir}")
    print("\n生成的文件需要:")
    print("1. 在 DBA.Build.cs 中添加到 PrivateDependencyModuleNames 或 PublicIncludePaths")
    print("2. 在技能数据表中配置每个技能的 AbilityClass 引用")

def generate_summary_header(output_dir: Path, count: int):
    """生成汇总头文件"""
    include_guard = "DIVINEBEASTSARENA_GAMEPLAY_ABILITIES_GENERATED_H"

    content = f"""// Copyright FreeboozStudio. All Rights Reserved.
// 自动生成的所有 GameplayAbility 类汇总

#pragma once

// 自动生成 {count} 个技能类
// 12 生肖 × 5 技能/生肖 = 60 技能类
//
// 文件列表:
// {get_file_list()}

#ifndef {include_guard}
#define {include_guard}

// 这里可以添加全局技能配置

#endif // {include_guard}
"""

    summary_path = output_dir / "DBAGameplayAbilities_AutoGenerated.generated.h"
    with open(summary_path, "w", encoding="utf-8") as f:
        f.write(content)

def get_file_list() -> str:
    """获取生成的文件列表"""
    files = []
    for zodiac_id, zodiac_cn in ZODIAC_TYPES:
        for skill, _, _, _ in SKILL_TYPES:
            files.append(f"  - DBAGameplayAbility_{zodiac_id}_{skill}.h/cpp")
    return "\n// ".join(files)

if __name__ == "__main__":
    main()