#!/usr/bin/env python3
import sys
import unreal


REQUIRED_ASSETS = [
    "/Game/DBA/VFX/Fireball/NS_DBA_Fireball_Projectile",
    "/Game/DBA/VFX/Fireball/NS_DBA_Fireball_Impact",
    "/Game/DBA/Blueprints/Projectiles/BP_DBA_FireballProjectile",
]


def main():
    ok = True
    for path in REQUIRED_ASSETS:
        asset = unreal.EditorAssetLibrary.load_asset(path)
        if asset:
            unreal.log(f"Loaded required fireball asset: {path} ({asset.get_class().get_name()})")
        else:
            unreal.log_error(f"Missing required fireball asset: {path}")
            ok = False

    fireball_class = unreal.load_class(
        None,
        "/Game/DBA/Blueprints/Projectiles/BP_DBA_FireballProjectile.BP_DBA_FireballProjectile_C",
    )
    if fireball_class:
        unreal.log(f"Loaded fireball projectile blueprint class: {fireball_class.get_name()}")
    else:
        unreal.log_error("Unable to load BP_DBA_FireballProjectile generated class.")
        ok = False

    native_class = unreal.load_class(None, "/Script/DivineBeastsArena.DBAFireballProjectile")
    if native_class:
        unreal.log(f"Loaded native fireball parent class: {native_class.get_name()}")
    else:
        unreal.log_error("Unable to load native ADBAFireballProjectile class.")
        ok = False

    if not ok:
        sys.exit(1)
    unreal.log("Lobby fireball validation passed.")


if __name__ == "__main__":
    main()
