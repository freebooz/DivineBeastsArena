import unreal

fbx_file = r"F:\游戏设计资源\角色目录\Kachujin G Rosales.fbx"
dest_path = "/Game/DBA/Characters/Kachujin"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

task = unreal.AssetImportTask()
task.set_editor_property("filename", fbx_file)
task.set_editor_property("destination_path", dest_path)
task.set_editor_property("replace_existing", True)
task.set_editor_property("automated", True)
task.set_editor_property("save", True)

ui = unreal.FbxImportUI()
ui.set_editor_property("import_mesh", True)
ui.set_editor_property("import_as_skeletal", True)
ui.set_editor_property("import_animations", False)
ui.set_editor_property("create_physics_asset", True)
task.set_editor_property("options", ui)

asset_tools.import_asset_tasks([task])

imported = task.get_editor_property("imported_object_paths")
unreal.log("ImportedObjectPaths=" + str(imported))

if imported:
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    for path in imported:
        data = registry.get_asset_by_object_path(path)
        if data.is_valid():
            unreal.log(f"ImportedAsset: {data.asset_class_path.asset_name} {path}")

