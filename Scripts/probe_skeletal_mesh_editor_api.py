import unreal


def log(msg: str) -> None:
    unreal.log(f"[ProbeSkelMeshAPI] {msg}")


def dump_type(label: str, cls) -> None:
    if not cls:
        log(f"{label}=<none>")
        return
    methods = [name for name in dir(cls) if "skeleton" in name.lower() or "mesh" in name.lower()]
    methods = sorted(set(methods))
    log(f"{label} methods={','.join(methods)}")


def main() -> None:
    dump_type("SkeletalMeshEditorSubsystem", unreal.SkeletalMeshEditorSubsystem)
    dump_type("SkeletalMeshEditingLibrary", getattr(unreal, "SkeletalMeshEditingLibrary", None))
    dump_type("EditorSkeletalMeshLibrary", getattr(unreal, "EditorSkeletalMeshLibrary", None))

    try:
        subsystem = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
        methods = [name for name in dir(subsystem) if "skeleton" in name.lower() or "mesh" in name.lower()]
        methods = sorted(set(methods))
        log(f"subsystem_instance methods={','.join(methods)}")
    except Exception as exc:
        log(f"subsystem_instance error={exc}")


if __name__ == "__main__":
    main()
