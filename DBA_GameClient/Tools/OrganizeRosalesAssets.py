# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：项目工具脚本，用于资产整理、模拟服务或本地开发辅助流程。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal

SRC_ROOT = "/Game/Characters/Kachujin"
DST_ROOT = "/Game/DBA/Characters/Rosales"
DST_MESH = f"{DST_ROOT}/Meshes"
DST_ANIM = f"{DST_ROOT}/Animations"
DST_ABP = f"{DST_ROOT}/AnimationBP"


def log(msg: str):
    unreal.log(f"[OrganizeRosalesAssets] {msg}")


def ensure_dirs():
    unreal.EditorAssetLibrary.make_directory(DST_ROOT)
    unreal.EditorAssetLibrary.make_directory(DST_MESH)
    unreal.EditorAssetLibrary.make_directory(DST_ANIM)
    unreal.EditorAssetLibrary.make_directory(DST_ABP)


def move_if_exists(src_asset: str, dst_asset: str):
    if not unreal.EditorAssetLibrary.does_asset_exist(src_asset):
        return False
    ok = unreal.EditorAssetLibrary.rename_asset(src_asset, dst_asset)
    log(f"Move {'OK' if ok else 'FAIL'}: {src_asset} -> {dst_asset}")
    return ok


def move_assets():
    # Mesh/skeleton/physics/materials/textures
    fixed_assets = {
        f"{SRC_ROOT}/Kachujin_G_Rosales": f"{DST_MESH}/SK_Rosales",
        f"{SRC_ROOT}/Kachujin_G_Rosales_Skeleton": f"{DST_MESH}/SKEL_Rosales",
        f"{SRC_ROOT}/Kachujin_G_Rosales_PhysicsAsset": f"{DST_MESH}/PHYS_Rosales",
        f"{SRC_ROOT}/kachujin_MAT": f"{DST_MESH}/M_Rosales",
        f"{SRC_ROOT}/kachujin_MAT_": f"{DST_MESH}/M_Rosales_Alt",
        f"{SRC_ROOT}/Kachujin_diffuse": f"{DST_MESH}/T_Rosales_Diffuse",
        f"{SRC_ROOT}/Kachujin_diffuse_body": f"{DST_MESH}/T_Rosales_Diffuse_Body",
        f"{SRC_ROOT}/Kachujin_normal": f"{DST_MESH}/T_Rosales_Normal",
        f"{SRC_ROOT}/Kachujin_specular": f"{DST_MESH}/T_Rosales_Specular",
    }
    for src, dst in fixed_assets.items():
        move_if_exists(src, dst)

    # Move imported anim sequences into Animations folder
    all_assets = unreal.EditorAssetLibrary.list_assets(SRC_ROOT, recursive=True, include_folder=False)
    for path in all_assets:
        name = path.split("/")[-1]
        if name.startswith("AN_") or name.endswith("_Anim"):
            move_if_exists(path, f"{DST_ANIM}/{name}")


def create_anim_blueprint():
    skeleton_path = f"{DST_MESH}/SKEL_Rosales.SKEL_Rosales"
    skeleton = unreal.load_object(None, skeleton_path)
    if not skeleton:
        raise RuntimeError(f"Skeleton not found: {skeleton_path}")

    anim_bp_asset = f"{DST_ABP}/ABP_Rosales"
    if unreal.EditorAssetLibrary.does_asset_exist(anim_bp_asset):
        log("AnimBP already exists: /Game/DBA/Characters/Rosales/AnimationBP/ABP_Rosales")
        return

    factory = unreal.AnimBlueprintFactory()
    configured = False
    for prop_name in ["target_skeleton", "TargetSkeleton", "skeleton", "Skeleton"]:
        try:
            factory.set_editor_property(prop_name, skeleton)
            configured = True
            log(f"AnimBlueprintFactory skeleton property set: {prop_name}")
            break
        except Exception:
            continue
    if not configured:
        log("AnimBlueprintFactory skeleton property set skipped; continue with default factory behavior.")
    new_asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name="ABP_Rosales",
        package_path=DST_ABP,
        asset_class=unreal.AnimBlueprint,
        factory=factory,
    )
    if not new_asset:
        raise RuntimeError("Failed to create ABP_Rosales")
    log(f"Created AnimBP: {new_asset.get_path_name()}")


def main():
    ensure_dirs()
    move_assets()
    create_anim_blueprint()
    unreal.EditorAssetLibrary.save_directory(DST_ROOT, only_if_is_dirty=False, recursive=True)
    log("Done")


if __name__ == "__main__":
    main()
