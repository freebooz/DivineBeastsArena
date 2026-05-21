import unreal


for name in dir(unreal):
    if "Skeletal" in name and "Subsystem" in name:
        unreal.log(name)

try:
    subsystem = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
    unreal.log("SkeletalMeshEditorSubsystem methods:")
    for name in dir(subsystem):
        if "skeleton" in name.lower() or "mesh" in name.lower():
            unreal.log(f"  {name}")
except Exception as exc:
    unreal.log_error(str(exc))
