import unreal


ROOTS = [
    "/Game/DBA/Heroes",
    "/Game/DBA/Characters",
    "/Game/DBA/Zodiacs/Chinese/Visuals/Meshes",
]


def log(msg: str) -> None:
    unreal.log(f"[ListMeshSkeletons] {msg}")


def list_meshes(root: str) -> list[str]:
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    filt = unreal.ARFilter(
        class_paths=[unreal.TopLevelAssetPath("/Script/Engine.SkeletalMesh")],
        package_paths=[unreal.Name(root)],
        recursive_paths=True,
        recursive_classes=True,
    )
    assets = registry.get_assets(filt)
    return [f"{a.package_name}.{a.asset_name}" for a in assets]


def main() -> None:
    for root in ROOTS:
        meshes = list_meshes(root)
        log(f"root={root} mesh_count={len(meshes)}")
        for object_path in meshes:
            mesh = unreal.load_asset(object_path)
            if not isinstance(mesh, unreal.SkeletalMesh):
                continue
            skeleton = mesh.get_editor_property("skeleton")
            skeleton_path = skeleton.get_path_name() if skeleton else "<none>"
            log(f"mesh={mesh.get_path_name()} skeleton={skeleton_path}")


if __name__ == "__main__":
    main()
