import unreal


MESH_PATH = "/Game/DBA/Characters/Rosales/Meshes/SK_Rosales.SK_Rosales"
SKELETON_PATH = "/Game/DBA/Characters/Rosales/Meshes/SKEL_Rosales.SKEL_Rosales"


def main():
    mesh = unreal.load_asset(MESH_PATH)
    skeleton = unreal.load_asset(SKELETON_PATH)
    if not mesh:
        raise RuntimeError(f"Failed to load mesh: {MESH_PATH}")
    if not skeleton:
        raise RuntimeError(f"Failed to load skeleton: {SKELETON_PATH}")

    current_skeleton = mesh.get_editor_property("skeleton")
    unreal.log(f"Rosales mesh current skeleton: {current_skeleton.get_path_name() if current_skeleton else '<none>'}")
    mesh.set_editor_property("skeleton", skeleton)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)
    unreal.EditorAssetLibrary.save_loaded_asset(skeleton, only_if_is_dirty=False)
    unreal.log(f"Rosales mesh skeleton fixed: {skeleton.get_path_name()}")


if __name__ == "__main__":
    main()
