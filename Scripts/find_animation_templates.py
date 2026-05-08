import unreal

roots = [
    "/Engine",
]

keywords = ["SK_", "SKM_", "Skeleton", "ABP_", "AnimBP", "AM_", "Montage"]

for root in roots:
    assets = unreal.EditorAssetLibrary.list_assets(root, True, False)
    unreal.log_warning(f"[FindAnimTemplates] scan_root={root} total={len(assets)}")
    hits = []
    for p in assets:
        name = p.split("/")[-1]
        if any(k.lower() in name.lower() for k in keywords):
            hits.append(p)
    unreal.log_warning(f"[FindAnimTemplates] hits={len(hits)}")
    for p in hits[:200]:
        unreal.log_warning(f"[FindAnimTemplates] {p}")
