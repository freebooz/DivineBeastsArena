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
