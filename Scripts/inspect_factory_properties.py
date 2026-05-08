import unreal


def dump_props(cls_name: str):
    cls = getattr(unreal, cls_name, None)
    if not cls:
        unreal.log_warning(f"[InspectFactory] missing class {cls_name}")
        return
    obj = cls()
    unreal.log_warning(f"[InspectFactory] {cls_name}")
    for p in sorted([x for x in dir(obj) if not x.startswith("_")]):
        unreal.log_warning(f"[InspectFactory]   {p}")


for name in [
    "SkeletalMeshFromStaticMeshFactory",
    "AnimBlueprintFactory",
    "AnimMontageFactory",
]:
    dump_props(name)
