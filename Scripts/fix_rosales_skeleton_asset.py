import unreal


ROSALES_MESH_PATH = "/Game/DBA/Characters/Rosales/Meshes/SK_Rosales.SK_Rosales"


def main():
    mesh = unreal.load_asset(ROSALES_MESH_PATH)
    if not mesh:
        raise RuntimeError(f"Failed to load Rosales mesh: {ROSALES_MESH_PATH}")

    skeleton = mesh.get_editor_property("skeleton")
    if not skeleton:
        raise RuntimeError("Rosales mesh has no skeleton")

    unreal.log(f"Rosales mesh: {mesh.get_path_name()}")
    unreal.log(f"Rosales skeleton: {skeleton.get_path_name()}")

    unreal.EditorAssetLibrary.save_loaded_asset(skeleton, only_if_is_dirty=False)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)
    unreal.log("Rosales skeleton and mesh saved.")


if __name__ == "__main__":
    main()
