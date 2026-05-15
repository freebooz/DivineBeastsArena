import unreal


def make_task(filename: str, dest_name: str) -> unreal.AssetImportTask:
    task = unreal.AssetImportTask()
    task.filename = filename
    task.destination_path = "/Game/DBA/UI/Lobby/Login/Textures"
    task.destination_name = dest_name
    task.automated = True
    task.replace_existing = True
    task.save = True
    return task


source_root = r"D:/DivineBeastsArena/Exports/Art/UI/LoginTextures"

tasks = [
    make_task(source_root + r"/T_DBA_LoginForestSanctuary.png", "T_DBA_LoginForestSanctuary"),
    make_task(source_root + r"/T_DBA_LoginPanel_StoneGold.png", "T_DBA_LoginPanel_StoneGold"),
    make_task(source_root + r"/T_DBA_LoginButton_ParchmentGold.png", "T_DBA_LoginButton_ParchmentGold"),
]

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
asset_tools.import_asset_tasks(tasks)

for task in tasks:
    if task.imported_object_paths:
        unreal.log(f"[DBA Login Texture Import] Imported: {task.imported_object_paths}")
    else:
        unreal.log_warning(f"[DBA Login Texture Import] No asset imported for {task.filename}")

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.SystemLibrary.quit_editor()
