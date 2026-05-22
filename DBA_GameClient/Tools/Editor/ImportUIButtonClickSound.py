# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：项目工具脚本，用于资产整理、模拟服务或本地开发辅助流程。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import os
import unreal


SOURCE_WAV = (
    "F:\\\u6dd8\u5b9d\u8d44\u6e90\\"
    "\u6e38\u620f\u52a8\u753b\u97f3\u6548\u5168\u96c6\\"
    "interface \u754c\u9762\\"
    "\u9f20\u6807\u52a8\u4f5c\uff08\u51b0\u5cf0\u738b\u5ea7\uff09\\"
    "\u9f20\u6807\u70b9\u51fb4-xys20070412.wav"
)
DEST_DIR = "/Game/DBA/Audio/UI/SFX"
ASSET_NAME = "SFX_UI_ButtonClick"


def log(message):
    unreal.log(f"[ImportUIButtonClickSound] {message}")


def main():
    if not os.path.exists(SOURCE_WAV):
        raise RuntimeError(f"Source wav does not exist: {SOURCE_WAV}")

    if not unreal.EditorAssetLibrary.does_directory_exist(DEST_DIR):
        unreal.EditorAssetLibrary.make_directory(DEST_DIR)

    task = unreal.AssetImportTask()
    task.filename = SOURCE_WAV
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

    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        raise RuntimeError(f"Imported asset could not be loaded: {asset_path}")

    try:
        asset.set_editor_property("sound_group", unreal.SoundGroup.SOUNDGROUP_UI)
    except Exception as exc:
        log(f"Could not set sound_group: {exc}")

    unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)
    log(f"Imported {SOURCE_WAV} as {asset_path}.{ASSET_NAME}")


if __name__ == "__main__":
    main()
