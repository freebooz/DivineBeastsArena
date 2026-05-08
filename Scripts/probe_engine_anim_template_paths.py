import unreal


def try_load(path: str):
    obj = unreal.load_asset(path)
    if obj:
        unreal.log_warning(f"[ProbeEngineTemplate] hit path={path} class={obj.get_class().get_name()}")
    else:
        unreal.log_warning(f"[ProbeEngineTemplate] miss path={path}")


skeletal_mesh_candidates = [
    "/Engine/EngineMeshes/SkeletalCube.SkeletalCube",
    "/Engine/EngineMeshes/SkeletalCube_Simple.SkeletalCube_Simple",
    "/Engine/EngineMeshes/Humanoid.Humanoid",
    "/Engine/EngineMeshes/Humanoid.Humanoid",
    "/Engine/EngineMeshes/SK_Mannequin.SK_Mannequin",
    "/Engine/EditorMeshes/EditorSkeletalMesh.EditorSkeletalMesh",
    "/Engine/EditorMeshes/PersonaMesh.PersonaMesh",
]

skeleton_candidates = [
    "/Engine/EngineMeshes/Humanoid.Humanoid",
    "/Engine/EngineMeshes/SK_Mannequin_Skeleton.SK_Mannequin_Skeleton",
    "/Engine/EditorMeshes/PersonaAnimEditorSkel.PersonaAnimEditorSkel",
]

for p in skeletal_mesh_candidates:
    try_load(p)

for p in skeleton_candidates:
    try_load(p)
