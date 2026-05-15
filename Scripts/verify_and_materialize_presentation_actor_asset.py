import unreal

PRIMARY_ASSET = "/Game/DBA/UI/Lobby/Character/BP_DBA_CharacterPresentationActor"
FALLBACK_DIR = "/Game/Blueprints/UI/Lobby"
FALLBACK_NAME = "BP_DBA_CharacterPresentationActor"
NATIVE_CLASS_PATH = "/Script/DivineBeastsArena.DBACharacterPresentationActor"


def log(msg: str) -> None:
    unreal.log(f"[VerifyPresentationActorAsset] {msg}")


def ensure_dir(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)
        log(f"Created directory: {path}")


def ensure_bp(path_dir: str, name: str, parent_class: unreal.Class) -> str:
    asset_path = f"{path_dir}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if asset:
            unreal.EditorAssetLibrary.save_loaded_asset(asset)
            log(f"Asset exists: {asset_path}")
            return asset_path

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("ParentClass", parent_class)
    bp = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name=name,
        package_path=path_dir,
        asset_class=unreal.Blueprint,
        factory=factory,
    )
    if not bp:
        raise RuntimeError(f"Failed to create blueprint asset: {asset_path}")
    unreal.EditorAssetLibrary.save_loaded_asset(bp)
    log(f"Created asset: {asset_path}")
    return asset_path


def main() -> None:
    parent_class = unreal.load_class(None, NATIVE_CLASS_PATH)
    if not parent_class:
        raise RuntimeError(f"Failed to load native class: {NATIVE_CLASS_PATH}")

    primary_exists = unreal.EditorAssetLibrary.does_asset_exist(PRIMARY_ASSET)
    log(f"Primary exists before: {primary_exists}")
    if primary_exists:
        a = unreal.EditorAssetLibrary.load_asset(PRIMARY_ASSET)
        log(f"Primary load: {'OK' if a else 'FAILED'}")
    else:
        ensure_bp("/Game/DBA/UI/Lobby/Character", "BP_DBA_CharacterPresentationActor", parent_class)

    ensure_dir(FALLBACK_DIR)
    fallback_asset = ensure_bp(FALLBACK_DIR, FALLBACK_NAME, parent_class)

    assets = unreal.EditorAssetLibrary.list_assets("/Game/DBA/UI/Lobby/Character", recursive=False, include_folder=False)
    log(f"Assets under /Game/DBA/UI/Lobby/Character: {assets}")
    assets2 = unreal.EditorAssetLibrary.list_assets(FALLBACK_DIR, recursive=False, include_folder=False)
    log(f"Assets under {FALLBACK_DIR}: {assets2}")

    # Save all dirty packages to maximize editor visibility after restart.
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    log(f"Final primary asset: {PRIMARY_ASSET}")
    log(f"Final fallback asset: {fallback_asset}")


if __name__ == "__main__":
    main()
