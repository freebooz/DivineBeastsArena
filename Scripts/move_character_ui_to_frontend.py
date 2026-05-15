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
