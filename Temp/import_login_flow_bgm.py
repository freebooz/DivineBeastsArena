import unreal


def make_task(filename: str, destination_name: str) -> unreal.AssetImportTask:
    task = unreal.AssetImportTask()
    task.filename = filename
    task.destination_path = "/Game/DBA/Audio/UI/BGM"
    task.destination_name = destination_name
    task.automated = True
    task.replace_existing = True
    task.save = True
    return task


source_file = r"D:/DivineBeastsArena/Exports/Audio/UI/BGM/BGM_LoginFlow_Loop.wav"

tasks = [
    make_task(source_file, "BGM_LoginFlow_Loop"),
    make_task(source_file, "BGM_Login_Loop"),
]

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
asset_tools.import_asset_tasks(tasks)

for task in tasks:
    if task.imported_object_paths:
        unreal.log(f"[DBA Login Flow BGM Import] Imported: {task.imported_object_paths}")
    else:
        unreal.log_warning(f"[DBA Login Flow BGM Import] No asset imported for {task.destination_name}")

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.SystemLibrary.quit_editor()
