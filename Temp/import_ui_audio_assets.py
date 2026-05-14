import unreal


def make_task(filename: str, dest_path: str, dest_name: str) -> unreal.AssetImportTask:
    task = unreal.AssetImportTask()
    task.filename = filename
    task.destination_path = dest_path
    task.destination_name = dest_name
    task.automated = True
    task.replace_existing = True
    task.save = True
    return task


source_root = r"D:/DivineBeastsArena/Temp/AudioSources"

tasks = [
    make_task(
        source_root + r"/kenney-interface-sounds/addons/kenney_interface_sounds/select_001.wav",
        "/Game/DBA/Audio/UI/SFX",
        "SFX_UI_ButtonClick",
    ),
    make_task(
        source_root + r"/kenney-allin1/Audio (295 files)/Jingle sounds (85 sounds)/jingles_STEEL/jingles_STEEL03.ogg",
        "/Game/DBA/Audio/UI/BGM",
        "BGM_Login_Loop",
    ),
    make_task(
        source_root + r"/kenney-allin1/Audio (295 files)/Jingle sounds (85 sounds)/jingles_STEEL/jingles_STEEL11.ogg",
        "/Game/DBA/Audio/UI/BGM",
        "BGM_CharacterSelect_Loop",
    ),
    make_task(
        source_root + r"/kenney-allin1/Audio (295 files)/Jingle sounds (85 sounds)/jingles_STEEL/jingles_STEEL14.ogg",
        "/Game/DBA/Audio/UI/BGM",
        "BGM_CharacterCreate_Loop",
    ),
]

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
asset_tools.import_asset_tasks(tasks)

for task in tasks:
    if task.imported_object_paths:
        unreal.log(f"[DBA Audio Import] Imported: {task.imported_object_paths}")
    else:
        unreal.log_warning(f"[DBA Audio Import] No asset imported for {task.filename}")

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
