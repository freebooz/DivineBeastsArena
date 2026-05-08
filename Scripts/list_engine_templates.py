import unreal

registry = unreal.AssetRegistryHelpers.get_asset_registry()

classes = ["SkeletalMesh", "AnimBlueprint", "AnimMontage", "ParticleSystem", "SoundBase"]

for cls in classes:
    unreal.log(f"[TemplateScan] class={cls}")
    filt = unreal.ARFilter(
        class_names=[unreal.Name(cls)],
        package_paths=[unreal.Name("/Engine"), unreal.Name("/Game")],
        recursive_paths=True,
        recursive_classes=True,
    )
    assets = registry.get_assets(filt)
    unreal.log(f"[TemplateScan] class={cls} count={len(assets)}")
    for a in assets[:20]:
        pkg = str(a.package_name)
        an = str(a.asset_name)
        unreal.log(f"[TemplateScan] {cls} {pkg}.{an}")
