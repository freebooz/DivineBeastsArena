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
