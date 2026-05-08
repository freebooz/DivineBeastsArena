import unreal

roots = ["/Engine/EngineMeshes", "/Engine/BasicShapes", "/Engine/VREditor", "/Game"]
for root in roots:
    assets = unreal.EditorAssetLibrary.list_assets(root, True, False)
    unreal.log_warning(f"[FindPlaceholders] root={root} count={len(assets)}")
    for p in assets[:30]:
        unreal.log_warning(f"[FindPlaceholders] {p}")
