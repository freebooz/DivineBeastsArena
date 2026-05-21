import unreal


MATERIAL_PATH = "/Game/DBA/Materials/M_DBA_RuntimeTint.M_DBA_RuntimeTint"


def main():
    material = unreal.load_asset(MATERIAL_PATH)
    if not material:
        raise RuntimeError(f"Failed to load {MATERIAL_PATH}")

    # Python property naming varies a little across UE versions; try the known editor names.
    for prop_name in ("used_with_skeletal_mesh", "b_used_with_skeletal_mesh"):
        try:
            material.set_editor_property(prop_name, True)
            unreal.log(f"Set {prop_name}=true on {material.get_path_name()}")
            break
        except Exception as exc:
            unreal.log_warning(f"Could not set {prop_name}: {exc}")

    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)
    unreal.log(f"Runtime tint material usage flags saved: {material.get_path_name()}")


if __name__ == "__main__":
    main()
