#!/usr/bin/env python3
# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

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
