# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

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
