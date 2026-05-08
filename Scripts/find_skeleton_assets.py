import unreal

registry = unreal.AssetRegistryHelpers.get_asset_registry()

def dump_by_class(class_name, limit=40):
    filt = unreal.ARFilter(
        class_paths=[unreal.TopLevelAssetPath(f"/Script/Engine.{class_name}")],
        package_paths=[unreal.Name("/Engine"), unreal.Name("/Game")],
        recursive_paths=True,
        recursive_classes=True,
    )
    assets = registry.get_assets(filt)
    unreal.log_warning(f"[ClassScan] class={class_name} count={len(assets)}")
    for a in assets[:limit]:
        unreal.log_warning(f"[ClassScan] {class_name} {a.package_name}.{a.asset_name}")

for cls in ["Skeleton", "SkeletalMesh", "AnimSequence", "AnimBlueprint", "AnimMontage", "ParticleSystem"]:
    dump_by_class(cls)
