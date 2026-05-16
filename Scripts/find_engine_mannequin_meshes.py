import unreal


def log(msg: str) -> None:
    unreal.log(f"[FindEngineManny] {msg}")


def main() -> None:
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    filt = unreal.ARFilter(
        class_paths=[unreal.TopLevelAssetPath("/Script/Engine.SkeletalMesh")],
        package_paths=[unreal.Name("/Engine")],
        recursive_paths=True,
        recursive_classes=True,
    )
    assets = registry.get_assets(filt)
    count = 0
    for asset in assets:
        path = f"{asset.package_name}.{asset.asset_name}"
        lower = path.lower()
        if "manny" in lower or "mannequin" in lower or "quinn" in lower:
            mesh = unreal.load_asset(path)
            if not isinstance(mesh, unreal.SkeletalMesh):
                continue
            skeleton = mesh.get_editor_property("skeleton")
            skel_path = skeleton.get_path_name() if skeleton else "<none>"
            log(f"mesh={path} skeleton={skel_path}")
            count += 1
    log(f"count={count}")


if __name__ == "__main__":
    main()
