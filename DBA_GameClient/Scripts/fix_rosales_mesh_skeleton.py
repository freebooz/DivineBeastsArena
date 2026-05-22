# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

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
