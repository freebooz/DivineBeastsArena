# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal

registry = unreal.AssetRegistryHelpers.get_asset_registry()

def dump_by_class(class_name, limit=40):
    filt = unreal.ARFilter(
        class_paths=[unreal.TopLevelAssetPath(f"/Script/Engine.{class_name}")],
        package_paths=[unreal.Name("/Engine"), unreal.Name("/Game")],
        recursive_paths=True,
        recursive_classes=True,
    )
    assets = registry.get_assets(filt)
    unreal.log_warning(f"[ClassScan] class={class_name} count={len(assets)}")
    for a in assets[:limit]:
        unreal.log_warning(f"[ClassScan] {class_name} {a.package_name}.{a.asset_name}")

for cls in ["Skeleton", "SkeletalMesh", "AnimSequence", "AnimBlueprint", "AnimMontage", "ParticleSystem"]:
    dump_by_class(cls)
