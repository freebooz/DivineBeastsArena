#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
生成技能投射物类脚本

功能:
- 为十二生肖技能的投射物生成具体类
- 投射物类用于弹道类技能

用法:
    python generate_skill_projectile_scripts.py
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

# 投射物技能类型 (Q/W/E技能中需要投射物的)
PROJECTILE_SKILLS = [
    ("Q", "技能Q"),
    ("E", "技能E"),
]


def generate_projectile(zodiac_id, zodiac_cn, skill_id, skill_name):
    """生成投射物类代码"""

    header = f"""// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能投射物 - {zodiac_cn}{skill_name}

#pragma once

#include "CoreMinimal.h"
#include "DBAProjectile_{zodiac_id}_{skill_id}.generated.h"
#include "DBA/Combat/DBASkillProjectileBase.h"

/**
 * ADBAProjectile_{zodiac_id}_{skill_id}
 * {zodiac_cn}{skill_name}投射物
 * 用于{zodiac_cn}的{skill_name}技能
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAProjectile_{zodiac_id}_{skill_id} : public ADBASkillProjectileBase
{{
	GENERATED_BODY()

public:
	ADBAProjectile_{zodiac_id}_{skill_id}();

protected:
	virtual void BeginPlay() override;

public:
	/** 初始化投射物 */
	virtual void InitializeProjectile(
		FName InSkillId,
		AActor* InOwner,
		AActor* InTarget,
		float InDamage,
		float InSpeed,
		float InRadius) override;
}};
"""

    cpp = f"""// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能投射物 - {zodiac_cn}{skill_name}

#include "Combat/Projectiles/DBAProjectile_{zodiac_id}_{skill_id}.h"
#include "Components/SphereComponent.h"

ADBAProjectile_{zodiac_id}_{skill_id}::ADBAProjectile_{zodiac_id}_{skill_id}()
{{
	// 设置投射物属性
	Speed = 1200.0f;
	Radius = 30.0f;
	Damage = 50.0f;

	// 设置特效资源路径
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ProjectileVFXFinder(
		TEXT("/Game/VFX/Projectiles/{zodiac_id}/P_{zodiac_id}_{skill_id}_Projectile.P_{zodiac_id}_{skill_id}_Projectile"));
	if (ProjectileVFXFinder.Succeeded())
	{{
		ProjectileVFXAsset = ProjectileVFXFinder.Object;
	}}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ImpactVFXFinder(
		TEXT("/Game/VFX/Projectiles/{zodiac_id}/P_{zodiac_id}_{skill_id}_Impact.P_{zodiac_id}_{skill_id}_Impact"));
	if (ImpactVFXFinder.Succeeded())
	{{
		ImpactVFXAsset = ImpactVFXFinder.Object;
	}}

	static ConstructorHelpers::FObjectFinder<USoundCue> FlySFXFinder(
		TEXT("/Game/Audio/SFX/Projectiles/{zodiac_id}/S_{zodiac_id}_{skill_id}_Fly.S_{zodiac_id}_{skill_id}_Fly"));
	if (FlySFXFinder.Succeeded())
	{{
		FlySFXAsset = FlySFXFinder.Object;
	}}

	static ConstructorHelpers::FObjectFinder<USoundCue> ImpactSFXFinder(
		TEXT("/Game/Audio/SFX/Projectiles/{zodiac_id}/S_{zodiac_id}_{skill_id}_Impact.S_{zodiac_id}_{skill_id}_Impact"));
	if (ImpactSFXFinder.Succeeded())
	{{
		ImpactSFXAsset = ImpactSFXFinder.Object;
	}}
}}

void ADBAProjectile_{zodiac_id}_{skill_id}::BeginPlay()
{{
	Super::BeginPlay();
}}

void ADBAProjectile_{zodiac_id}_{skill_id}::InitializeProjectile(
	FName InSkillId,
	AActor* InOwner,
	AActor* InTarget,
	float InDamage,
	float InSpeed,
	float InRadius)
{{
	SkillId = InSkillId;
	ProjectileOwner = InOwner;
	TargetActor = InTarget;
	Damage = InDamage;
	Speed = InSpeed;
	Radius = InRadius;

	// 更新移动组件
	if (ProjectileMovement)
	{{
		ProjectileMovement->InitialSpeed = Speed;
		ProjectileMovement->MaxSpeed = Speed * 1.5f;
	}}

	// 更新碰撞半径
	if (USphereComponent* Sphere = Cast<USphereComponent>(RootComponent))
	{{
		Sphere->SetSphereRadius(Radius);
	}}

	// 加载飞行特效
	if (ProjectileVFXAsset.IsValid())
	{{
		if (UParticleSystem* VFX = ProjectileVFXAsset.LoadSynchronous())
		{{
			ProjectileVFX->SetTemplate(VFX);
		}}
	}}

	// 设置初始速度方向朝向目标
	if (InTarget)
	{{
		FVector Direction = (InTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		ProjectileMovement->Velocity = Direction * Speed;
	}}
}}
"""

    return {"header": header, "cpp": cpp}


def main():
    project_root = Path(__file__).parent.parent / "Source" / "DivineBeastsArena"

    # 投射物目录
    projectile_public_dir = project_root / "Public" / "Combat" / "Projectiles"
    projectile_private_dir = project_root / "Private" / "Combat" / "Projectiles"
    projectile_public_dir.mkdir(parents=True, exist_ok=True)
    projectile_private_dir.mkdir(parents=True, exist_ok=True)

    count = 0

    for zodiac_id, zodiac_cn in ZODIAC_TYPES:
        for skill_id, skill_name in PROJECTILE_SKILLS:
            code = generate_projectile(zodiac_id, zodiac_cn, skill_id, skill_name)

            header_path = projectile_public_dir / f"DBAProjectile_{zodiac_id}_{skill_id}.h"
            cpp_path = projectile_private_dir / f"DBAProjectile_{zodiac_id}_{skill_id}.cpp"

            with open(header_path, "w", encoding="utf-8") as f:
                f.write(code["header"])
            with open(cpp_path, "w", encoding="utf-8") as f:
                f.write(code["cpp"])

            count += 1
            print(f"  生成: DBAProjectile_{zodiac_id}_{skill_id}.h/cpp")

    print(f"\n完成! 共生成:")
    print(f"  - {count} 个投射物类")
    print(f"\n资源目录结构:")
    print(f"  Projectile VFX: /Game/VFX/Projectiles/{{zodiac}}/")
    print(f"  Projectile SFX: /Game/Audio/SFX/Projectiles/{{zodiac}}/")


if __name__ == "__main__":
    main()