# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：项目工具脚本，用于资产整理、模拟服务或本地开发辅助流程。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal

MESH_PATH = "/Game/DBA/Characters/Rosales/Meshes/SK_Rosales.SK_Rosales"
SKELETON_PATH = "/Game/DBA/Characters/Rosales/Meshes/SKEL_Rosales.SKEL_Rosales"
PHYS_PATH = "/Game/DBA/Characters/Rosales/Meshes/PHYS_Rosales.PHYS_Rosales"


def log(msg: str):
    unreal.log(f"[FixRosalesSkeletonBinding] {msg}")


def main():
    mesh = unreal.load_object(None, MESH_PATH)
    skeleton = unreal.load_object(None, SKELETON_PATH)
    physics = unreal.load_object(None, PHYS_PATH)

    if not mesh:
        raise RuntimeError(f"Missing mesh: {MESH_PATH}")
    if not skeleton:
        raise RuntimeError(f"Missing skeleton: {SKELETON_PATH}")

    current_skeleton = mesh.get_editor_property("skeleton")
    log(f"Before skeleton={current_skeleton.get_path_name() if current_skeleton else 'None'}")

    if not current_skeleton:
        mesh.set_editor_property("skeleton", skeleton)
        log("Skeleton assigned to mesh.")

    if physics:
        current_physics = mesh.get_editor_property("physics_asset")
        if not current_physics:
            mesh.set_editor_property("physics_asset", physics)
            log("Physics asset assigned to mesh.")

    unreal.EditorAssetLibrary.save_asset("/Game/DBA/Characters/Rosales/Meshes/SK_Rosales", only_if_is_dirty=False)
    mesh = unreal.load_object(None, MESH_PATH)
    final_skeleton = mesh.get_editor_property("skeleton")
    log(f"After skeleton={final_skeleton.get_path_name() if final_skeleton else 'None'}")


if __name__ == "__main__":
    main()

