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
