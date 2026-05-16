import unreal


SKELETON_PATH = "/Game/DBA/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin"
RETARGETER_PATH = "/Game/DBA/Characters/Mannequins/Rigs/RTG_Mannequin.RTG_Mannequin"
TARGET_MESH_PATH = "/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rat.SKM_DBA_Zodiac_Rat"
SOURCE_ASSET_PATH = "/Game/DBA/Characters/Mannequins/Animations/Manny/MM_Idle.MM_Idle"


def log(msg: str) -> None:
    unreal.log(f"[TestStarterRetarget] {msg}")


def main() -> None:
    skel = unreal.load_asset(SKELETON_PATH)
    rtg = unreal.load_asset(RETARGETER_PATH)
    mesh = unreal.load_asset(TARGET_MESH_PATH)
    source_asset = unreal.load_asset(SOURCE_ASSET_PATH)

    if not skel or not rtg or not mesh or not source_asset:
        log("missing required asset")
        return

    ok = unreal.DBAEditorAnimationTools.assign_skeleton_to_mesh(mesh, skel)
    log(f"assign_skeleton ok={ok}")
    mesh = unreal.load_asset(TARGET_MESH_PATH)
    current_skeleton = mesh.get_editor_property("skeleton")
    log(f"mesh_skeleton={current_skeleton.get_path_name() if current_skeleton else '<none>'}")

    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    asset_data = registry.get_asset_by_object_path(unreal.SoftObjectPath(SOURCE_ASSET_PATH))
    results = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
        [asset_data],
        mesh,
        mesh,
        rtg,
        "",
        "",
        "RTG_Rat_",
        "",
        True,
        True,
    )
    log(f"retarget_results={len(results) if results else 0}")
    if results:
        for item in results:
            log(f"retargeted={item.package_name}.{item.asset_name}")


if __name__ == "__main__":
    main()
