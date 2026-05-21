#!/usr/bin/env python3
import sys
import unreal


PAIRS = [
    (
        "/Game/DBA/Characters/Mannequins/Meshes/SKM_Manny_Simple",
        "/Game/DBA/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny_C",
    ),
    (
        "/Game/DBA/Characters/Mannequins/Meshes/SK_Mannequin",
        "/Game/DBA/Characters/Mannequins/Animations/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C",
    ),
    (
        "/Game/DBA/Characters/Mannequins/Meshes/SKM_Manny_Simple",
        "/Game/DBA/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed.ABP_Unarmed_C",
    ),
]


def main():
    ok = False
    for mesh_path, anim_path in PAIRS:
        mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
        anim_class = unreal.load_class(None, anim_path)
        unreal.log(f"Candidate mesh={mesh_path} loaded={bool(mesh)} anim={anim_path} loaded={bool(anim_class)}")
        if mesh and anim_class:
            ok = True
    if not ok:
        sys.exit(1)


if __name__ == "__main__":
    main()
