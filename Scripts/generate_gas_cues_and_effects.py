#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
生成 GAS GameplayCue 和 GameplayEffect 脚本
十二生肖 × 5 技能 = 60 个技能 Cue + 60 个技能 Effect

生成规则:
- 每个生肖有 5 个技能: Passive, Q, W, E, Ultimate(R)
- 每个技能对应一个 GameplayCue 和一个 GameplayEffect

用法:
    python generate_gas_cues_and_effects.py
"""

import os
from pathlib import Path

# ============== 配置 ==============
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

SKILL_TYPES = [
    ("Passive", "被动技能", "ZodiacPassive"),
    ("Q", "Q技能", "ElementSkill"),
    ("W", "W技能", "ElementSkill"),
    ("E", "E技能", "ElementSkill"),
    ("R", "终极技能", "ZodiacUltimate"),
]

ELEMENTS = [
    ("Fire", "火"),
    ("Water", "水"),
    ("Wood", "木"),
    ("Gold", "金"),
    ("Earth", "土"),
]

# ============== 模板 ==============

# GameplayCue 头文件模板
CUE_HEADER_TEMPLATE = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayCue - {zodiac_cn}{skill_name}

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "DBACue_{zodiac}_{skill}.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API UDBACue_{zodiac}_{skill} : public AGameplayCueNotify_Actor
{{
	GENERATED_BODY()

public:
	UDBACue_{zodiac}_{skill}();

	// 当 Cue 被触发时调用
	virtual bool OnExecuteGameplayCue(AActor* Target, FGameplayCueParameters& Parameters) override;

	// 当 Cue 生效时调用 (持续性 Cue)
	virtual void OnActiveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters) override;

	// 当 Cue 结束时调用
	virtual void OnRemoveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters) override;

protected:
	// 特效峰值缩放
	UPROPERTY(EditDefaultsOnly, Category = "Cue")
	float CueScale = 1.0f;
}};
"""

# GameplayCue cpp 文件模板
CUE_CPP_TEMPLATE = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayCue - {zodiac_cn}{skill_name}

#include "DBA/GAS/Cues/DBACue_{zodiac}_{skill}.h"

UDBACue_{zodiac}_{skill}::UDBACue_{zodiac}_{skill}()
{{
	// 默认配置
	bAutoDestroyOnOwnerRemoved = true;
	bOnlyRelevantToOwner = false;
	RollbackPolicy = ECueRollback::CanRollback;
}}

bool UDBACue_{zodiac}_{skill}::OnExecuteGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{{
	// TODO: 播放施法特效
	// 示例: PlayEffects(Target, Parameters);

	return true;
}}

void UDBACue_{zodiac}_{skill}::OnActiveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{{
	// TODO: 激活状态特效
}}

void UDBACue_{zodiac}_{skill}::OnRemoveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{{
	// TODO: 移除特效
}}
"""

# GameplayEffect 头文件模板
GE_HEADER_TEMPLATE = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayEffect - {zodiac_cn}{skill_name}

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "DBAGE_{zodiac}_{skill}.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API UDBAGE_{zodiac}_{skill} : public UGameplayEffect
{{
	GENERATED_BODY()

public:
	UDBAGE_{zodiac}_{skill}();
}};
"""

# GameplayEffect cpp 文件模板
GE_CPP_TEMPLATE = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayEffect - {zodiac_cn}{skill_name}

#include "DBA/GAS/Effects/DBAGE_{zodiac}_{skill}.h"

UDBAGE_{zodiac}_{skill}::UDBAGE_{zodiac}_{skill}()
{{
	// 技能效果配置
	// DurationPolicy = EGameplayEffectDurationType::Instant;

	// 示例: 伤害修饰
	// FGameplayModifierInfo DamageMod;
	// DamageMod.Attribute = UDBABattleAttributeSet::GetCurrentHealthAttribute();
	// DamageMod.ModifierOp = EGameplayModOp::Additive;
	// DamageMod.ModifierMagnitude = FScalableFloat(10.0f);
	// Modifiers.Add(DamageMod);
}}
"""


def generate_cue_class(zodiac: str, zodiac_cn: str, skill: str, skill_name: str) -> dict:
    """生成单个 GameplayCue 类的头文件和cpp文件内容"""
    header = CUE_HEADER_TEMPLATE.format(
        zodiac_cn=zodiac_cn,
        skill_name=skill_name,
        zodiac=zodiac,
        skill=skill,
    )
    cpp = CUE_CPP_TEMPLATE.format(
        zodiac_cn=zodiac_cn,
        skill_name=skill_name,
        zodiac=zodiac,
        skill=skill,
    )
    return {"header": header, "cpp": cpp}


def generate_ge_class(zodiac: str, zodiac_cn: str, skill: str, skill_name: str) -> dict:
    """生成单个 GameplayEffect 类的头文件和cpp文件内容"""
    header = GE_HEADER_TEMPLATE.format(
        zodiac_cn=zodiac_cn,
        skill_name=skill_name,
        zodiac=zodiac,
        skill=skill,
    )
    cpp = GE_CPP_TEMPLATE.format(
        zodiac_cn=zodiac_cn,
        skill_name=skill_name,
        zodiac=zodiac,
        skill=skill,
    )
    return {"header": header, "cpp": cpp}


def main():
    project_root = Path(__file__).parent.parent / "Source" / "DivineBeastsArena"

    # Cue 目录
    cue_public_dir = project_root / "Public" / "DBA" / "GAS" / "Cues"
    cue_private_dir = project_root / "Private" / "DBA" / "GAS" / "Cues"
    cue_public_dir.mkdir(parents=True, exist_ok=True)
    cue_private_dir.mkdir(parents=True, exist_ok=True)

    # Effect 目录
    ge_public_dir = project_root / "Public" / "DBA" / "GAS" / "Effects"
    ge_private_dir = project_root / "Private" / "DBA" / "GAS" / "Effects"
    ge_public_dir.mkdir(parents=True, exist_ok=True)
    ge_private_dir.mkdir(parents=True, exist_ok=True)

    cue_count = 0
    ge_count = 0

    for zodiac_id, zodiac_cn in ZODIAC_TYPES:
        for skill, skill_name, cue_type in SKILL_TYPES:
            # 生成 GameplayCue
            cue_code = generate_cue_class(zodiac_id, zodiac_cn, skill, skill_name)
            cue_header_path = cue_public_dir / f"DBACue_{zodiac_id}_{skill}.h"
            cue_cpp_path = cue_private_dir / f"DBACue_{zodiac_id}_{skill}.cpp"
            with open(cue_header_path, "w", encoding="utf-8") as f:
                f.write(cue_code["header"])
            with open(cue_cpp_path, "w", encoding="utf-8") as f:
                f.write(cue_code["cpp"])
            cue_count += 1

            # 生成 GameplayEffect
            ge_code = generate_ge_class(zodiac_id, zodiac_cn, skill, skill_name)
            ge_header_path = ge_public_dir / f"DBAGE_{zodiac_id}_{skill}.h"
            ge_cpp_path = ge_private_dir / f"DBAGE_{zodiac_id}_{skill}.cpp"
            with open(ge_header_path, "w", encoding="utf-8") as f:
                f.write(ge_code["header"])
            with open(ge_cpp_path, "w", encoding="utf-8") as f:
                f.write(ge_code["cpp"])
            ge_count += 1

            print(f"  生成: DBACue_{zodiac_id}_{skill} + DBAGE_{zodiac_id}_{skill}")

    # 生成元素共鸣效果
    generate_resonance_effects(ge_public_dir, ge_private_dir, ge_count)

    print(f"\\n完成! 共生成:")
    print(f"  - {cue_count} 个 GameplayCue 类")
    print(f"  - {ge_count + 5} 个 GameplayEffect 类 (含 5 个元素共鸣)")


def generate_resonance_effects(ge_public_dir, ge_private_dir, start_count):
    """生成元素共鸣 GameplayEffect"""
    for element_id, element_cn in ELEMENTS:
        header = GE_HEADER_TEMPLATE.format(
            zodiac_cn=f"{element_cn}共鸣",
            skill_name="元素共鸣",
            zodiac=element_id,
            skill="Resonance",
        )
        cpp = GE_CPP_TEMPLATE.format(
            zodiac_cn=f"{element_cn}共鸣",
            skill_name="元素共鸣",
            zodiac=element_id,
            skill="Resonance",
        )
        header_path = ge_public_dir / f"DBAGE_{element_id}_Resonance.h"
        cpp_path = ge_private_dir / f"DBAGE_{element_id}_Resonance.cpp"
        with open(header_path, "w", encoding="utf-8") as f:
            f.write(header)
        with open(cpp_path, "w", encoding="utf-8") as f:
            f.write(cpp)
        print(f"  生成: DBAGE_Resonance_{element_id}")


if __name__ == "__main__":
    main()