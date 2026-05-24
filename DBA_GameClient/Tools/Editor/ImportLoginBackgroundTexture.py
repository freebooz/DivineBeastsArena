# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：项目工具脚本，用于资产整理、模拟服务或本地开发辅助流程。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import os
from pathlib import Path
import unreal


PROJECT_ROOT = Path(__file__).resolve().parents[2]
SOURCE_PNG = str(PROJECT_ROOT / "SourceAssets" / "UI" / "LoginCover" / "T_DBA_LoginArenaCover_Background_2048x1152.png")
DEST_DIR = "/Game/DBA/UI/Lobby/Login/Textures"
ASSET_NAME = "T_DBA_LoginBackground_Custom"


def log(message):
    unreal.log(f"[ImportLoginBackgroundTexture] {message}")


def main():
    if not os.path.exists(SOURCE_PNG):
        raise RuntimeError(f"Source image does not exist: {SOURCE_PNG}")

    if not unreal.EditorAssetLibrary.does_directory_exist(DEST_DIR):
        unreal.EditorAssetLibrary.make_directory(DEST_DIR)

    task = unreal.AssetImportTask()
    task.filename = SOURCE_PNG
    task.destination_path = DEST_DIR
    task.destination_name = ASSET_NAME
    task.automated = True
    task.replace_existing = True
    task.replace_existing_settings = True
    task.save = True

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    asset_path = f"{DEST_DIR}/{ASSET_NAME}"
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        raise RuntimeError(f"Import failed: {asset_path}")

    texture = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not texture:
        raise RuntimeError(f"Imported texture could not be loaded: {asset_path}")

    try:
        texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    except Exception as exc:
        log(f"Could not set lod_group: {exc}")

    try:
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DEFAULT)
    except Exception as exc:
        log(f"Could not set compression_settings: {exc}")

    unreal.EditorAssetLibrary.save_loaded_asset(texture, only_if_is_dirty=False)
    log(f"Imported {SOURCE_PNG} as {asset_path}.{ASSET_NAME}")


if __name__ == "__main__":
    main()
