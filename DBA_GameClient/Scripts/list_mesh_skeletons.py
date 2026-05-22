# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


ROOTS = [
    "/Game/DBA/Heroes",
    "/Game/DBA/Characters",
    "/Game/DBA/Zodiacs/Chinese/Visuals/Meshes",
]


def log(msg: str) -> None:
    unreal.log(f"[ListMeshSkeletons] {msg}")


def list_meshes(root: str) -> list[str]:
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    filt = unreal.ARFilter(
        class_paths=[unreal.TopLevelAssetPath("/Script/Engine.SkeletalMesh")],
        package_paths=[unreal.Name(root)],
        recursive_paths=True,
        recursive_classes=True,
    )
    assets = registry.get_assets(filt)
    return [f"{a.package_name}.{a.asset_name}" for a in assets]


def main() -> None:
    for root in ROOTS:
        meshes = list_meshes(root)
        log(f"root={root} mesh_count={len(meshes)}")
        for object_path in meshes:
            mesh = unreal.load_asset(object_path)
            if not isinstance(mesh, unreal.SkeletalMesh):
                continue
            skeleton = mesh.get_editor_property("skeleton")
            skeleton_path = skeleton.get_path_name() if skeleton else "<none>"
            log(f"mesh={mesh.get_path_name()} skeleton={skeleton_path}")


if __name__ == "__main__":
    main()
