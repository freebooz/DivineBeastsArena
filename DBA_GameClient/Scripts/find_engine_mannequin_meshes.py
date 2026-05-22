# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


def log(msg: str) -> None:
    unreal.log(f"[FindEngineManny] {msg}")


def main() -> None:
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    filt = unreal.ARFilter(
        class_paths=[unreal.TopLevelAssetPath("/Script/Engine.SkeletalMesh")],
        package_paths=[unreal.Name("/Engine")],
        recursive_paths=True,
        recursive_classes=True,
    )
    assets = registry.get_assets(filt)
    count = 0
    for asset in assets:
        path = f"{asset.package_name}.{asset.asset_name}"
        lower = path.lower()
        if "manny" in lower or "mannequin" in lower or "quinn" in lower:
            mesh = unreal.load_asset(path)
            if not isinstance(mesh, unreal.SkeletalMesh):
                continue
            skeleton = mesh.get_editor_property("skeleton")
            skel_path = skeleton.get_path_name() if skeleton else "<none>"
            log(f"mesh={path} skeleton={skel_path}")
            count += 1
    log(f"count={count}")


if __name__ == "__main__":
    main()
