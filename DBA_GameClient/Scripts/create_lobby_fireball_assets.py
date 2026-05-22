#!/usr/bin/env python3
# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


VFX_DIR = "/Game/DBA/VFX/Fireball"
BP_DIR = "/Game/DBA/Blueprints/Projectiles"
PROJECTILE_SYSTEM = f"{VFX_DIR}/NS_DBA_Fireball_Projectile"
IMPACT_SYSTEM = f"{VFX_DIR}/NS_DBA_Fireball_Impact"
FIREBALL_BP = f"{BP_DIR}/BP_DBA_FireballProjectile"


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def duplicate_if_missing(source, dest):
    if unreal.EditorAssetLibrary.does_asset_exist(dest):
        return unreal.EditorAssetLibrary.load_asset(dest)

    asset = unreal.EditorAssetLibrary.duplicate_asset(source, dest)
    if not asset:
        unreal.log_warning(f"Failed to duplicate asset: {source} -> {dest}")
        return None

    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    unreal.log(f"Created asset: {dest}")
    return asset


def ensure_blueprint():
    if unreal.EditorAssetLibrary.does_asset_exist(FIREBALL_BP):
        return unreal.EditorAssetLibrary.load_asset(FIREBALL_BP)

    parent_class = unreal.load_class(None, "/Script/DivineBeastsArena.DBAFireballProjectile")
    if not parent_class:
        unreal.log_error("Unable to load ADBAFireballProjectile class.")
        return None

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    blueprint = asset_tools.create_asset(
        "BP_DBA_FireballProjectile",
        BP_DIR,
        unreal.Blueprint,
        factory,
    )
    if blueprint:
        unreal.EditorAssetLibrary.save_loaded_asset(blueprint)
        unreal.log(f"Created fireball projectile blueprint: {FIREBALL_BP}")
    return blueprint


def configure_blueprint_defaults(blueprint, projectile_system, impact_system):
    if not blueprint:
        return

    # The native parent class already points at the generated Niagara assets.
    # Keeping the child blueprint thin makes it easy for designers to tune later.
    try:
        unreal.KismetCompilerLibrary.compile_blueprint(blueprint)
    except Exception:
        pass
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint)


def main():
    ensure_dir(VFX_DIR)
    ensure_dir(BP_DIR)

    projectile_system = duplicate_if_missing(
        "/Niagara/DefaultAssets/Templates/Systems/FountainLightweight",
        PROJECTILE_SYSTEM,
    )
    impact_system = duplicate_if_missing(
        "/Niagara/DefaultAssets/Templates/Systems/SimpleExplosion",
        IMPACT_SYSTEM,
    )
    blueprint = ensure_blueprint()
    configure_blueprint_defaults(blueprint, projectile_system, impact_system)
    unreal.log("Lobby fireball Niagara systems and projectile blueprint are ready.")


if __name__ == "__main__":
    main()
