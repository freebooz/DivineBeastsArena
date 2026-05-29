#!/usr/bin/env python3
# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：在 Unreal Editor Python 环境中创建 P0 魔法技能 Niagara 资产占位。
# - 阅读重点：ASSET_SPECS 定义目标路径和可复用源资产；main() 会按需创建目录和复制资产。
# - 修改提示：只做资产脚手架，不改变 C++ 默认引用；确认资产在编辑器内完成美术调校后再迁移运行时路径。

import sys
import unreal


P0_VFX_ROOT = "/Game/DBA/VFX/Skills"

SOURCE_ASSETS = {
    "fire_cast": "/Game/DBA/VFX/Abilities/FireLion/NS_FireLion_Q_FlameClaw_Slash",
    "fire_projectile": "/Game/DBA/VFX/Fireball/NS_DBA_Fireball_Projectile",
    "fire_impact": "/Game/DBA/VFX/Fireball/NS_DBA_Fireball_Impact",
    "burning_status": "/Game/DBA/VFX/Common/Status/NS_Status_Burning",
    "frost_cast": "/Game/ProjectileHitVFX/NS/NS_IceCrystal",
    "frost_projectile": "/Game/ProjectileHitVFX/NS/NS_IceDart",
    "frost_impact": "/Game/ProjectileHitVFX/NS/NS_Hit_Ice_01",
    "slow_status": "/Game/DBA/VFX/Common/Status/NS_Status_Slowed",
    "wood_area": "/Game/DBA/VFX/Abilities/WoodCrane/NS_WoodCrane_Q_HealingGrove_Area",
    "wood_target": "/Game/DBA/VFX/Abilities/WoodCrane/NS_WoodCrane_Q_HealingSeed_Projectile",
    "wood_tick": "/Game/DBA/VFX/Abilities/WoodCrane/NS_WoodCrane_Q_HealingBurst_Impact",
    "lightning_cast": "/Game/ProjectileHitVFX/NS/NS_Hit_Eletric_01",
    "lightning_beam": "/Game/ProjectileHitVFX/NS/NS_ThunderBolt",
    "lightning_impact": "/Game/ProjectileHitVFX/NS/NS_Hit_Thunder",
    "holy_cast": "/Game/ProjectileHitVFX/NS/NS_Hit_Bless",
    "holy_shield": "/Game/DBA/VFX/Common/Status/NS_Status_Shielded",
    "holy_break": "/Game/ProjectileHitVFX/NS/NS_HolyEnergy",
    "shadow_cast": "/Game/ProjectileHitVFX/NS/NS_Hit_Magic",
    "shadow_projectile": "/Game/ProjectileHitVFX/NS/NS_PoisonSkullFish",
    "shadow_impact": "/Game/ProjectileHitVFX/NS/NS_Hit_Poison",
    "template_projectile": "/Niagara/DefaultAssets/Templates/Systems/FountainLightweight",
    "template_impact": "/Niagara/DefaultAssets/Templates/Systems/SimpleExplosion",
}

ASSET_SPECS = [
    ("fire_cast", f"{P0_VFX_ROOT}/Mage/Fireball/NS_Fireball_Cast"),
    ("fire_projectile", f"{P0_VFX_ROOT}/Mage/Fireball/NS_Fireball_Projectile"),
    ("fire_impact", f"{P0_VFX_ROOT}/Mage/Fireball/NS_Fireball_Impact"),
    ("burning_status", f"{P0_VFX_ROOT}/Mage/Fireball/NS_Fireball_BurningStatus"),
    ("frost_cast", f"{P0_VFX_ROOT}/Mage/FrostBolt/NS_FrostBolt_Cast"),
    ("frost_projectile", f"{P0_VFX_ROOT}/Mage/FrostBolt/NS_FrostBolt_Projectile"),
    ("frost_impact", f"{P0_VFX_ROOT}/Mage/FrostBolt/NS_FrostBolt_Impact"),
    ("slow_status", f"{P0_VFX_ROOT}/Mage/FrostBolt/NS_FrostBolt_SlowStatus"),
    ("wood_area", f"{P0_VFX_ROOT}/Druid/BloomHealing/NS_BloomHealing_Area"),
    ("wood_target", f"{P0_VFX_ROOT}/Druid/BloomHealing/NS_BloomHealing_Target"),
    ("wood_tick", f"{P0_VFX_ROOT}/Druid/BloomHealing/NS_BloomHealing_Tick"),
    ("lightning_cast", f"{P0_VFX_ROOT}/Shaman/ChainLightning/NS_ChainLightning_Cast"),
    ("lightning_beam", f"{P0_VFX_ROOT}/Shaman/ChainLightning/NS_ChainLightning_Beam"),
    ("lightning_impact", f"{P0_VFX_ROOT}/Shaman/ChainLightning/NS_ChainLightning_Impact"),
    ("holy_cast", f"{P0_VFX_ROOT}/Paladin/Sanctuary/NS_Sanctuary_Cast"),
    ("holy_shield", f"{P0_VFX_ROOT}/Paladin/Sanctuary/NS_Sanctuary_Shield"),
    ("holy_break", f"{P0_VFX_ROOT}/Paladin/Sanctuary/NS_Sanctuary_Break"),
    ("shadow_cast", f"{P0_VFX_ROOT}/Warlock/ShadowBolt/NS_ShadowBolt_Cast"),
    ("shadow_projectile", f"{P0_VFX_ROOT}/Warlock/ShadowBolt/NS_ShadowBolt_Projectile"),
    ("shadow_impact", f"{P0_VFX_ROOT}/Warlock/ShadowBolt/NS_ShadowBolt_Impact"),
]


def asset_dir(asset_path):
    return asset_path.rsplit("/", 1)[0]


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)
        unreal.log(f"Created directory: {path}")


def resolve_source(source_key):
    preferred_path = SOURCE_ASSETS[source_key]
    if unreal.EditorAssetLibrary.does_asset_exist(preferred_path):
        return preferred_path

    fallback_key = "template_impact" if "impact" in source_key or "break" in source_key else "template_projectile"
    fallback_path = SOURCE_ASSETS[fallback_key]
    if unreal.EditorAssetLibrary.does_asset_exist(fallback_path):
        unreal.log_warning(f"Missing source {preferred_path}; using fallback {fallback_path}")
        return fallback_path

    unreal.log_error(f"Missing source and fallback for key={source_key}: {preferred_path}")
    return None


def duplicate_if_missing(source_key, dest_path):
    ensure_dir(asset_dir(dest_path))

    if unreal.EditorAssetLibrary.does_asset_exist(dest_path):
        unreal.log(f"Asset already exists: {dest_path}")
        return unreal.EditorAssetLibrary.load_asset(dest_path)

    source_path = resolve_source(source_key)
    if not source_path:
        return None

    asset = unreal.EditorAssetLibrary.duplicate_asset(source_path, dest_path)
    if not asset:
        unreal.log_error(f"Failed to duplicate asset: {source_path} -> {dest_path}")
        return None

    unreal.EditorAssetLibrary.set_metadata_tag(asset, "DBA.VFX.SourceAsset", source_path)
    unreal.EditorAssetLibrary.set_metadata_tag(asset, "DBA.VFX.Status", "GeneratedPlaceholder")
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    unreal.log(f"Created P0 VFX asset: {dest_path} from {source_path}")
    return asset


def main():
    created_or_existing = 0
    failed = []

    ensure_dir(P0_VFX_ROOT)
    for source_key, dest_path in ASSET_SPECS:
        asset = duplicate_if_missing(source_key, dest_path)
        if asset:
            created_or_existing += 1
        else:
            failed.append(dest_path)

    if failed:
        for path in failed:
            unreal.log_error(f"Missing generated P0 VFX asset: {path}")
        sys.exit(1)

    unreal.log(f"P0 magic VFX asset scaffold complete. Assets ready: {created_or_existing}")


if __name__ == "__main__":
    main()
