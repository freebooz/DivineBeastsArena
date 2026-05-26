# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：导入大厅 HUD 原创参考纹理资源，供 WBP_DBA_LobbyPlayerHUD 与 C++ 兜底布局使用。
# - 修改提示：保持来源目录与目标目录稳定，避免蓝图和 C++ 纹理路径漂移。

import os
from pathlib import Path

import unreal


PROJECT_ROOT = Path(__file__).resolve().parents[2]
SOURCE_DIR = PROJECT_ROOT / "SourceAssets" / "UI" / "LobbyHUD"
DEST_DIR = "/Game/DBA/UI/Lobby/HUD/Textures"

TEXTURES = [
    "T_DBA_LobbyHUD_UnitFrame_512x192.png",
    "T_DBA_LobbyHUD_PlayerPortrait_Default_256.png",
    "T_DBA_LobbyHUD_PortraitFrame_256.png",
    "T_DBA_LobbyHUD_SkillBar_1024x160.png",
    "T_DBA_LobbyHUD_SkillSlot_128.png",
    "T_DBA_LobbyHUD_MinimapFrame_512.png",
]


def log(message):
    unreal.log(f"[ImportLobbyHUDTextures] {message}")


def fail(message):
    unreal.log_error(f"[ImportLobbyHUDTextures] {message}")
    raise RuntimeError(message)


def configure_texture(asset_path):
    texture = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not texture:
        fail(f"Imported texture could not be loaded: {asset_path}")

    for prop, value in [
        ("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI),
        ("srgb", True),
        ("never_stream", True),
    ]:
        try:
            texture.set_editor_property(prop, value)
        except Exception as exc:
            log(f"Could not set {prop} on {asset_path}: {exc}")

    try:
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DEFAULT)
    except Exception as exc:
        log(f"Could not set compression_settings on {asset_path}: {exc}")

    unreal.EditorAssetLibrary.save_loaded_asset(texture, only_if_is_dirty=False)


def main():
    if not SOURCE_DIR.exists():
        fail(f"Source directory does not exist: {SOURCE_DIR}")

    if not unreal.EditorAssetLibrary.does_directory_exist(DEST_DIR):
        unreal.EditorAssetLibrary.make_directory(DEST_DIR)

    missing_texture = None
    for filename in TEXTURES:
        source_path = SOURCE_DIR / filename
        if not os.path.exists(source_path):
            fail(f"Source image does not exist: {source_path}")

        asset_name = Path(filename).stem
        asset_path = f"{DEST_DIR}/{asset_name}"
        if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            missing_texture = filename
            break

    if missing_texture:
        source_path = SOURCE_DIR / missing_texture
        task = unreal.AssetImportTask()
        task.filename = str(source_path)
        task.destination_path = DEST_DIR
        task.destination_name = Path(missing_texture).stem
        task.automated = True
        task.replace_existing = False
        task.replace_existing_settings = False
        task.save = True
        log(f"Importing one missing texture: {missing_texture}")
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    for filename in TEXTURES:
        asset_name = Path(filename).stem
        asset_path = f"{DEST_DIR}/{asset_name}"
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            configure_texture(asset_path)
            log(f"Configured {asset_path}")
        elif not missing_texture:
            fail(f"Import failed: {asset_path}")


if __name__ == "__main__":
    main()
