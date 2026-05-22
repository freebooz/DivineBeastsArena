# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal

MAP_PATH = "/Game/Maps/Lobby/LobbyMap"
BP_PACKAGE_PATH = "/Game/DBA/UI/Lobby/Character"
BP_ASSET_NAME = "BP_DBA_CharacterPresentationActor"
NATIVE_CLASS_PATH = "/Script/DivineBeastsArena.DBACharacterPresentationActor"


def log(msg: str) -> None:
    unreal.log(f"[EnsurePresentationActor] {msg}")


def fail(msg: str) -> None:
    unreal.log_error(f"[EnsurePresentationActor] {msg}")
    raise RuntimeError(msg)


def ensure_blueprint_asset(parent_class: unreal.Class) -> unreal.Object:
    asset_path = f"{BP_PACKAGE_PATH}/{BP_ASSET_NAME}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        bp = unreal.EditorAssetLibrary.load_asset(asset_path)
        if not bp:
            fail(f"Blueprint exists but failed to load: {asset_path}")
        log(f"Blueprint already exists: {asset_path}")
        return bp

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("ParentClass", parent_class)
    bp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name=BP_ASSET_NAME,
        package_path=BP_PACKAGE_PATH,
        asset_class=unreal.Blueprint,
        factory=factory,
    )
    if not bp:
        fail(f"Failed to create blueprint: {asset_path}")

    unreal.EditorAssetLibrary.save_loaded_asset(bp)
    log(f"Blueprint created: {asset_path}")
    return bp


def ensure_actor_in_lobby_map(parent_class: unreal.Class) -> None:
    if not unreal.EditorLevelLibrary.load_level(MAP_PATH):
        fail(f"Failed to load map: {MAP_PATH}")
    log(f"Loaded map: {MAP_PATH}")

    existing = []
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        try:
            if actor.get_class() == parent_class:
                existing.append(actor)
        except Exception:
            continue

    if existing:
        log(f"Map already has presentation actor(s): {len(existing)}")
    else:
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            parent_class,
            unreal.Vector(0.0, 0.0, 0.0),
            unreal.Rotator(0.0, 0.0, 0.0),
        )
        if not actor:
            fail("Failed to spawn ADBACharacterPresentationActor in LobbyMap")
        actor.set_actor_label("DBA_CharacterPresentationStage")
        log("Spawned actor in map: DBA_CharacterPresentationStage")

    if not unreal.EditorLevelLibrary.save_current_level():
        fail("Failed to save current level after ensuring actor")
    log("Saved LobbyMap")


def main() -> None:
    parent_class = unreal.load_class(None, NATIVE_CLASS_PATH)
    if not parent_class:
        fail(f"Failed to load native class: {NATIVE_CLASS_PATH}")

    ensure_blueprint_asset(parent_class)
    ensure_actor_in_lobby_map(parent_class)
    log("Done")


if __name__ == "__main__":
    main()
