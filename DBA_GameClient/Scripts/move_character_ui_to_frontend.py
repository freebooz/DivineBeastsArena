# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal

SOURCE_TARGETS = [
    ("/Game/DBA/UI/Lobby/Character/WBP_DBA_CharacterSelect", "/Game/DBA/UI/Frontend/Character/WBP_DBA_CharacterSelect"),
    ("/Game/DBA/UI/Lobby/Character/WBP_DBA_CharacterCreate", "/Game/DBA/UI/Frontend/Character/WBP_DBA_CharacterCreate"),
]


def log(msg: str) -> None:
    unreal.log(f"[MoveCharacterUIToFrontend] {msg}")


def ensure_directory(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)
        log(f"Created directory: {path}")


def duplicate_if_needed(src: str, dst: str) -> None:
    if not unreal.EditorAssetLibrary.does_asset_exist(src):
        raise RuntimeError(f"Source asset does not exist: {src}")

    if unreal.EditorAssetLibrary.does_asset_exist(dst):
        log(f"Already exists, skip: {dst}")
        return

    ok = unreal.EditorAssetLibrary.duplicate_asset(src, dst)
    if not ok:
        raise RuntimeError(f"Duplicate failed: {src} -> {dst}")
    log(f"Duplicated: {src} -> {dst}")


def main() -> None:
    ensure_directory("/Game/DBA/UI/Frontend")
    ensure_directory("/Game/DBA/UI/Frontend/Character")

    for src, dst in SOURCE_TARGETS:
        duplicate_if_needed(src, dst)

    if not unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True):
        raise RuntimeError("Failed to save dirty packages.")
    log("Done.")


if __name__ == "__main__":
    main()
