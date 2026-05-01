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
#include "Data/DBASkillDataRow.h"
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
	// 从技能数据表加载配置
	void LoadSkillData();

protected:
	// 特效峰值缩放
	UPROPERTY(EditDefaultsOnly, Category = "Cue")
	float CueScale = 1.0f;

	// 技能ID (用于查询数据)
	UPROPERTY(EditDefaultsOnly, Category = "Cue")
	FName SkillId = FName(TEXT("{zodiac}_{skill}"));
}};
"""

# GameplayCue cpp 文件模板
CUE_CPP_TEMPLATE = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayCue - {zodiac_cn}{skill_name}

#include "DBA/GAS/Cues/DBACue_{zodiac}_{skill}.h"
#include "DBA/GAS/DBAAbilitySystemComponent.h"
#include "Data/DBADataAsset.h"
#include "Kismet/GameplayStatics.h"

UDBACue_{zodiac}_{skill}::UDBACue_{zodiac}_{skill}()
{{
	// 默认配置
	bAutoDestroyOnOwnerRemoved = true;
	bOnlyRelevantToOwner = false;
	RollbackPolicy = ECueRollback::CanRollback;

	// 加载技能数据
	LoadSkillData();
}}

void UDBACue_{zodiac}_{skill}::LoadSkillData()
{{
	// 从 DataTable 加载技能数据
	UDataTable* SkillTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/Skills/SkillDataTable.SkillDataTable'"));
	if (SkillTable)
	{{
		static const FString ContextString = TEXT("DBACue_{zodiac}_{skill}");
		FDBASkillDataRow* SkillRow = SkillTable->FindRow<FDBASkillDataRow>(FName(TEXT("{zodiac}_{skill}")), ContextString, false);
		if (SkillRow)
		{{
			CueScale = SkillRow->BaseDamage > 0 ? 1.0f + SkillRow->DamageScaling * 0.1f : 1.0f;
		}}
	}}
}}

bool UDBACue_{zodiac}_{skill}::OnExecuteGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{{
	// 获取技能数据
	FDBASkillDataRow* SkillData = nullptr;
	UDataTable* SkillTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/Skills/SkillDataTable.SkillDataTable'"));
	if (SkillTable)
	{{
		static const FString ContextString = TEXT("OnExecuteGameplayCue");
		SkillData = SkillTable->FindRow<FDBASkillDataRow>(SkillId, ContextString, false);
	}}

	// 播放视觉效果
	if (SkillData && SkillData->VFXAsset.IsValid())
	{{
		UParticleSystem* VFX = SkillData->VFXAsset.LoadSynchronous();
		if (VFX)
		{{
			FVector Location = Target ? Target->GetActorLocation() : FVector::ZeroVector;
			FRotator Rotation = Target ? Target->GetActorRotation() : FRotator::ZeroRotator;
			UGameplayStatics::SpawnEmitterAtLocation(Target, VFX, Location, Rotation, CueScale);
		}}
	}}

	// 播放音效
	if (SkillData && SkillData->SFXAsset.IsValid())
	{{
		USoundCue* SFX = SkillData->SFXAsset.LoadSynchronous();
		if (SFX)
		{{
			UGameplayStatics::PlaySoundAtLocation(Target, SFX, Target ? Target->GetActorLocation() : FVector::ZeroVector);
		}}
	}}

	// 通过 ASC 广播技能事件
	if (AActor* Owner = GetOwningActor())
	{{
		if (UDBAAbilitySystemComponent* ASC = Owner->FindComponentByClass<UDBAAbilitySystemComponent>())
		{{
			ASC->OnSkillCueExecuted.Broadcast(SkillId, Target);
		}}
	}}

	return true;
}}

void UDBACue_{zodiac}_{skill}::OnActiveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{{
	// 获取技能数据，检查是否有引导特效
	FDBASkillDataRow* SkillData = nullptr;
	UDataTable* SkillTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/Skills/SkillDataTable.SkillDataTable'"));
	if (SkillTable)
	{{
		static const FString ContextString = TEXT("OnActiveGameplayCue");
		SkillData = SkillTable->FindRow<FDBASkillDataRow>(SkillId, ContextString, false);
	}}

	// 播放持续性特效（如引导、充能）
	if (SkillData && SkillData->VFXAsset.IsValid())
	{{
		UParticleSystem* VFX = SkillData->VFXAsset.LoadSynchronous();
		if (VFX)
		{{
			FVector Location = Target ? Target->GetActorLocation() : FVector::ZeroVector;
			FRotator Rotation = Target ? Target->GetActorRotation() : FRotator::ZeroRotator;
			UGameplayStatics::SpawnEmitterAtLocation(Target, VFX, Location, Rotation, CueScale * 0.7f);
		}}
	}}
}}

void UDBACue_{zodiac}_{skill}::OnRemoveGameplayCue(AActor* Target, FGameplayCueParameters& Parameters)
{{
	// 清理特效 - 在目标位置停止所有相关粒子系统
	if (Target)
	{{
		TArray<UParticleSystemComponent*> ParticleComponents;
		Target->GetComponents<UParticleSystemComponent>(ParticleComponents);
		for (UParticleSystemComponent* Particle : ParticleComponents)
		{{
			if (Particle && Particle->bIsActive)
			{{
				// 检查是否是此 Cue 创建的特效（通过 Tag 标记）
				if (Particle->ComponentHasTag(FName(TEXT("Cue_{zodiac}_{skill}"))))
				{{
					Particle->Deactivate();
				}}
			}}
		}}
	}}
}}
"""

# GameplayEffect 头文件模板
GE_HEADER_TEMPLATE = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayEffect - {zodiac_cn}{skill_name}

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Data/DBASkillDataRow.h"
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
#include "DBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "Engine/DataTable.h"
#include "Data/DBADataAsset.h"

UDBAGE_{zodiac}_{skill}::UDBAGE_{zodiac}_{skill}()
{{
	// 从技能数据表加载配置
	UDataTable* SkillTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/Skills/SkillDataTable.SkillDataTable'"));
	if (SkillTable)
	{{
		static const FString ContextString = TEXT("DBAGE_{zodiac}_{skill}");
		FDBASkillDataRow* SkillRow = SkillTable->FindRow<FDBASkillDataRow>(FName(TEXT("{zodiac}_{skill}")), ContextString, false);
		if (SkillRow)
		{{
			// 伤害修饰符
			if (SkillRow->BaseDamage > 0)
			{{
				FGameplayModifierInfo DamageMod;
				DamageMod.Attribute = UDBABattleAttributeSet::GetCurrentHealthAttribute();
				DamageMod.ModifierOp = EGameplayModOp::Additive;
				DamageMod.ModifierMagnitude = FScalableFloat(SkillRow->BaseDamage);
				Modifiers.Add(DamageMod);
			}}

			// 治疗修饰符
			if (SkillRow->HealAmount > 0)
			{{
				FGameplayModifierInfo HealMod;
				HealMod.Attribute = UDBABattleAttributeSet::GetCurrentHealthAttribute();
				HealMod.ModifierOp = EGameplayModOp::Additive;
				HealMod.ModifierMagnitude = FScalableFloat(SkillRow->HealAmount);
				Modifiers.Add(HealMod);
			}}

			// 护盾修饰符
			if (SkillRow->ShieldValue > 0)
			{{
				FGameplayModifierInfo ShieldMod;
				ShieldMod.Attribute = UDBABattleAttributeSet::GetCurrentHealthAttribute();
				ShieldMod.ModifierOp = EGameplayModOp::Additive;
				ShieldMod.ModifierMagnitude = FScalableFloat(SkillRow->ShieldValue);
				Modifiers.Add(ShieldMod);
			}}

			// 设置持续时间
			if (SkillRow->ControlTime > 0)
			{{
				DurationPolicy = EGameplayEffectDurationType::HasDuration;
				Period = 0.0f;
				DurationMagnitude = FScalableFloat(SkillRow->ControlTime);
			}}
		}}
	}}
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
    # 共鸣效果特殊模板
    resonance_header = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayEffect - {zodiac_cn}共鸣元素共鸣

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Data/DBAElementResonanceRow.h"
#include "DBAGE_{zodiac}_Resonance.generated.h"

UCLASS()
class DIVINEBEASTSARENA_API UDBAGE_{zodiac}_Resonance : public UGameplayEffect
{{
	GENERATED_BODY()

public:
	UDBAGE_{zodiac}_Resonance();
}};
"""

    resonance_cpp = """// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameplayEffect - {zodiac_cn}共鸣元素共鸣

#include "DBA/GAS/Effects/DBAGE_{zodiac}_Resonance.h"
#include "DBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "Engine/DataTable.h"

UDBAGE_{zodiac}_Resonance::UDBAGE_{zodiac}_Resonance()
{{
	// 从元素共鸣数据表加载配置
	UDataTable* ResonanceTable = LoadObject<UDataTable>(nullptr, TEXT("DataTable'/Game/Data/Elements/ElementResonanceTable.ElementResonanceTable'"));
	if (ResonanceTable)
	{{
		static const FString ContextString = TEXT("DBAGE_{zodiac}_Resonance");
		FDBADBElemenetResonanceRow* ResonanceRow = ResonanceTable->FindRow<FDBADBElemenetResonanceRow>(FName(TEXT("{zodiac}")), ContextString, false);
		if (ResonanceRow)
		{{
			// 控制时间加成
			if (ResonanceRow->ControlTimeBonus > 0)
			{{
				FGameplayModifierInfo ControlMod;
				ControlMod.Attribute = UDBABattleAttributeSet::GetCurrentHealthAttribute();
				ControlMod.ModifierOp = EGameplayModOp::Additive;
				ControlMod.ModifierMagnitude = FScalableFloat(ResonanceRow->ControlTimeBonus);
				Modifiers.Add(ControlMod);
			}}

			// 护盾值加成
			if (ResonanceRow->ShieldBonus > 0)
			{{
				FGameplayModifierInfo ShieldMod;
				ShieldMod.Attribute = UDBABattleAttributeSet::GetCurrentHealthAttribute();
				ShieldMod.ModifierOp = EGameplayModOp::Additive;
				ShieldMod.ModifierMagnitude = FScalableFloat(ResonanceRow->ShieldBonus);
				Modifiers.Add(ShieldMod);
			}}

			// 设置持续时间
			if (ResonanceRow->Duration > 0)
			{{
				DurationPolicy = EGameplayEffectDurationType::Infinite;
			}}
		}}
	}}
}}
"""

    for element_id, element_cn in ELEMENTS:
        header = resonance_header.format(
            zodiac_cn=element_cn,
            zodiac=element_id,
        )
        cpp = resonance_cpp.format(
            zodiac_cn=element_cn,
            zodiac=element_id,
        )
        header_path = ge_public_dir / f"DBAGE_{element_id}_Resonance.h"
        cpp_path = ge_private_dir / f"DBAGE_{element_id}_Resonance.cpp"
        with open(header_path, "w", encoding="utf-8") as f:
            f.write(header)
        with open(cpp_path, "w", encoding="utf-8") as f:
            f.write(cpp)
        print(f"  生成: DBAGE_{element_id}_Resonance")


if __name__ == "__main__":
    main()