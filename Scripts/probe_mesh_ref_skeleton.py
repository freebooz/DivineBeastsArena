import unreal


def log(msg: str) -> None:
    unreal.log(f"[ProbeMeshRefSkeleton] {msg}")


def inspect_mesh(path: str) -> None:
    mesh = unreal.load_asset(path)
    if not isinstance(mesh, unreal.SkeletalMesh):
        log(f"mesh missing: {path}")
        return
    log(f"mesh={mesh.get_path_name()}")
    for prop in ["skeleton", "ref_skeleton"]:
        try:
            value = mesh.get_editor_property(prop)
            log(f"prop {prop} type={type(value)} value={value}")
            names = [n for n in dir(value) if "bone" in n.lower() or "num" in n.lower() or "name" in n.lower()]
            log(f"prop {prop} methods={','.join(sorted(set(names)))}")
        except Exception as ex:
            log(f"prop {prop} error={ex}")


def main() -> None:
    inspect_mesh("/Game/DBA/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin")
    inspect_mesh("/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rat.SKM_DBA_Zodiac_Rat")


if __name__ == "__main__":
    main()
