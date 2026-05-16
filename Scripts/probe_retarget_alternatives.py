import unreal


def log(msg: str) -> None:
    unreal.log(f"[ProbeRetargetAlt] {msg}")


def dump_obj(name: str, obj) -> None:
    if not obj:
        log(f"{name}=<none>")
        return
    names = sorted(set(dir(obj)))
    filtered = [n for n in names if any(k in n.lower() for k in ["retarget", "ik", "rig", "anim", "batch", "duplicate"])]
    log(f"{name} methods={','.join(filtered)}")


def main() -> None:
    for attr in sorted(dir(unreal)):
        lower = attr.lower()
        if "retarget" in lower or "ikrig" in lower:
            try:
                obj = getattr(unreal, attr)
                dump_obj(attr, obj)
            except Exception as ex:
                log(f"{attr} error={ex}")

    try:
        subsystem = unreal.get_editor_subsystem(unreal.IKRigEditorSubsystem)
        dump_obj("IKRigEditorSubsystemInstance", subsystem)
    except Exception as ex:
        log(f"IKRigEditorSubsystem unavailable: {ex}")


if __name__ == "__main__":
    main()
