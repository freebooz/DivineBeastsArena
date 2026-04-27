# Copyright Freebooz Studio. All Rights Reserved.
#
# AssignAbilityVFXSFX.py
#
# 将 Niagara VFX / SoundWave SFX 自动写入 AbilitySet DataAsset
#
# 执行方式:
# py D:/DivineBeastsArena/Scripts/AssignAbilityVFXSFX.py
#
# 前置条件:
# 1. 已经运行 GenerateVFXSFXAssets.py，生成 VFX / SFX 资源。
# 2. UDBAAbilitySetDataAsset 已增加以下属性之一：
#
#    强类型:
#       CastVFX, ImpactVFX, LoopVFX
#       CastSFX, ImpactSFX, LoopSFX
#
#    Python 属性名通常为:
#       cast_vfx, impact_vfx, loop_vfx
#       cast_sfx, impact_sfx, loop_sfx
#
# 3. 已经创建 /Game/DBA/AbilitySets/DA_AS_* 数据资产。
#

import unreal


CONTENT_PATH = "/Game"


# ============================================
# 日志
# ============================================

def log_info(msg):
    unreal.log("[AssignAbilityVFXSFX][INFO] {}".format(msg))


def log_warn(msg):
    unreal.log_warning("[AssignAbilityVFXSFX][WARN] {}".format(msg))


def log_error(msg):
    unreal.log_error("[AssignAbilityVFXSFX][ERROR] {}".format(msg))


# ============================================
# 工具
# ============================================

def load_asset(path):
    if not path:
        return None

    try:
        asset = unreal.EditorAssetLibrary.load_asset(path)
        if not asset:
            log_warn("加载资产失败: {}".format(path))
        return asset
    except Exception as e:
        log_warn("加载资产异常: {} - {}".format(path, str(e)))
        return None


def set_prop(asset, prop_name, value):
    if not asset:
        return False

    try:
        asset.set_editor_property(prop_name, value)
        log_info("设置属性成功: {}.{}".format(asset.get_name(), prop_name))
        return True

    except Exception as e:
        log_warn("设置属性失败: {}.{} - {}".format(asset.get_name(), prop_name, str(e)))
        return False


def save_asset(asset, path):
    if not asset:
        return False

    try:
        saved = unreal.EditorAssetLibrary.save_loaded_asset(
            asset,
            only_if_is_dirty=False
        )

        if saved:
            log_info("保存资产成功: {}".format(path))
            return True

        log_error("保存资产失败: {}".format(path))
        return False

    except Exception as e:
        log_error("保存资产异常: {} - {}".format(path, str(e)))
        return False


def ability_path(asset_name):
    return "{}/DBA/AbilitySets/{}".format(CONTENT_PATH, asset_name)


def vfx_ability(hero, asset_name):
    return "{}/DBA/VFX/Abilities/{}/{}".format(CONTENT_PATH, hero, asset_name)


def vfx_common(category, asset_name):
    return "{}/DBA/VFX/Common/{}/{}".format(CONTENT_PATH, category, asset_name)


def sfx_ability(hero, asset_name):
    return "{}/DBA/Audio/SFX/Abilities/{}/{}".format(CONTENT_PATH, hero, asset_name)


def sfx_common(category, asset_name):
    return "{}/DBA/Audio/SFX/Common/{}/{}".format(CONTENT_PATH, category, asset_name)


def try_load_ref(path):
    if not path:
        return None

    asset = load_asset(path)

    if asset:
        return asset

    return None


def assign_refs(row):
    ability_asset_name = row["ability"]
    ability_asset_path = ability_path(ability_asset_name)

    ability_asset = load_asset(ability_asset_path)

    if not ability_asset:
        log_error("AbilitySet 不存在，跳过: {}".format(ability_asset_path))
        return False

    log_info("=" * 80)
    log_info("开始写入 VFX/SFX: {}".format(ability_asset_path))
    log_info("=" * 80)

    cast_vfx = try_load_ref(row.get("cast_vfx", ""))
    impact_vfx = try_load_ref(row.get("impact_vfx", ""))
    loop_vfx = try_load_ref(row.get("loop_vfx", ""))

    cast_sfx = try_load_ref(row.get("cast_sfx", ""))
    impact_sfx = try_load_ref(row.get("impact_sfx", ""))
    loop_sfx = try_load_ref(row.get("loop_sfx", ""))

    if cast_vfx:
        set_prop(ability_asset, "cast_vfx", cast_vfx)

    if impact_vfx:
        set_prop(ability_asset, "impact_vfx", impact_vfx)

    if loop_vfx:
        set_prop(ability_asset, "loop_vfx", loop_vfx)

    if cast_sfx:
        set_prop(ability_asset, "cast_sfx", cast_sfx)

    if impact_sfx:
        set_prop(ability_asset, "impact_sfx", impact_sfx)

    if loop_sfx:
        set_prop(ability_asset, "loop_sfx", loop_sfx)

    save_asset(ability_asset, ability_asset_path)
    return True


# ============================================
# 对应关系表
# ============================================

ABILITY_VFX_SFX_MAP = [
    {
        "ability": "DA_AS_FireLion_Passive",
        "impact_vfx": vfx_common("Status", "NS_Status_Burning"),
        "loop_vfx": vfx_ability("FireLion", "NS_FireLion_Passive_BurnAura"),
        "loop_sfx": sfx_ability("FireLion", "SFX_FireLion_Passive_BurnAura"),
    },
    {
        "ability": "DA_AS_FireLion_Q",
        "cast_vfx": vfx_ability("FireLion", "NS_FireLion_Q_FlameClaw_Slash"),
        "impact_vfx": vfx_ability("FireLion", "NS_FireLion_Q_FlameClaw_Impact"),
        "cast_sfx": sfx_ability("FireLion", "SFX_FireLion_Q_FlameClaw_Slash"),
        "impact_sfx": sfx_ability("FireLion", "SFX_FireLion_Q_FlameClaw_Impact"),
    },
    {
        "ability": "DA_AS_FireLion_W",
        "cast_vfx": vfx_ability("FireLion", "NS_FireLion_W_RoarShield"),
        "impact_vfx": vfx_common("Status", "NS_Status_Shielded"),
        "cast_sfx": sfx_ability("FireLion", "SFX_FireLion_W_RoarShield"),
        "impact_sfx": sfx_common("Status", "SFX_Status_Shielded"),
    },
    {
        "ability": "DA_AS_FireLion_E",
        "cast_vfx": vfx_ability("FireLion", "NS_FireLion_E_FlameLeap_Trail"),
        "impact_vfx": vfx_ability("FireLion", "NS_FireLion_E_FlameLeap_Impact"),
        "cast_sfx": sfx_ability("FireLion", "SFX_FireLion_E_FlameLeap_Jump"),
        "impact_sfx": sfx_ability("FireLion", "SFX_FireLion_E_FlameLeap_Impact"),
    },
    {
        "ability": "DA_AS_FireLion_R",
        "cast_vfx": vfx_ability("FireLion", "NS_FireLion_R_DivineBeastTransform"),
        "loop_vfx": vfx_ability("FireLion", "NS_FireLion_Passive_BurnAura"),
        "cast_sfx": sfx_ability("FireLion", "SFX_FireLion_R_DivineBeastTransform"),
        "loop_sfx": sfx_ability("FireLion", "SFX_FireLion_Passive_BurnAura"),
    },

    {
        "ability": "DA_AS_WaterDragon_Passive",
        "impact_vfx": vfx_common("Status", "NS_Status_Wet"),
        "loop_vfx": vfx_ability("WaterDragon", "NS_WaterDragon_Passive_WaterBreathAura"),
        "impact_sfx": sfx_common("Status", "SFX_Status_Wet"),
        "loop_sfx": sfx_ability("WaterDragon", "SFX_WaterDragon_Passive_WaterBreathAura"),
    },
    {
        "ability": "DA_AS_WaterDragon_Q",
        "cast_vfx": vfx_ability("WaterDragon", "NS_WaterDragon_Q_WaterBlast_Projectile"),
        "impact_vfx": vfx_ability("WaterDragon", "NS_WaterDragon_Q_WaterBlast_Impact"),
        "cast_sfx": sfx_ability("WaterDragon", "SFX_WaterDragon_Q_WaterBlast_Cast"),
        "impact_sfx": sfx_ability("WaterDragon", "SFX_WaterDragon_Q_WaterBlast_Impact"),
    },

    {
        "ability": "DA_AS_EarthBear_Passive",
        "impact_vfx": vfx_common("Status", "NS_Status_Shielded"),
        "loop_vfx": vfx_ability("EarthBear", "NS_EarthBear_Passive_RockArmor"),
        "impact_sfx": sfx_common("Status", "SFX_Status_Shielded"),
        "loop_sfx": sfx_ability("EarthBear", "SFX_EarthBear_Passive_RockArmor"),
    },
    {
        "ability": "DA_AS_EarthBear_Q",
        "cast_vfx": vfx_ability("EarthBear", "NS_EarthBear_Q_GroundSlam_Crack"),
        "impact_vfx": vfx_ability("EarthBear", "NS_EarthBear_Q_GroundSlam_Impact"),
        "cast_sfx": sfx_ability("EarthBear", "SFX_EarthBear_Q_GroundSlam_Cast"),
        "impact_sfx": sfx_ability("EarthBear", "SFX_EarthBear_Q_GroundSlam_Impact"),
    },

    {
        "ability": "DA_AS_GoldPhoenix_Passive",
        "impact_vfx": vfx_common("Impact", "NS_Impact_Heal_Burst"),
        "loop_vfx": vfx_ability("GoldPhoenix", "NS_GoldPhoenix_Passive_RebirthFeathers"),
        "impact_sfx": sfx_common("Impact", "SFX_Impact_Heal_Burst"),
        "loop_sfx": sfx_ability("GoldPhoenix", "SFX_GoldPhoenix_Passive_Rebirth"),
    },
    {
        "ability": "DA_AS_GoldPhoenix_Q",
        "cast_vfx": vfx_ability("GoldPhoenix", "NS_GoldPhoenix_Q_GoldFeather_Projectile"),
        "impact_vfx": vfx_ability("GoldPhoenix", "NS_GoldPhoenix_Q_GoldFeather_Impact"),
        "cast_sfx": sfx_ability("GoldPhoenix", "SFX_GoldPhoenix_Q_GoldFeather_Cast"),
        "impact_sfx": sfx_ability("GoldPhoenix", "SFX_GoldPhoenix_Q_GoldFeather_Impact"),
    },

    {
        "ability": "DA_AS_WoodCrane_Passive",
        "impact_vfx": vfx_common("Status", "NS_Status_HealingOverTime"),
        "loop_vfx": vfx_ability("WoodCrane", "NS_WoodCrane_Passive_LifePulse"),
        "impact_sfx": sfx_common("Status", "SFX_Status_HealingOverTime"),
        "loop_sfx": sfx_ability("WoodCrane", "SFX_WoodCrane_Passive_LifePulse"),
    },
    {
        "ability": "DA_AS_WoodCrane_Q",
        "cast_vfx": vfx_ability("WoodCrane", "NS_WoodCrane_Q_HealingSeed_Projectile"),
        "impact_vfx": vfx_ability("WoodCrane", "NS_WoodCrane_Q_HealingBurst_Impact"),
        "loop_vfx": vfx_ability("WoodCrane", "NS_WoodCrane_Q_HealingGrove_Area"),
        "cast_sfx": sfx_ability("WoodCrane", "SFX_WoodCrane_Q_HealingSeed_Cast"),
        "impact_sfx": sfx_ability("WoodCrane", "SFX_WoodCrane_Q_HealingBurst_Impact"),
        "loop_sfx": sfx_ability("WoodCrane", "SFX_WoodCrane_Q_HealingGrove_Area"),
    },
]


# ============================================
# 汇总检查
# ============================================

def print_summary():
    log_info("=" * 80)
    log_info("AbilitySet VFX/SFX 引用检查")
    log_info("=" * 80)

    for row in ABILITY_VFX_SFX_MAP:
        ability_asset_path = ability_path(row["ability"])
        exists = unreal.EditorAssetLibrary.does_asset_exist(ability_asset_path)

        log_info("[{}] {}".format("存在" if exists else "缺失", ability_asset_path))

        for key in ["cast_vfx", "impact_vfx", "loop_vfx", "cast_sfx", "impact_sfx", "loop_sfx"]:
            path = row.get(key, "")
            if path:
                ref_exists = unreal.EditorAssetLibrary.does_asset_exist(path)
                log_info("  - {} [{}] {}".format(key, "存在" if ref_exists else "缺失", path))

    log_info("=" * 80)


# ============================================
# 主函数
# ============================================

def main():
    log_info("=" * 80)
    log_info("开始写入 AbilitySet VFX / SFX 引用")
    log_info("=" * 80)

    success_count = 0

    for row in ABILITY_VFX_SFX_MAP:
        if assign_refs(row):
            success_count += 1

    print_summary()

    log_info("=" * 80)
    log_info("写入完成，成功处理 AbilitySet 数量: {}".format(success_count))
    log_info("=" * 80)


if __name__ == "__main__":
    main()
