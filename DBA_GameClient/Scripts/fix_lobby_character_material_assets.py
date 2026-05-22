#!/usr/bin/env python3
# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


BROKEN_PREFIX = "/Game/Characters/Mannequins/"
FIXED_PREFIX = "/Game/DBA/Characters/Mannequins/"

MATERIAL_ASSETS = [
    "/Game/DBA/Characters/Mannequins/Materials/M_Mannequin",
    "/Game/DBA/Characters/Mannequins/Materials/Instances/Manny/MI_Manny_01",
    "/Game/DBA/Characters/Mannequins/Materials/Instances/Manny/MI_Manny_02",
    "/Game/DBA/Characters/Mannequins/Materials/Manny/MI_Manny_01_New",
    "/Game/DBA/Characters/Mannequins/Materials/Manny/MI_Manny_02_New",
    "/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Rat",
    "/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Ox",
    "/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Tiger",
    "/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Rabbit",
    "/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Dragon",
    "/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Snake",
    "/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Horse",
    "/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Goat",
    "/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Monkey",
    "/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Rooster",
    "/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Dog",
    "/Game/DBA/Zodiacs/Chinese/Visuals/Materials/Instances/MI_DBA_Zodiac_Pig",
]

TEXTURE_ALIASES = [
    "Manny/T_Manny_01_ASAOPMASK_MSK",
    "Manny/T_Manny_01_BN",
    "Manny/T_Manny_01_CCRCCPlastic_MSK",
    "Manny/T_Manny_01_D",
    "Manny/T_Manny_01_MRA",
    "Manny/T_Manny_01_MSR_MSK",
    "Manny/T_Manny_01_N",
    "Manny/T_Manny_01_Tan",
    "Manny/T_Manny_02_ASAOPMASK_MSK",
    "Manny/T_Manny_02_BN",
    "Manny/T_Manny_02_CCRCCPlastic_MSK",
    "Manny/T_Manny_02_D",
    "Manny/T_Manny_02_MRA",
    "Manny/T_Manny_02_MSR_MSK",
    "Manny/T_Manny_02_N",
    "Manny/T_Manny_02_Tan",
    "Shared/T_UE_Logo_M",
]

ASSET_ALIASES = [
    (
        "/Game/DBA/Characters/Mannequins/Materials/M_Mannequin",
        "/Game/Characters/Mannequins/Materials/M_Mannequin",
    ),
    (
        "/Game/DBA/Characters/Mannequins/Materials/Manny/MI_Manny_01_New",
        "/Game/Characters/Mannequins/Materials/Manny/MI_Manny_01_New",
    ),
    (
        "/Game/DBA/Characters/Mannequins/Materials/Manny/MI_Manny_02_New",
        "/Game/Characters/Mannequins/Materials/Manny/MI_Manny_02_New",
    ),
]


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        unreal.log_warning(f"Missing asset: {path}")
    return asset


def ensure_texture_alias(alias_path):
    broken_path = BROKEN_PREFIX + "Textures/" + alias_path
    fixed_path = FIXED_PREFIX + "Textures/" + alias_path

    if unreal.EditorAssetLibrary.does_asset_exist(broken_path):
        return False

    fixed_texture = load_asset(fixed_path)
    if not fixed_texture:
        unreal.log_warning(f"Source texture for alias is missing: {fixed_path}")
        return False

    duplicated = unreal.EditorAssetLibrary.duplicate_asset(fixed_path, broken_path)
    if not duplicated:
        unreal.log_warning(f"Failed to create texture alias: {fixed_path} -> {broken_path}")
        return False

    unreal.EditorAssetLibrary.save_loaded_asset(duplicated)
    unreal.log(f"Created texture alias for legacy material reference: {fixed_path} -> {broken_path}")
    return True


def ensure_texture_aliases():
    changed = False
    for alias_path in TEXTURE_ALIASES:
        changed = ensure_texture_alias(alias_path) or changed
    return changed


def ensure_asset_alias(source_path, alias_path):
    if unreal.EditorAssetLibrary.does_asset_exist(alias_path):
        return False

    source_asset = load_asset(source_path)
    if not source_asset:
        unreal.log_warning(f"Source asset for alias is missing: {source_path}")
        return False

    duplicated = unreal.EditorAssetLibrary.duplicate_asset(source_path, alias_path)
    if not duplicated:
        unreal.log_warning(f"Failed to create asset alias: {source_path} -> {alias_path}")
        return False

    unreal.EditorAssetLibrary.save_loaded_asset(duplicated)
    unreal.log(f"Created asset alias for legacy material reference: {source_path} -> {alias_path}")
    return True


def ensure_asset_aliases():
    changed = False
    for source_path, alias_path in ASSET_ALIASES:
        changed = ensure_asset_alias(source_path, alias_path) or changed
    return changed


def fix_texture_sample(expression):
    if not hasattr(expression, "texture"):
        return False

    texture = expression.get_editor_property("texture")
    if not texture:
        return False

    texture_path = texture.get_path_name().split(".")[0]
    if not texture_path.startswith(BROKEN_PREFIX):
        return False

    fixed_path = texture_path.replace(BROKEN_PREFIX, FIXED_PREFIX, 1)
    fixed_texture = load_asset(fixed_path)
    if not fixed_texture:
        unreal.log_warning(f"Replacement texture not found: {fixed_path}")
        return False

    expression.set_editor_property("texture", fixed_texture)
    unreal.log(f"Repointed texture sample: {texture_path} -> {fixed_path}")
    return True


def fix_material(material):
    changed = False
    try:
        expressions = unreal.MaterialEditingLibrary.get_material_expressions(material)
        for expression in expressions:
            changed = fix_texture_sample(expression) or changed
    except AttributeError:
        unreal.log_warning("Material expression list API is unavailable; legacy texture aliases keep M_Mannequin dependencies loadable.")

    if changed:
        unreal.MaterialEditingLibrary.recompile_material(material)
        unreal.EditorAssetLibrary.save_loaded_asset(material)
    return changed


def save_material_family():
    for path in MATERIAL_ASSETS:
        asset = load_asset(path)
        if asset:
            unreal.EditorAssetLibrary.save_loaded_asset(asset)


def save_rosales_animation_assets():
    changed = False
    mesh = load_asset("/Game/DBA/Characters/Rosales/Meshes/SK_Rosales")
    if mesh:
        skeleton = None
        try:
            skeleton = mesh.get_editor_property("skeleton")
        except Exception as exc:
            unreal.log_warning(f"Unable to read Rosales skeleton from mesh: {exc}")

        if skeleton:
            unreal.EditorAssetLibrary.save_loaded_asset(skeleton)
            unreal.log(f"Saved Rosales skeleton for lobby animation compatibility: {skeleton.get_path_name()}")
            changed = True

    anim_blueprint = load_asset("/Game/DBA/Characters/Rosales/AnimationBP/ABP_Rosales")
    if anim_blueprint:
        try:
            unreal.load_class(None, "/Game/DBA/Characters/Rosales/AnimationBP/ABP_Rosales.ABP_Rosales_C")
        except Exception as exc:
            unreal.log_warning(f"Unable to load Rosales generated animation class: {exc}")

        if mesh:
            unreal.EditorAssetLibrary.save_loaded_asset(mesh)
        unreal.EditorAssetLibrary.save_loaded_asset(anim_blueprint)
        unreal.log("Saved Rosales animation blueprint used by lobby/preview display.")
        changed = True
    return changed


def main():
    changed_any = ensure_texture_aliases()
    changed_any = ensure_asset_aliases() or changed_any
    mannequin = load_asset("/Game/DBA/Characters/Mannequins/Materials/M_Mannequin")
    if isinstance(mannequin, unreal.Material):
        changed_any = fix_material(mannequin) or changed_any
    elif mannequin:
        unreal.log_warning("M_Mannequin is not a Material; skipping expression rewrite.")

    save_material_family()
    changed_any = save_rosales_animation_assets() or changed_any
    if changed_any:
        unreal.log("Lobby character material asset references repaired and saved.")
    else:
        unreal.log("No broken material texture references found; material family saved.")


if __name__ == "__main__":
    main()
