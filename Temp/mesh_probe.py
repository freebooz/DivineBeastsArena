import unreal

paths = [
    '/Game/Models/Zodiac/Rat/SK_Rat_Mesh.SK_Rat_Mesh',
    '/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rat.SKM_DBA_Zodiac_Rat',
    '/Game/Models/Zodiac/Tiger/SK_Tiger_Mesh.SK_Tiger_Mesh',
    '/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Tiger.SKM_DBA_Zodiac_Tiger',
]
for p in paths:
    a = unreal.load_asset(p)
    if not a:
        unreal.log_warning(f'[MeshProbe] MISS {p}')
        continue
    cls = a.get_class().get_name()
    sk = getattr(a, 'skeleton', None)
    bounds = a.get_bounds()
    unreal.log_warning(f'[MeshProbe] OK {p} class={cls} skeleton={"Y" if sk else "N"} bounds={bounds.box_extent}')
