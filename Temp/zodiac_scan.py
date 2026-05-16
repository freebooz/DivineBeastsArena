import unreal

ar = unreal.AssetRegistryHelpers.get_asset_registry()
assets = ar.get_assets_by_path('/Game', recursive=True)
for a in assets:
    if a.asset_class_path.asset_name != 'SkeletalMesh':
        continue
    p = a.object_path.string
    if 'Zodiac' not in p and 'zodiac' not in p:
        continue
    obj = unreal.load_asset(p)
    if not obj:
        continue
    b = obj.get_bounds().box_extent
    sk = getattr(obj, 'skeleton', None)
    if b.x > 20 or b.y > 20 or b.z > 20:
        unreal.log_warning(f'[ZodiacScan] {p} skeleton={"Y" if sk else "N"} extent=({b.x:.1f},{b.y:.1f},{b.z:.1f})')
