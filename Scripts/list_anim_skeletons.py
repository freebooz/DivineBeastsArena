import unreal


ROOTS = [
    "/Game/DBA/Characters/Rosales/Animations",
    "/Game/Animation/Zodiac",
]


def log(msg: str) -> None:
    unreal.log(f"[ListAnimSkeletons] {msg}")


def main() -> None:
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    for root in ROOTS:
        paths = unreal.EditorAssetLibrary.list_assets(root, recursive=True, include_folder=False)
        log(f"root={root} assets={len(paths)}")
        for object_path in paths:
            package_name = object_path.split(".")[0]
            datas = registry.get_assets_by_package_name(unreal.Name(package_name), include_only_on_disk_assets=False)
            if not datas:
                continue
            data = datas[0]
            cls = str(data.asset_class_path.asset_name)
            if cls not in {"AnimSequence", "AnimMontage", "BlendSpace", "AnimBlueprint"}:
                continue
            obj = unreal.load_asset(object_path)
            if not obj:
                continue
            skel = None
            for prop in ["skeleton", "target_skeleton"]:
                try:
                    skel = obj.get_editor_property(prop)
                    if skel:
                        break
                except Exception:
                    pass
            skel_path = skel.get_path_name() if skel else "<none>"
            log(f"{cls} {object_path} skeleton={skel_path}")


if __name__ == "__main__":
    main()
