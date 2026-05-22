# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal
from typing import Optional, Tuple


STARTER_SKELETON_CANDIDATES = [
    "/Game/DBA/Characters/Mannequins/Meshes/SK_Mannequin_Skeleton.SK_Mannequin_Skeleton",
    "/Game/DBA/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin",
    "/Game/Characters/Mannequins/Meshes/SKM_Manny_Skeleton.SKM_Manny_Skeleton",
    "/Game/Characters/Mannequins/Meshes/SK_Mannequin_Skeleton.SK_Mannequin_Skeleton",
    "/Engine/Characters/Mannequins/Meshes/SKM_Manny_Skeleton.SKM_Manny_Skeleton",
    "/Engine/EngineMeshes/SK_Mannequin_Skeleton.SK_Mannequin_Skeleton",
]

SOURCE_MESH_CANDIDATES = [
    "/Game/DBA/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin",
    "/Game/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny",
    "/Game/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin",
    "/Engine/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny",
    "/Engine/EngineMeshes/SK_Mannequin.SK_Mannequin",
]

RETARGETER_CANDIDATES = [
    "/Game/DBA/Characters/Mannequins/Rigs/RTG_Mannequin.RTG_Mannequin",
    "/Game/Characters/Mannequins/Retargeting/RTG_UE5Manny_UE4Manny.RTG_UE5Manny_UE4Manny",
    "/Engine/Characters/Mannequins/Retargeting/RTG_UE5Manny_UE4Manny.RTG_UE5Manny_UE4Manny",
]

SOURCE_ANIM_ROOT_CANDIDATES = [
    "/Game/DBA/Characters/Mannequins/Animations",
    "/Game/DBA/Characters/Mannequins/Anims",
    "/Game/Characters/Mannequins/Animations",
    "/Engine/Characters/Mannequins/Animations",
]

TARGET_MESH_ROOTS = [
    "/Game/DBA/Heroes",
]

ASSET_CLASSES_FOR_RETARGET = {"AnimSequence", "AnimMontage", "BlendSpace", "AnimBlueprint"}


def log(msg: str) -> None:
    unreal.log(f"[StarterSkeletonRetarget] {msg}")


def warn(msg: str) -> None:
    unreal.log_warning(f"[StarterSkeletonRetarget] {msg}")


def try_load_asset(path: str):
    try:
        return unreal.load_asset(path)
    except Exception as ex:
        warn(f"load_failed path={path} err={ex}")
        return None


def _asset_data_to_object_path(asset_data: unreal.AssetData) -> str:
    return f"{asset_data.package_name}.{asset_data.asset_name}"


def resolve_source_mesh_and_skeleton() -> Tuple[Optional[unreal.SkeletalMesh], Optional[unreal.Skeleton]]:
    preferred_skeletons = []
    for path in STARTER_SKELETON_CANDIDATES:
        asset = try_load_asset(path)
        if isinstance(asset, unreal.Skeleton):
            preferred_skeletons.append(asset)

    for mesh_path in SOURCE_MESH_CANDIDATES:
        mesh = try_load_asset(mesh_path)
        if not isinstance(mesh, unreal.SkeletalMesh):
            continue
        skeleton = mesh.get_editor_property("skeleton")
        if not isinstance(skeleton, unreal.Skeleton) and preferred_skeletons:
            for preferred in preferred_skeletons:
                ok = unreal.DBAEditorAnimationTools.assign_skeleton_to_mesh(mesh, preferred)
                if ok:
                    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
                    mesh = try_load_asset(mesh_path)
                    skeleton = mesh.get_editor_property("skeleton") if isinstance(mesh, unreal.SkeletalMesh) else None
                    log(
                        f"source mesh skeleton repaired mesh={mesh_path} skeleton={preferred.get_path_name()}"
                    )
                    break
        if not isinstance(skeleton, unreal.Skeleton):
            continue
        if preferred_skeletons and skeleton not in preferred_skeletons:
            continue
        log(f"source_mesh={mesh.get_path_name()} source_skeleton={skeleton.get_path_name()}")
        return mesh, skeleton

    for mesh_path in SOURCE_MESH_CANDIDATES:
        mesh = try_load_asset(mesh_path)
        if not isinstance(mesh, unreal.SkeletalMesh):
            continue
        skeleton = mesh.get_editor_property("skeleton")
        if isinstance(skeleton, unreal.Skeleton):
            warn(
                "using source mesh with non-preferred skeleton "
                f"mesh={mesh.get_path_name()} skeleton={skeleton.get_path_name()}"
            )
            return mesh, skeleton

    return None, None


def resolve_retargeter() -> Optional[unreal.IKRetargeter]:
    for path in RETARGETER_CANDIDATES:
        asset = try_load_asset(path)
        if isinstance(asset, unreal.IKRetargeter):
            log(f"retargeter={asset.get_path_name()}")
            return asset
    return None


def list_skeletal_meshes(root: str) -> list[unreal.SkeletalMesh]:
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    filt = unreal.ARFilter(
        class_paths=[unreal.TopLevelAssetPath("/Script/Engine.SkeletalMesh")],
        package_paths=[unreal.Name(root)],
        recursive_paths=True,
        recursive_classes=True,
    )
    out: list[unreal.SkeletalMesh] = []
    for asset_data in registry.get_assets(filt):
        mesh = unreal.load_asset(f"{asset_data.package_name}.{asset_data.asset_name}")
        if isinstance(mesh, unreal.SkeletalMesh):
            out.append(mesh)
    return out


def gather_target_meshes() -> list[unreal.SkeletalMesh]:
    meshes: list[unreal.SkeletalMesh] = []
    seen = set()
    for root in TARGET_MESH_ROOTS:
        for mesh in list_skeletal_meshes(root):
            path = mesh.get_path_name()
            if path in seen:
                continue
            seen.add(path)
            meshes.append(mesh)
    return meshes


def assign_starter_skeleton(
    meshes: list[unreal.SkeletalMesh], skeleton: unreal.Skeleton, source_mesh: unreal.SkeletalMesh
) -> list[unreal.SkeletalMesh]:
    assigned: list[unreal.SkeletalMesh] = []
    source_path = source_mesh.get_path_name()
    for mesh in meshes:
        if not mesh:
            continue
        if mesh.get_path_name() == source_path:
            continue
        current = mesh.get_editor_property("skeleton")
        if current == skeleton:
            assigned.append(mesh)
            log(f"mesh already uses starter skeleton: {mesh.get_path_name()}")
            continue

        ok = unreal.DBAEditorAnimationTools.assign_skeleton_to_mesh(mesh, skeleton)
        if ok:
            unreal.EditorAssetLibrary.save_loaded_asset(mesh)
            assigned.append(mesh)
            log(f"mesh skeleton assigned: {mesh.get_path_name()}")
        else:
            log(f"mesh skeleton assign failed: {mesh.get_path_name()}")
    return assigned


def collect_assets_to_retarget(source_skeleton: unreal.Skeleton) -> list[unreal.AssetData]:
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    expected_skeleton_path = source_skeleton.get_path_name()
    expected_skeleton_name = source_skeleton.get_name()
    result: list[unreal.AssetData] = []
    seen = set()

    for root in SOURCE_ANIM_ROOT_CANDIDATES:
        if not unreal.EditorAssetLibrary.does_directory_exist(root):
            continue
        raw_assets = unreal.EditorAssetLibrary.list_assets(root, recursive=True, include_folder=False)
        for object_path in raw_assets:
            package_name = object_path.split(".")[0]
            if package_name in seen:
                continue

            asset_datas = registry.get_assets_by_package_name(
                unreal.Name(package_name), include_only_on_disk_assets=False
            )
            if not asset_datas:
                continue
            asset_data = asset_datas[0]
            class_name = str(asset_data.asset_class_path.asset_name)
            if class_name not in ASSET_CLASSES_FOR_RETARGET:
                continue

            object_path = f"{asset_data.package_name}.{asset_data.asset_name}"
            try:
                skeleton_tag = str(asset_data.get_tag_value("Skeleton") or "")
            except Exception:
                skeleton_tag = ""
            matched_by_tag = expected_skeleton_path in skeleton_tag or expected_skeleton_name in skeleton_tag
            if not matched_by_tag:
                asset_obj = try_load_asset(object_path)
                matched_by_object = False
                if isinstance(asset_obj, unreal.AnimationAsset):
                    asset_skel = asset_obj.get_editor_property("skeleton")
                    matched_by_object = asset_skel == source_skeleton
                elif isinstance(asset_obj, unreal.AnimBlueprint):
                    target_skel = asset_obj.get_editor_property("target_skeleton")
                    matched_by_object = target_skel == source_skeleton
                if not matched_by_object:
                    continue

            seen.add(package_name)
            result.append(asset_data)

        if result:
            log(f"source_anim_root={root} valid_assets={len(result)}")
            return result

    return result


def choose_output_path_for_target_mesh(target_mesh: unreal.SkeletalMesh) -> str:
    mesh_name = target_mesh.get_name()
    clean_name = mesh_name.replace("SKM_", "").replace("SK_", "")
    output_root = f"/Game/DBA/Heroes/Retarget/{clean_name}"
    unreal.EditorAssetLibrary.make_directory(output_root)
    return output_root


def save_generated_assets(generated: list[unreal.AssetData]) -> None:
    for asset_data in generated:
        object_path = _asset_data_to_object_path(asset_data)
        try:
            loaded = unreal.load_asset(object_path)
            if loaded:
                unreal.EditorAssetLibrary.save_loaded_asset(loaded)
        except Exception:
            continue


def run_retarget_for_mesh(
    source_mesh: unreal.SkeletalMesh,
    target_mesh: unreal.SkeletalMesh,
    retargeter: unreal.IKRetargeter,
    assets_to_retarget: list[unreal.AssetData],
) -> None:
    mesh_short = target_mesh.get_name().replace("SKM_", "").replace("SK_", "")
    prefix = f"RTG_{mesh_short}_"
    output_root = choose_output_path_for_target_mesh(target_mesh)

    generated = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
        assets_to_retarget,
        source_mesh,
        target_mesh,
        retargeter,
        "/Game/",
        output_root + "/",
        prefix,
        "",
        True,
        True,
    )

    count = len(generated) if generated else 0
    log(f"retarget target_mesh={target_mesh.get_path_name()} generated={count}")
    if generated:
        save_generated_assets(generated)
        for asset_data in generated:
            log(f"generated {asset_data.package_name}.{asset_data.asset_name}")


def main() -> None:
    source_mesh, skeleton = resolve_source_mesh_and_skeleton()
    retargeter = resolve_retargeter()

    if not isinstance(source_mesh, unreal.SkeletalMesh) or not isinstance(skeleton, unreal.Skeleton):
        warn("no valid starter mannequin source mesh/skeleton found from candidates.")
        for path in SOURCE_MESH_CANDIDATES:
            warn(f"source_mesh_candidate={path}")
        for path in STARTER_SKELETON_CANDIDATES:
            warn(f"skeleton_candidate={path}")
        return
    if not isinstance(retargeter, unreal.IKRetargeter):
        warn("no valid IK Retargeter found from candidates.")
        for path in RETARGETER_CANDIDATES:
            warn(f"retargeter_candidate={path}")
        return

    meshes = gather_target_meshes()
    if not meshes:
        warn("no target skeletal meshes found.")
        return

    assigned_meshes = assign_starter_skeleton(meshes, skeleton, source_mesh)
    if not assigned_meshes:
        warn("no mesh could be assigned starter skeleton.")
        return

    assets_to_retarget = collect_assets_to_retarget(skeleton)
    if not assets_to_retarget:
        warn("no source animation assets found for retarget.")
        warn("checked animation roots:")
        for root in SOURCE_ANIM_ROOT_CANDIDATES:
            warn(f"source_root={root}")
        return
    log(f"assets_to_retarget={len(assets_to_retarget)}")

    for target_mesh in assigned_meshes:
        run_retarget_for_mesh(source_mesh, target_mesh, retargeter, assets_to_retarget)

    log("done")


if __name__ == "__main__":
    main()
