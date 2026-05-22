# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


SKELETON_PATH = "/Game/DBA/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin"
RETARGETER_PATH = "/Game/DBA/Characters/Mannequins/Rigs/RTG_Mannequin.RTG_Mannequin"
TARGET_MESH_PATH = "/Game/DBA/Zodiacs/Chinese/Visuals/Meshes/SKM_DBA_Zodiac_Rat.SKM_DBA_Zodiac_Rat"
SOURCE_ASSET_PATH = "/Game/DBA/Characters/Mannequins/Animations/Manny/MM_Idle.MM_Idle"


def log(msg: str) -> None:
    unreal.log(f"[TestStarterRetarget] {msg}")


def main() -> None:
    skel = unreal.load_asset(SKELETON_PATH)
    rtg = unreal.load_asset(RETARGETER_PATH)
    mesh = unreal.load_asset(TARGET_MESH_PATH)
    source_asset = unreal.load_asset(SOURCE_ASSET_PATH)

    if not skel or not rtg or not mesh or not source_asset:
        log("missing required asset")
        return

    ok = unreal.DBAEditorAnimationTools.assign_skeleton_to_mesh(mesh, skel)
    log(f"assign_skeleton ok={ok}")
    mesh = unreal.load_asset(TARGET_MESH_PATH)
    current_skeleton = mesh.get_editor_property("skeleton")
    log(f"mesh_skeleton={current_skeleton.get_path_name() if current_skeleton else '<none>'}")

    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    asset_data = registry.get_asset_by_object_path(unreal.SoftObjectPath(SOURCE_ASSET_PATH))
    results = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
        [asset_data],
        mesh,
        mesh,
        rtg,
        "",
        "",
        "RTG_Rat_",
        "",
        True,
        True,
    )
    log(f"retarget_results={len(results) if results else 0}")
    if results:
        for item in results:
            log(f"retargeted={item.package_name}.{item.asset_name}")


if __name__ == "__main__":
    main()
