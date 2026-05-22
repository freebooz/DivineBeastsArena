# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


FRONTEND_LOGIN = "/Game/DBA/UI/Frontend/Login/WBP_DBA_Login"
LOBBY_LOGIN = "/Game/DBA/UI/Lobby/Login/WBP_DBA_Login"


def log(msg: str):
    unreal.log(f"[CleanupLobbyLoginWidget] {msg}")


def describe(asset_path: str):
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        log(f"missing: {asset_path}")
        return None
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        log(f"failed load: {asset_path}")
        return None
    log(f"{asset_path} class={asset.get_class().get_name()}")
    return asset


def main():
    frontend_asset = describe(FRONTEND_LOGIN)
    lobby_asset = describe(LOBBY_LOGIN)

    if not frontend_asset:
        raise RuntimeError("frontend login asset missing, abort cleanup")
    if not lobby_asset:
        return

    if not unreal.EditorAssetLibrary.delete_asset(LOBBY_LOGIN):
        raise RuntimeError(f"failed to delete old lobby login asset: {LOBBY_LOGIN}")
    log(f"deleted old lobby login asset: {LOBBY_LOGIN}")


main()
