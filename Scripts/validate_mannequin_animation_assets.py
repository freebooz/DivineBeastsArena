import unreal


CLASS_CANDIDATES = [
    "/Game/DBA/Characters/Mannequins/Animations/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C",
    "/Game/DBA/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C",
    "/Game/DBA/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny_C",
]

ANIM_CANDIDATES = [
    "/Game/DBA/Characters/Mannequins/Animations/ThirdPersonIdle.ThirdPersonIdle",
    "/Game/DBA/Characters/Mannequins/Animations/Manny/MM_Idle.MM_Idle",
    "/Game/DBA/Characters/Mannequins/Anims/Unarmed/MM_Idle.MM_Idle",
]


for path in CLASS_CANDIDATES:
    loaded = unreal.load_class(None, path)
    unreal.log(f"anim class candidate: {path} -> {loaded.get_name() if loaded else '<failed>'}")

for path in ANIM_CANDIDATES:
    loaded = unreal.EditorAssetLibrary.load_asset(path)
    skeleton = None
    if loaded:
        try:
            skeleton = loaded.get_editor_property("skeleton")
        except Exception:
            skeleton = None
    unreal.log(f"animation asset candidate: {path} -> {loaded.get_name() if loaded else '<failed>'}; skeleton={skeleton.get_path_name() if skeleton else '<none>'}")
