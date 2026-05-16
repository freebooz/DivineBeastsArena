import unreal

ASSET_DIR = "/Game/DBA/UI/Lobby/HUD"
ASSET_NAME = "WBP_DBA_LobbyPlayerHUD"
ASSET_PATH = f"{ASSET_DIR}/{ASSET_NAME}"
PARENT_CLASS_PATH = "/Script/DivineBeastsArena.DBALobbyPlayerHUDWidgetBase"


def log(message):
    unreal.log(f"[CreateLobbyHUDWidgetBlueprint] {message}")


def fail(message):
    unreal.log_error(f"[CreateLobbyHUDWidgetBlueprint] {message}")
    raise RuntimeError(message)


def main():
    parent_class = unreal.load_object(None, PARENT_CLASS_PATH)
    if not parent_class:
        fail(f"Unable to load parent class: {PARENT_CLASS_PATH}")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    editor_asset_lib = unreal.EditorAssetLibrary

    if not editor_asset_lib.does_directory_exist(ASSET_DIR):
        editor_asset_lib.make_directory(ASSET_DIR)
        log(f"Created directory: {ASSET_DIR}")

    asset = editor_asset_lib.load_asset(ASSET_PATH)
    if asset:
        log(f"Asset already exists: {ASSET_PATH}")

    if not asset:
        factory = unreal.WidgetBlueprintFactory()
        factory.set_editor_property("parent_class", parent_class)
        asset = asset_tools.create_asset(
            asset_name=ASSET_NAME,
            package_path=ASSET_DIR,
            asset_class=unreal.WidgetBlueprint,
            factory=factory,
        )
        if not asset:
            fail(f"Failed to create widget blueprint: {ASSET_PATH}")
        log(f"Created widget blueprint: {ASSET_PATH}")

    generated_class = asset.generated_class() if hasattr(asset, "generated_class") else None
    log(f"GeneratedClass={generated_class}")

    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    log(f"Saved asset: {ASSET_PATH}")


main()
