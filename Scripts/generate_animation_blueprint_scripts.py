#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
生成动画蓝图脚本

功能:
- 为十二生肖角色生成动画蓝图配置
- 定义动画状态机结构
- 生成动画蓝图支持类

用法:
    python generate_animation_blueprint_scripts.py
"""

import os
from pathlib import Path

# ============== 配置 ==============
ZODIAC_TYPES = [
    ("Rat", "夜隐灵鼠"),
    ("Ox", "镇岳神牛"),
    ("Tiger", "裂风虎君"),
    ("Rabbit", "月华灵兔"),
    ("Dragon", "云巡龙君"),
    ("Snake", "玄花灵蛇"),
    ("Horse", "踏风天驹"),
    ("Goat", "灵泽仙羊"),
    ("Monkey", "幻云灵猿"),
    ("Rooster", "曜鸣神鸡"),
    ("Dog", "镇魄灵犬"),
    ("Pig", "福岳灵猪"),
]

# 动画类型
ANIM_TYPES = [
    ("Idle", "待机"),
    ("Walk", "行走"),
    ("Run", "奔跑"),
    ("Attack", "攻击"),
    ("Skill", "技能"),
    ("Hit", "受击"),
    ("Death", "死亡"),
]

# ============== 模板 ==============

# 动画状态枚举头文件
ANIM_STATE_ENUM = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 动画状态枚举

#pragma once

#include "CoreMinimal.h"
#include "DBAAnimState.generated.h"

/**
 * EDBAAnimState
 * 动画状态枚举
 */
UENUM(BlueprintType)
enum class EDBAAnimState : uint8
{{
	Idle UMETA(DisplayName = "待机"),
	Walk UMETA(DisplayName = "行走"),
	Run UMETA(DisplayName = "奔跑"),
	Attack UMETA(DisplayName = "攻击"),
	Skill UMETA(DisplayName = "技能"),
	Hit UMETA(DisplayName = "受击"),
	Death UMETA(DisplayName = "死亡"),
}};
"""

# 动画配置结构体
ANIM_CONFIG_H = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 动画配置

#pragma once

#include "CoreMinimal.h"
#include "DBAAnimConfig.generated.h"

USTRUCT(BlueprintType)
struct FDBAAnimConfig
{{
	GENERATED_BODY()

	/** 待机动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> IdleMontage;

	/** 行走动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> WalkMontage;

	/** 奔跑动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> RunMontage;

	/** 攻击动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> AttackMontage;

	/** 技能动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> SkillMontage;

	/** 受击动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> HitMontage;

	/** 死亡动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> DeathMontage;
}};
"""

# 生肖特定动画配置
ZODIAC_ANIM_CONFIG_H = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖动画配置 - {zodiac_cn}

#pragma once

#include "CoreMinimal.h"
#include "DBAZodiacAnimConfig_{zodiac}.generated.h"
#include "Animation/DBAAnimConfig.h"

/**
 * UDBAZodiacAnimConfig_{zodiac}
 * {zodiac_cn}动画配置
 * 定义{zodiac_cn}角色特有的动画资源
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAZodiacAnimConfig_{zodiac} : public UDataAsset
{{
	GENERATED_BODY()

public:
	UDBAZodiacAnimConfig_{zodiac}();

	/** 待机动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|{zodiac_cn}")
	TSoftObjectPtr<UAnimMontage> Idle_Montage;

	/** 行走动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|{zodiac_cn}")
	TSoftObjectPtr<UAnimMontage> Walk_Montage;

	/** 奔跑动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|{zodiac_cn}")
	TSoftObjectPtr<UAnimMontage> Run_Montage;

	/** 普通攻击动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|{zodiac_cn}")
	TSoftObjectPtr<UAnimMontage> Attack_Montage;

	/** 被动技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|{zodiac_cn}")
	TSoftObjectPtr<UAnimMontage> Passive_Montage;

	/** Q技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|{zodiac_cn}")
	TSoftObjectPtr<UAnimMontage> Q_Montage;

	/** W技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|{zodiac_cn}")
	TSoftObjectPtr<UAnimMontage> W_Montage;

	/** E技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|{zodiac_cn}")
	TSoftObjectPtr<UAnimMontage> E_Montage;

	/** R终极技能动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|{zodiac_cn}")
	TSoftObjectPtr<UAnimMontage> R_Montage;

	/** 受击动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|{zodiac_cn}")
	TSoftObjectPtr<UAnimMontage> Hit_Montage;

	/** 死亡动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|{zodiac_cn}")
	TSoftObjectPtr<UAnimMontage> Death_Montage;
}};
"""

# 生肖特定动画配置cpp
ZODIAC_ANIM_CONFIG_CPP = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖动画配置 - {zodiac_cn}

#include "Animation/Zodiac/DBAZodiacAnimConfig_{zodiac}.h"

UDBAZodiacAnimConfig_{zodiac}::UDBAZodiacAnimConfig_{zodiac}()
{{
	// 设置默认资源路径
	static ConstructorHelpers::FObjectFinder<UAnimMontage> IdleFinder(
		TEXT("/Game/Animation/Zodiac/{zodiac}/Montages/AM_{zodiac}_Idle.AM_{zodiac}_Idle"));
	if (IdleFinder.Succeeded())
	{{
		Idle_Montage = IdleFinder.Object;
	}}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> WalkFinder(
		TEXT("/Game/Animation/Zodiac/{zodiac}/Montages/AM_{zodiac}_Walk.AM_{zodiac}_Walk"));
	if (WalkFinder.Succeeded())
	{{
		Walk_Montage = WalkFinder.Object;
	}}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> RunFinder(
		TEXT("/Game/Animation/Zodiac/{zodiac}/Montages/AM_{zodiac}_Run.AM_{zodiac}_Run"));
	if (RunFinder.Succeeded())
	{{
		Run_Montage = RunFinder.Object;
	}}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> AttackFinder(
		TEXT("/Game/Animation/Zodiac/{zodiac}/Montages/AM_{zodiac}_Attack.AM_{zodiac}_Attack"));
	if (AttackFinder.Succeeded())
	{{
		Attack_Montage = AttackFinder.Object;
	}}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> PassiveFinder(
		TEXT("/Game/Animation/Zodiac/{zodiac}/Montages/AM_{zodiac}_Passive.AM_{zodiac}_Passive"));
	if (PassiveFinder.Succeeded())
	{{
		Passive_Montage = PassiveFinder.Object;
	}}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> QFinder(
		TEXT("/Game/Animation/Zodiac/{zodiac}/Montages/AM_{zodiac}_Q.AM_{zodiac}_Q"));
	if (QFinder.Succeeded())
	{{
		Q_Montage = QFinder.Object;
	}}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> WFinder(
		TEXT("/Game/Animation/Zodiac/{zodiac}/Montages/AM_{zodiac}_W.AM_{zodiac}_W"));
	if (WFinder.Succeeded())
	{{
		W_Montage = WFinder.Object;
	}}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> EFinder(
		TEXT("/Game/Animation/Zodiac/{zodiac}/Montages/AM_{zodiac}_E.AM_{zodiac}_E"));
	if (EFinder.Succeeded())
	{{
		E_Montage = EFinder.Object;
	}}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> RFinder(
		TEXT("/Game/Animation/Zodiac/{zodiac}/Montages/AM_{zodiac}_R.AM_{zodiac}_R"));
	if (RFinder.Succeeded())
	{{
		R_Montage = RFinder.Object;
	}}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> HitFinder(
		TEXT("/Game/Animation/Zodiac/{zodiac}/Montages/AM_{zodiac}_Hit.AM_{zodiac}_Hit"));
	if (HitFinder.Succeeded())
	{{
		Hit_Montage = HitFinder.Object;
	}}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> DeathFinder(
		TEXT("/Game/Animation/Zodiac/{zodiac}/Montages/AM_{zodiac}_Death.AM_{zodiac}_Death"));
	if (DeathFinder.Succeeded())
	{{
		Death_Montage = DeathFinder.Object;
	}}
}}
"""


def generate_zodiac_anim_config(zodiac_id, zodiac_cn):
    """生成生肖动画配置类"""
    header = ZODIAC_ANIM_CONFIG_H.format(zodiac_cn=zodiac_cn, zodiac=zodiac_id)
    cpp = ZODIAC_ANIM_CONFIG_CPP.format(zodiac_cn=zodiac_cn, zodiac=zodiac_id)
    return {"header": header, "cpp": cpp}


def main():
    project_root = Path(__file__).parent.parent / "Source" / "DivineBeastsArena"

    # 动画目录
    anim_public_dir = project_root / "Public" / "Animation" / "Zodiac"
    anim_private_dir = project_root / "Private" / "Animation" / "Zodiac"
    anim_public_dir.mkdir(parents=True, exist_ok=True)
    anim_private_dir.mkdir(parents=True, exist_ok=True)

    # 生成动画状态枚举
    with open(anim_public_dir / "DBAAnimState.h", "w", encoding="utf-8") as f:
        f.write(ANIM_STATE_ENUM)
    print("  生成: DBAAnimState.h")

    # 生成通用动画配置
    with open(anim_public_dir / "DBAAnimConfig.h", "w", encoding="utf-8") as f:
        f.write(ANIM_CONFIG_H)
    print("  生成: DBAAnimConfig.h")

    # 生成12个生肖的动画配置
    count = 0
    for zodiac_id, zodiac_cn in ZODIAC_TYPES:
        code = generate_zodiac_anim_config(zodiac_id, zodiac_cn)
        header_path = anim_public_dir / f"DBAZodiacAnimConfig_{zodiac_id}.h"
        cpp_path = anim_private_dir / f"DBAZodiacAnimConfig_{zodiac_id}.cpp"
        with open(header_path, "w", encoding="utf-8") as f:
            f.write(code["header"])
        with open(cpp_path, "w", encoding="utf-8") as f:
            f.write(code["cpp"])
        count += 1
        print(f"  生成: DBAZodiacAnimConfig_{zodiac_id}.h/cpp")

    print(f"\n完成! 共生成:")
    print(f"  - 1 个动画状态枚举 (DBAAnimState)")
    print(f"  - 1 个通用动画配置 (DBAAnimConfig)")
    print(f"  - {count} 个生肖动画配置")
    print(f"\n动画资源目录结构:")
    print(f"  Montages: /Game/Animation/Zodiac/{{zodiac}}/Montages/")
    print(f"  Anim Blueprints: /Game/Animation/Zodiac/{{zodiac}}/ABP_{{zodiac}}")
    print(f"\n注意: 动画蓝图是UE编辑器资源,无法通过代码生成")
    print(f"      请在UE编辑器中创建动画蓝图并引用对应的Montage资源")


if __name__ == "__main__":
    main()