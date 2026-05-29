#!/usr/bin/env python3
# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：在 Unreal Editor Python 环境中校验 P0 魔法技能 Niagara 资产脚手架。
# - 阅读重点：REQUIRED_ASSETS 应与 create_magic_vfx_skill_assets.py 的目标路径保持一致。
# - 修改提示：迁移 C++/DataAsset 默认路径前，先运行本脚本确认目标资产已经存在。

import sys
import unreal


REQUIRED_ASSETS = [
    "/Game/DBA/VFX/Skills/Mage/Fireball/NS_Fireball_Cast",
    "/Game/DBA/VFX/Skills/Mage/Fireball/NS_Fireball_Projectile",
    "/Game/DBA/VFX/Skills/Mage/Fireball/NS_Fireball_Impact",
    "/Game/DBA/VFX/Skills/Mage/Fireball/NS_Fireball_BurningStatus",
    "/Game/DBA/VFX/Skills/Mage/FrostBolt/NS_FrostBolt_Cast",
    "/Game/DBA/VFX/Skills/Mage/FrostBolt/NS_FrostBolt_Projectile",
    "/Game/DBA/VFX/Skills/Mage/FrostBolt/NS_FrostBolt_Impact",
    "/Game/DBA/VFX/Skills/Mage/FrostBolt/NS_FrostBolt_SlowStatus",
    "/Game/DBA/VFX/Skills/Druid/BloomHealing/NS_BloomHealing_Area",
    "/Game/DBA/VFX/Skills/Druid/BloomHealing/NS_BloomHealing_Target",
    "/Game/DBA/VFX/Skills/Druid/BloomHealing/NS_BloomHealing_Tick",
    "/Game/DBA/VFX/Skills/Shaman/ChainLightning/NS_ChainLightning_Cast",
    "/Game/DBA/VFX/Skills/Shaman/ChainLightning/NS_ChainLightning_Beam",
    "/Game/DBA/VFX/Skills/Shaman/ChainLightning/NS_ChainLightning_Impact",
    "/Game/DBA/VFX/Skills/Paladin/Sanctuary/NS_Sanctuary_Cast",
    "/Game/DBA/VFX/Skills/Paladin/Sanctuary/NS_Sanctuary_Shield",
    "/Game/DBA/VFX/Skills/Paladin/Sanctuary/NS_Sanctuary_Break",
    "/Game/DBA/VFX/Skills/Warlock/ShadowBolt/NS_ShadowBolt_Cast",
    "/Game/DBA/VFX/Skills/Warlock/ShadowBolt/NS_ShadowBolt_Projectile",
    "/Game/DBA/VFX/Skills/Warlock/ShadowBolt/NS_ShadowBolt_Impact",
]

REQUIRED_NATIVE_CLASSES = [
    "/Script/DivineBeastsArena.DBANiagaraSkillParameterLibrary",
    "/Script/DivineBeastsArena.DBAFireballProjectile",
    "/Script/DivineBeastsArena.DBAFrostShardProjectile",
    "/Script/DivineBeastsArena.DBAShadowBoltProjectile",
    "/Script/DivineBeastsArena.DBAChainLightningSpell",
    "/Script/DivineBeastsArena.DBABloomHealingSpell",
    "/Script/DivineBeastsArena.DBAHolyShieldSpell",
]


def validate_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        unreal.log_error(f"Missing P0 magic VFX asset: {path}")
        return False

    class_name = asset.get_class().get_name()
    unreal.log(f"Loaded P0 magic VFX asset: {path} ({class_name})")
    if "NiagaraSystem" not in class_name:
        unreal.log_warning(f"Asset is not a NiagaraSystem: {path} ({class_name})")
    return True


def validate_class(path):
    native_class = unreal.load_class(None, path)
    if not native_class:
        unreal.log_error(f"Missing native class: {path}")
        return False
    unreal.log(f"Loaded native class: {path}")
    return True


def main():
    ok = True

    for path in REQUIRED_ASSETS:
        ok = validate_asset(path) and ok

    for path in REQUIRED_NATIVE_CLASSES:
        ok = validate_class(path) and ok

    if not ok:
        sys.exit(1)

    unreal.log("P0 magic VFX validation passed.")


if __name__ == "__main__":
    main()
