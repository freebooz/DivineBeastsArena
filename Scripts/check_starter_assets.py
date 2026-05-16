import unreal


def log(msg: str) -> None:
    unreal.log(f"[CheckStarterAssets] {msg}")


def main() -> None:
    paths = [
        "/Game/DBA/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin",
        "/Game/DBA/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny",
        "/Game/DBA/Characters/Mannequins/Rigs/IK_Mannequin.IK_Mannequin",
        "/Game/DBA/Characters/Mannequins/Rigs/RTG_Mannequin.RTG_Mannequin",
        "/Game/DBA/Characters/Mannequins/Animations/Manny/MM_Idle.MM_Idle",
    ]
    for path in paths:
        obj = unreal.load_asset(path)
        if not obj:
            log(f"missing {path}")
            continue
        cls = obj.get_class().get_name()
        log(f"asset={path} class={cls}")
        for prop in ["skeleton", "target_skeleton", "source_mesh", "target_mesh", "source_ik_rig", "target_ik_rig"]:
            try:
                value = obj.get_editor_property(prop)
                if value:
                    value_path = value.get_path_name() if hasattr(value, "get_path_name") else str(value)
                    log(f"  {prop}={value_path}")
                else:
                    log(f"  {prop}=<none>")
            except Exception:
                pass


if __name__ == "__main__":
    main()
