# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：项目工具脚本，用于资产整理、模拟服务或本地开发辅助流程。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import os
import unreal

SOURCE_DIR = r"F:\游戏设计资源\角色\Rosales"
DEST_ROOT = "/Game/DBA/Characters/Rosales"
DEST_MESH = f"{DEST_ROOT}/Meshes"
DEST_ANIM = f"{DEST_ROOT}/Animations"
DEST_ABP = f"{DEST_ROOT}/AnimationBP"
MODEL_FILE = "Kachujin G Rosales.fbx"
MODEL_ASSET_NAME = "SK_Rosales"
SKELETON_ASSET_NAME = "SKEL_Rosales"
ANIM_BP_NAME = "ABP_Rosales"

ANIM_FILES = [
    "Standing Idle.fbx",
    "Standard Walk.fbx",
    "Run Forward.fbx",
    "Jumping Up.fbx",
    "Standing 2H Magic Attack 02.fbx",
    "Sword And Shield Death.fbx",
    "Arms Hip Hop Dance.fbx",
]


def log(msg: str):
    unreal.log(f"[ImportRosales] {msg}")


def build_task(filename: str, destination_path: str, options: unreal.FbxImportUI, save: bool = True) -> unreal.AssetImportTask:
    task = unreal.AssetImportTask()
    task.filename = filename
    task.destination_path = destination_path
    task.automated = True
    task.save = save
    task.replace_existing = True
    task.replace_existing_settings = True
    task.options = options
    return task


def import_model():
    model_path = os.path.join(SOURCE_DIR, MODEL_FILE)
    if not os.path.exists(model_path):
        raise RuntimeError(f"Model file not found: {model_path}")

    fbx_ui = unreal.FbxImportUI()
    fbx_ui.import_as_skeletal = True
    fbx_ui.import_mesh = True
    fbx_ui.import_animations = False
    fbx_ui.import_materials = True
    fbx_ui.import_textures = True
    fbx_ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
    try:
        fbx_ui.skeletal_mesh_import_data.set_editor_property("import_mesh_lods", True)
    except Exception:
        pass

    task = build_task(model_path, DEST_MESH, fbx_ui)
    task.destination_name = MODEL_ASSET_NAME
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    if not task.imported_object_paths:
        raise RuntimeError("Model import failed with empty imported_object_paths.")

    log(f"Model imported: {task.imported_object_paths}")


def load_skeleton() -> unreal.Skeleton:
    skeleton_path = f"{DEST_MESH}/{SKELETON_ASSET_NAME}.{SKELETON_ASSET_NAME}"
    skeleton = unreal.load_object(None, skeleton_path)
    if not skeleton:
        raise RuntimeError(f"Skeleton not found after model import: {skeleton_path}")
    return skeleton


def sanitize_anim_asset_name(filename: str) -> str:
    base = os.path.splitext(os.path.basename(filename))[0]
    valid = base.replace(" ", "_").replace("-", "_")
    return f"AN_{valid}"


def import_animations(skeleton: unreal.Skeleton):
    for anim_file in ANIM_FILES:
        src = os.path.join(SOURCE_DIR, anim_file)
        if not os.path.exists(src):
            log(f"Skip missing animation: {src}")
            continue

        fbx_ui = unreal.FbxImportUI()
        fbx_ui.import_as_skeletal = True
        fbx_ui.import_mesh = False
        fbx_ui.import_animations = True
        fbx_ui.import_materials = False
        fbx_ui.import_textures = False
        fbx_ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION
        fbx_ui.skeleton = skeleton

        task = build_task(src, DEST_ANIM, fbx_ui)
        task.destination_name = sanitize_anim_asset_name(anim_file)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        log(f"Animation imported from {anim_file}: {task.imported_object_paths}")


def create_anim_blueprint(skeleton: unreal.Skeleton):
    anim_bp_package = f"{DEST_ABP}/{ANIM_BP_NAME}"
    if unreal.EditorAssetLibrary.does_asset_exist(anim_bp_package):
        log(f"AnimBP already exists: {anim_bp_package}")
        return

    factory = unreal.AnimBlueprintFactory()
    factory.target_skeleton = skeleton
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    new_asset = asset_tools.create_asset(
        asset_name=ANIM_BP_NAME,
        package_path=DEST_ABP,
        asset_class=unreal.AnimBlueprint,
        factory=factory,
    )
    if not new_asset:
        raise RuntimeError("Failed to create Anim Blueprint.")
    log(f"AnimBP created: {new_asset.get_path_name()}")


def main():
    unreal.EditorAssetLibrary.make_directory(DEST_ROOT)
    unreal.EditorAssetLibrary.make_directory(DEST_MESH)
    unreal.EditorAssetLibrary.make_directory(DEST_ANIM)
    unreal.EditorAssetLibrary.make_directory(DEST_ABP)
    import_model()
    skeleton = load_skeleton()
    import_animations(skeleton)
    create_anim_blueprint(skeleton)
    unreal.EditorAssetLibrary.save_directory(DEST_ROOT, only_if_is_dirty=False, recursive=True)
    log("ImportRosales completed.")


if __name__ == "__main__":
    main()
