import unreal


ASSETS = [
    "/Game/DBA/Characters/Rosales/Meshes/SK_Rosales.SK_Rosales",
    "/Game/DBA/Characters/Rosales/Meshes/SKEL_Rosales.SKEL_Rosales",
    "/Game/DBA/Characters/Rosales/Animations/AN_Standing_Idle.AN_Standing_Idle",
    "/Game/DBA/Characters/Rosales/Animations/AN_Run_Forward.AN_Run_Forward",
    "/Game/DBA/Characters/Rosales/Animations/AN_Standing_2H_Magic_Attack_02.AN_Standing_2H_Magic_Attack_02",
]


for path in ASSETS:
    asset = unreal.load_asset(path)
    unreal.log(f"ASSET {path}: {asset}")
    if asset:
        for prop in ("skeleton", "preview_mesh"):
            try:
                value = asset.get_editor_property(prop)
                unreal.log(f"  {prop}: {value.get_path_name() if value else value}")
            except Exception:
                pass
