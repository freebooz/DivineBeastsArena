# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


SOURCE = "/Game/DBA/UI/Lobby/Login/WBP_DBA_Login"
TARGET_DIR = "/Game/DBA/UI/Frontend/Login"
TARGET = f"{TARGET_DIR}/WBP_DBA_Login"


def log(msg: str):
    unreal.log(f"[MoveLoginUIToFrontend] {msg}")


def ensure_directory(path: str):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        if not unreal.EditorAssetLibrary.make_directory(path):
            raise RuntimeError(f"failed to create directory: {path}")
        log(f"created directory: {path}")


def main():
    ensure_directory("/Game/DBA/UI/Frontend")
    ensure_directory(TARGET_DIR)

    if unreal.EditorAssetLibrary.does_asset_exist(TARGET):
        log(f"target already exists: {TARGET}")
        return

    if not unreal.EditorAssetLibrary.does_asset_exist(SOURCE):
        log(f"source not found, skip move: {SOURCE}")
        return

    ok = unreal.EditorAssetLibrary.rename_asset(SOURCE, TARGET)
    if not ok:
        raise RuntimeError(f"failed to move: {SOURCE} -> {TARGET}")

    log(f"moved: {SOURCE} -> {TARGET}")


main()
