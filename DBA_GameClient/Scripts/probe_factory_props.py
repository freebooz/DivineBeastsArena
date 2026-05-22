# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal

def dump(factory_cls_name):
    cls = getattr(unreal, factory_cls_name, None)
    if not cls:
        unreal.log_warning(f"[FactoryProps] missing {factory_cls_name}")
        return
    obj = cls()
    props = [p for p in dir(obj) if not p.startswith("_")]
    unreal.log_warning(f"[FactoryProps] {factory_cls_name} props_count={len(props)}")
    for p in props:
        if "mesh" in p.lower() or "skeleton" in p.lower() or "target" in p.lower() or "asset" in p.lower():
            unreal.log_warning(f"[FactoryProps] {factory_cls_name}.{p}")

for name in [
    "SkeletalMeshFromStaticMeshFactory",
    "SkeletonFromStaticMeshFactory",
    "AnimMontageFactory",
    "AnimBlueprintFactory",
]:
    dump(name)
