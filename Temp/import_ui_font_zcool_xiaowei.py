import unreal


task = unreal.AssetImportTask()
task.filename = r"D:/DivineBeastsArena/Exports/Fonts/ZCOOLXiaoWei/ZCOOLXiaoWei-Regular.ttf"
task.destination_path = "/Game/DBA/UI/Fonts"
task.destination_name = "F_DBA_ZCOOL_XiaoWei"
task.automated = True
task.replace_existing = True
task.save = True

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
asset_tools.import_asset_tasks([task])

for object_path in task.imported_object_paths:
    asset = unreal.load_asset(object_path)
    unreal.log(f"[DBA Font Import] Imported: {object_path} ({asset.get_class().get_name() if asset else 'None'})")

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.SystemLibrary.quit_editor()
