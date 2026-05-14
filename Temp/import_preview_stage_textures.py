import unreal


def make_task(filename: str, dest_name: str) -> unreal.AssetImportTask:
    task = unreal.AssetImportTask()
    task.filename = filename
    task.destination_path = "/Game/DBA/UI/Lobby/Character/Textures"
    task.destination_name = dest_name
    task.automated = True
    task.replace_existing = True
    task.save = True
    return task


source_root = r"D:/DivineBeastsArena/Exports/Art/UI/PreviewStage"

tasks = [
    make_task(source_root + r"/T_DBA_PreviewStage_Backdrop.png", "T_DBA_PreviewStage_Backdrop"),
    make_task(source_root + r"/T_DBA_PreviewStage_Foreground.png", "T_DBA_PreviewStage_Foreground"),
]

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
asset_tools.import_asset_tasks(tasks)

for task in tasks:
    if task.imported_object_paths:
        unreal.log(f"[DBA Preview Stage Texture Import] Imported: {task.imported_object_paths}")
    else:
        unreal.log_warning(f"[DBA Preview Stage Texture Import] No asset imported for {task.filename}")

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.SystemLibrary.quit_editor()
