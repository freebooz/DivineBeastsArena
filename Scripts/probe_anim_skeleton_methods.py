import unreal


def log(msg: str) -> None:
    unreal.log(f"[ProbeAnimSkeleton] {msg}")


def main() -> None:
    anim = unreal.load_asset("/Game/DBA/Characters/Mannequins/Animations/Manny/MM_Idle.MM_Idle")
    if not anim:
        log("anim missing")
        return
    skeleton = None
    try:
        skeleton = anim.get_editor_property("skeleton")
    except Exception as exc:
        log(f"get skeleton failed: {exc}")
    log(f"skeleton={skeleton.get_path_name() if skeleton else '<none>'}")
    log(f"has_set_skeleton={hasattr(anim, 'set_skeleton')}")
    if hasattr(anim, "set_skeleton"):
        skel = unreal.load_asset("/Game/DBA/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin")
        try:
            anim.set_skeleton(skel)
            log("set_skeleton call succeeded")
        except Exception as exc:
            log(f"set_skeleton call failed: {exc}")


if __name__ == "__main__":
    main()
