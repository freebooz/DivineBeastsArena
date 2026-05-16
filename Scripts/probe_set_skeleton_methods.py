import unreal


def log(msg: str) -> None:
    unreal.log(f"[ProbeSetSkeleton] {msg}")


def main() -> None:
    candidates = [
        ("EditorSkeletalMeshLibrary", getattr(unreal, "EditorSkeletalMeshLibrary", None)),
        ("SkeletalMeshEditorSubsystem", getattr(unreal, "SkeletalMeshEditorSubsystem", None)),
        ("IKRetargetBatchOperation", getattr(unreal, "IKRetargetBatchOperation", None)),
    ]
    method_names = [
        "set_skeleton",
        "assign_skeleton",
        "set_skeletal_mesh_skeleton",
        "retarget_skeleton",
        "replace_skeleton",
    ]
    for label, obj in candidates:
        if not obj:
            log(f"{label}=<none>")
            continue
        for name in method_names:
            has = hasattr(obj, name)
            log(f"{label}.{name}={has}")

    mesh = unreal.load_asset("/Game/DBA/Characters/Rosales/Meshes/SK_Rosales.SK_Rosales")
    skel = unreal.load_asset("/Game/DBA/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin")
    if mesh and skel:
        log(f"mesh_has_set_skeleton={hasattr(mesh, 'set_skeleton')}")
        if hasattr(mesh, "set_skeleton"):
            try:
                mesh.set_skeleton(skel)
                log("mesh.set_skeleton call succeeded")
            except Exception as exc:
                log(f"mesh.set_skeleton call failed: {exc}")

        for method in [
            ("EditorSkeletalMeshLibrary.set_skeleton", getattr(getattr(unreal, "EditorSkeletalMeshLibrary", object), "set_skeleton", None)),
            ("SkeletalMeshEditorSubsystem.set_skeleton", getattr(unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem), "set_skeleton", None)),
        ]:
            name, fn = method
            if not fn:
                continue
            try:
                fn(mesh, skel)
                log(f"{name} call succeeded")
            except Exception as exc:
                log(f"{name} call failed: {exc}")


if __name__ == "__main__":
    main()
