# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

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
