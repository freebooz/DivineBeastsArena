# Copyright FreeboozStudio. All Rights Reserved.
#
# CreateDataAssets.py
#
# UE5 DataAsset 创建脚本
#
# 执行方式:
# py D:/DivineBeastsArena/Scripts/CreateDataAssets.py
#
# 重要:
# 1. 不要直接创建 unreal.DataAsset。
# 2. 不要直接创建 unreal.PrimaryDataAsset。
# 3. unreal.DataAsset / unreal.PrimaryDataAsset 在你的 UE 版本中是 abstract，直接创建会导致保存失败。
# 4. 必须使用你项目中的非抽象 C++ DataAsset 子类。
#
# 推荐 C++ 基类:
#
# UCLASS(BlueprintType)
# class DIVINEBEASTSARENA_API UDBADataAsset : public UPrimaryDataAsset
# {
#     GENERATED_BODY()
# };
#
# Python 加载路径:
# /Script/DivineBeastsArena.DBADataAsset
#

import unreal


# ============================================
# 基础配置
# ============================================

CONTENT_PATH = "/Game"
PROJECT_MODULE_NAME = "DivineBeastsArena"

DELETE_EXISTING_ASSETS_BEFORE_CREATE = True
COLLECT_GARBAGE_AFTER_DELETE = True

# 重要：
# 如果你的 C++ 类是 UDBADataAsset，这里写 DBADataAsset，不要写 UDBADataAsset。
DEFAULT_NATIVE_DATA_ASSET_CLASS_NAME = "DBADataAsset"

# 如果你已经有更细分的 C++ DataAsset 类，可以在下面 DataAsset 类配置区域单独指定。


# ============================================
# 日志
# ============================================

def log_info(msg):
    unreal.log("[CreateDataAssets][INFO] {}".format(msg))


def log_warn(msg):
    unreal.log_warning("[CreateDataAssets][WARN] {}".format(msg))


def log_error(msg):
    unreal.log_error("[CreateDataAssets][ERROR] {}".format(msg))


# ============================================
# 路径工具
# ============================================

def normalize_directory_path(path):
    if not path:
        return CONTENT_PATH

    if not path.startswith("/"):
        path = "/" + path

    return path.rstrip("/")


def make_asset_path(package_path, asset_name):
    package_path = normalize_directory_path(package_path)
    return "{}/{}".format(package_path, asset_name)


def ensure_folder_exists(folder_path):
    folder_path = normalize_directory_path(folder_path)

    if unreal.EditorAssetLibrary.does_directory_exist(folder_path):
        return True

    try:
        success = unreal.EditorAssetLibrary.make_directory(folder_path)

        if success:
            log_info("创建文件夹: {}".format(folder_path))
            return True

        log_error("创建文件夹失败: {}".format(folder_path))
        return False

    except Exception as e:
        log_error("创建文件夹异常: {} - {}".format(folder_path, str(e)))
        return False


# ============================================
# 类加载
# ============================================

def load_native_class(class_name, required=False):
    """
    加载 C++ UObject 类。

    C++:
        UDBADataAsset

    class_name:
        DBADataAsset

    最终路径:
        /Script/DivineBeastsArena.DBADataAsset
    """

    if not class_name:
        if required:
            raise RuntimeError("class_name 为空")
        return None

    class_path = "/Script/{}.{}".format(PROJECT_MODULE_NAME, class_name)

    try:
        loaded_class = unreal.load_class(None, class_path)

        if loaded_class:
            log_info("加载 C++ 类成功: {}".format(class_path))
            return loaded_class

        msg = "加载 C++ 类失败: {}".format(class_path)
        if required:
            raise RuntimeError(msg)

        log_warn(msg)
        return None

    except Exception as e:
        msg = "加载 C++ 类异常: {} - {}".format(class_path, str(e))
        if required:
            raise RuntimeError(msg)

        log_warn(msg)
        return None


def is_invalid_abstract_base_class(asset_class):
    """
    阻止使用抽象基类创建资产。
    """

    if asset_class is None:
        return True

    if asset_class == unreal.DataAsset:
        return True

    if asset_class == unreal.PrimaryDataAsset:
        return True

    return False


# ============================================
# DataAsset 类配置
# ============================================
#
# 必须使用非抽象 C++ 类。
#
# 最简单做法：
# 所有 DataAsset 先统一使用 UDBADataAsset。
#
# 如果你有更细分的类，可改成：
#
# FIVE_CAMP_DATA_ASSET_CLASS = load_native_class("DBAFiveCampDataAsset", required=True)
# ELEMENT_DATA_ASSET_CLASS = load_native_class("DBAElementDataAsset", required=True)
# ZODIAC_DATA_ASSET_CLASS = load_native_class("DBAZodiacDataAsset", required=True)
# HERO_DATA_ASSET_CLASS = load_native_class("DBAHeroDataAsset", required=True)
# ABILITY_SET_DATA_ASSET_CLASS = load_native_class("DBAAbilitySetDataAsset", required=True)
# POOL_CONFIG_DATA_ASSET_CLASS = load_native_class("DBAObjectPoolConfigDataAsset", required=True)
# SKILL_GROUP_DATA_ASSET_CLASS = load_native_class("DBAFixedSkillGroupDataAsset", required=True)
#

DEFAULT_DATA_ASSET_CLASS = load_native_class(
    DEFAULT_NATIVE_DATA_ASSET_CLASS_NAME,
    required=True
)

FIVE_CAMP_DATA_ASSET_CLASS = DEFAULT_DATA_ASSET_CLASS
ELEMENT_DATA_ASSET_CLASS = DEFAULT_DATA_ASSET_CLASS
ZODIAC_DATA_ASSET_CLASS = DEFAULT_DATA_ASSET_CLASS
HERO_DATA_ASSET_CLASS = DEFAULT_DATA_ASSET_CLASS
ABILITY_SET_DATA_ASSET_CLASS = DEFAULT_DATA_ASSET_CLASS
POOL_CONFIG_DATA_ASSET_CLASS = DEFAULT_DATA_ASSET_CLASS
SKILL_GROUP_DATA_ASSET_CLASS = DEFAULT_DATA_ASSET_CLASS


# ============================================
# 删除 / GC / 保存
# ============================================

def collect_garbage():
    if not COLLECT_GARBAGE_AFTER_DELETE:
        return

    try:
        unreal.SystemLibrary.collect_garbage()
    except Exception as e:
        log_warn("执行 GC 失败，可忽略: {}".format(str(e)))


def delete_asset_if_exists(asset_path):
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        return True

    log_warn("资产已存在，准备删除: {}".format(asset_path))

    try:
        deleted = unreal.EditorAssetLibrary.delete_asset(asset_path)

        if deleted:
            log_info("删除资产成功: {}".format(asset_path))
            collect_garbage()
            return True

        log_error("删除资产失败: {}".format(asset_path))
        return False

    except Exception as e:
        log_error("删除资产异常: {} - {}".format(asset_path, str(e)))
        return False


def save_package_for_asset(asset):
    """
    使用 EditorLoadingAndSavingUtils.save_packages 保存资产所在 Package。
    比 save_loaded_asset 在部分 DataAsset 场景下更直接。
    """

    if not asset:
        return False

    try:
        package = asset.get_outermost()
        saved = unreal.EditorLoadingAndSavingUtils.save_packages(
            [package],
            only_dirty=False
        )

        return bool(saved)

    except Exception as e:
        log_error("save_package_for_asset 异常: {} - {}".format(asset.get_name(), str(e)))
        return False


def save_loaded_asset_safely(asset, asset_path):
    if not asset:
        log_error("保存失败，asset 为 None: {}".format(asset_path))
        return False

    try:
        saved = unreal.EditorAssetLibrary.save_loaded_asset(
            asset,
            only_if_is_dirty=False
        )

        if saved:
            log_info("保存资产成功: {}".format(asset_path))
            return True

        log_warn("save_loaded_asset 返回 False，尝试 save_packages: {}".format(asset_path))

        saved_by_package = save_package_for_asset(asset)

        if saved_by_package:
            log_info("通过 save_packages 保存资产成功: {}".format(asset_path))
            return True

        log_error("保存资产失败: {}".format(asset_path))
        return False

    except Exception as e:
        log_error("保存资产异常: {} - {}".format(asset_path, str(e)))
        return False


# ============================================
# DataAsset 创建
# ============================================

def create_data_asset(package_path, asset_name, data_asset_class):
    package_path = normalize_directory_path(package_path)
    asset_path = make_asset_path(package_path, asset_name)

    log_info("-" * 60)
    log_info("准备创建 DataAsset: {}".format(asset_path))

    if is_invalid_abstract_base_class(data_asset_class):
        log_error("DataAsset 类无效或是抽象基类，禁止创建: {}".format(asset_path))
        log_error("请使用项目中的非抽象 C++ 类，例如 /Script/DivineBeastsArena.DBADataAsset")
        return None

    if not ensure_folder_exists(package_path):
        log_error("目录不存在且创建失败，跳过: {}".format(package_path))
        return None

    if DELETE_EXISTING_ASSETS_BEFORE_CREATE:
        if not delete_asset_if_exists(asset_path):
            log_error("删除已有资产失败，跳过创建: {}".format(asset_path))
            return None
    else:
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            log_warn("资产已存在，跳过: {}".format(asset_path))
            return unreal.EditorAssetLibrary.load_asset(asset_path)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    try:
        factory = unreal.DataAssetFactory()
        factory.set_editor_property("data_asset_class", data_asset_class)

        created_asset = asset_tools.create_asset(
            asset_name=asset_name,
            package_path=package_path,
            asset_class=data_asset_class,
            factory=factory
        )

        if not created_asset:
            log_error("创建 DataAsset 失败: {}".format(asset_path))
            return None

        log_info("创建 DataAsset 成功: {}".format(asset_path))

        saved = save_loaded_asset_safely(created_asset, asset_path)

        if not saved:
            log_error("资产已创建但保存失败: {}".format(asset_path))

        return created_asset

    except Exception as e:
        log_error("创建 DataAsset 异常: {} - {}".format(asset_path, str(e)))
        return None


# ============================================
# 文件夹
# ============================================

def create_folders():
    log_info("开始创建文件夹结构...")

    folders = [
        "{}/DBA".format(CONTENT_PATH),
        "{}/DBA/FiveCamps".format(CONTENT_PATH),
        "{}/DBA/Elements".format(CONTENT_PATH),
        "{}/DBA/Zodiacs".format(CONTENT_PATH),
        "{}/DBA/Heroes".format(CONTENT_PATH),
        "{}/DBA/AbilitySets".format(CONTENT_PATH),
        "{}/DBA/Data".format(CONTENT_PATH),
        "{}/DBA/Data/PoolConfigs".format(CONTENT_PATH),
        "{}/DBA/Data/SkillGroups".format(CONTENT_PATH),
        "{}/Maps".format(CONTENT_PATH),
        "{}/Maps/Frontend".format(CONTENT_PATH),
        "{}/Maps/Lobby".format(CONTENT_PATH),
    ]

    for folder in folders:
        ensure_folder_exists(folder)

    log_info("文件夹结构创建完成!")


# ============================================
# FiveCamp
# ============================================

def create_five_camp_assets():
    log_info("开始创建 FiveCamp 数据资产...")

    camps = [
        ("FiveCamp_Fire", "火之阵营", "Fire"),
        ("FiveCamp_Water", "水之阵营", "Water"),
        ("FiveCamp_Earth", "土之阵营", "Earth"),
        ("FiveCamp_Gold", "金之阵营", "Gold"),
        ("FiveCamp_Wood", "木之阵营", "Wood"),
    ]

    for camp_id, camp_name, element in camps:
        create_data_asset(
            "{}/DBA/FiveCamps".format(CONTENT_PATH),
            "DA_{}".format(camp_id),
            FIVE_CAMP_DATA_ASSET_CLASS
        )

    log_info("FiveCamp 数据资产创建完成!")


# ============================================
# Element
# ============================================

def create_element_assets():
    log_info("开始创建 Element 数据资产...")

    elements = [
        ("Element_Fire", "火", "Fire"),
        ("Element_Water", "水", "Water"),
        ("Element_Earth", "土", "Earth"),
        ("Element_Gold", "金", "Gold"),
        ("Element_Wood", "木", "Wood"),
    ]

    for elem_id, elem_name, tag in elements:
        create_data_asset(
            "{}/DBA/Elements".format(CONTENT_PATH),
            "DA_{}".format(elem_id),
            ELEMENT_DATA_ASSET_CLASS
        )

    log_info("Element 数据资产创建完成!")


# ============================================
# Zodiac
# ============================================

def create_zodiac_assets():
    log_info("开始创建 Zodiac 数据资产...")

    zodiacs = [
        ("Zodiac_Aries", "白羊座"),
        ("Zodiac_Taurus", "金牛座"),
        ("Zodiac_Gemini", "双子座"),
        ("Zodiac_Cancer", "巨蟹座"),
        ("Zodiac_Leo", "狮子座"),
        ("Zodiac_Virgo", "处女座"),
        ("Zodiac_Libra", "天秤座"),
        ("Zodiac_Scorpio", "天蝎座"),
        ("Zodiac_Sagittarius", "射手座"),
        ("Zodiac_Capricorn", "摩羯座"),
        ("Zodiac_Aquarius", "水瓶座"),
        ("Zodiac_Pisces", "双鱼座"),
    ]

    for zodiac_id, zodiac_name in zodiacs:
        create_data_asset(
            "{}/DBA/Zodiacs".format(CONTENT_PATH),
            "DA_{}".format(zodiac_id),
            ZODIAC_DATA_ASSET_CLASS
        )

    log_info("Zodiac 数据资产创建完成!")


# ============================================
# Hero
# ============================================

def create_hero_assets():
    log_info("开始创建 Hero 数据资产...")

    heroes = [
        ("Hero_FireLion", "火之阵营 - 炽焰狮", "Fire", "FiveCamp_Fire"),
        ("Hero_WaterDragon", "水之阵营 - 碧波龙", "Water", "FiveCamp_Water"),
        ("Hero_EarthBear", "土之阵营 - 岩甲熊", "Earth", "FiveCamp_Earth"),
        ("Hero_GoldPhoenix", "金之阵营 - 金光凤", "Gold", "FiveCamp_Gold"),
        ("Hero_WoodCrane", "木之阵营 - 灵木鹤", "Wood", "FiveCamp_Wood"),
    ]

    for hero_id, hero_name, element, five_camp in heroes:
        create_data_asset(
            "{}/DBA/Heroes".format(CONTENT_PATH),
            "DA_{}".format(hero_id),
            HERO_DATA_ASSET_CLASS
        )

    log_info("Hero 数据资产创建完成!")


# ============================================
# AbilitySet
# ============================================

def create_ability_set_assets():
    log_info("开始创建 AbilitySet 数据资产...")

    ability_sets = [
        ("AS_FireLion_Passive", "火狮被动技能"),
        ("AS_FireLion_Q", "火狮 Q 技能"),
        ("AS_FireLion_W", "火狮 W 技能"),
        ("AS_FireLion_E", "火狮 E 技能"),
        ("AS_FireLion_R", "火狮 R 大招"),
        ("AS_WaterDragon_Passive", "水龙被动技能"),
        ("AS_WaterDragon_Q", "水龙 Q 技能"),
        ("AS_EarthBear_Passive", "土熊被动技能"),
        ("AS_EarthBear_Q", "土熊 Q 技能"),
        ("AS_GoldPhoenix_Passive", "金凤被动技能"),
        ("AS_GoldPhoenix_Q", "金凤 Q 技能"),
        ("AS_WoodCrane_Passive", "木鹤被动技能"),
        ("AS_WoodCrane_Q", "木鹤 Q 技能"),
    ]

    for as_id, desc in ability_sets:
        create_data_asset(
            "{}/DBA/AbilitySets".format(CONTENT_PATH),
            "DA_{}".format(as_id),
            ABILITY_SET_DATA_ASSET_CLASS
        )

    log_info("AbilitySet 数据资产创建完成!")


# ============================================
# ObjectPool
# ============================================

def create_pool_config_assets():
    log_info("开始创建 ObjectPool 配置数据资产...")

    pool_types = [
        ("OPT_Projectile", "弹道对象池"),
        ("OPT_Effect", "特效对象池"),
        ("OPT_UI", "UI 对象池"),
    ]

    for pool_id, desc in pool_types:
        create_data_asset(
            "{}/DBA/Data/PoolConfigs".format(CONTENT_PATH),
            "DA_{}".format(pool_id),
            POOL_CONFIG_DATA_ASSET_CLASS
        )

    log_info("ObjectPool 配置数据资产创建完成!")


# ============================================
# FixedSkillGroup
# ============================================

def create_skill_group_assets():
    log_info("开始创建 FixedSkillGroup 数据资产...")

    skill_groups = [
        ("FSG_Standard_5v5_1", "标准 5v5 配置组 1"),
        ("FSG_Standard_5v5_2", "标准 5v5 配置组 2"),
        ("FSG_Standard_5v5_3", "标准 5v5 配置组 3"),
        ("FSG_Standard_5v5_4", "标准 5v5 配置组 4"),
        ("FSG_Standard_5v5_5", "标准 5v5 配置组 5"),
    ]

    for fsg_id, desc in skill_groups:
        create_data_asset(
            "{}/DBA/Data/SkillGroups".format(CONTENT_PATH),
            "DA_{}".format(fsg_id),
            SKILL_GROUP_DATA_ASSET_CLASS
        )

    log_info("FixedSkillGroup 数据资产创建完成!")


# ============================================
# Summary
# ============================================

def print_summary():
    log_info("=" * 60)
    log_info("DataAsset 生成结果检查")
    log_info("=" * 60)

    assets = []

    for name in [
        "FiveCamp_Fire",
        "FiveCamp_Water",
        "FiveCamp_Earth",
        "FiveCamp_Gold",
        "FiveCamp_Wood",
    ]:
        assets.append("/Game/DBA/FiveCamps/DA_{}".format(name))

    for name in [
        "Element_Fire",
        "Element_Water",
        "Element_Earth",
        "Element_Gold",
        "Element_Wood",
    ]:
        assets.append("/Game/DBA/Elements/DA_{}".format(name))

    for name in [
        "Zodiac_Aries",
        "Zodiac_Taurus",
        "Zodiac_Gemini",
        "Zodiac_Cancer",
        "Zodiac_Leo",
        "Zodiac_Virgo",
        "Zodiac_Libra",
        "Zodiac_Scorpio",
        "Zodiac_Sagittarius",
        "Zodiac_Capricorn",
        "Zodiac_Aquarius",
        "Zodiac_Pisces",
    ]:
        assets.append("/Game/DBA/Zodiacs/DA_{}".format(name))

    for name in [
        "Hero_FireLion",
        "Hero_WaterDragon",
        "Hero_EarthBear",
        "Hero_GoldPhoenix",
        "Hero_WoodCrane",
    ]:
        assets.append("/Game/DBA/Heroes/DA_{}".format(name))

    for name in [
        "AS_FireLion_Passive",
        "AS_FireLion_Q",
        "AS_FireLion_W",
        "AS_FireLion_E",
        "AS_FireLion_R",
        "AS_WaterDragon_Passive",
        "AS_WaterDragon_Q",
        "AS_EarthBear_Passive",
        "AS_EarthBear_Q",
        "AS_GoldPhoenix_Passive",
        "AS_GoldPhoenix_Q",
        "AS_WoodCrane_Passive",
        "AS_WoodCrane_Q",
    ]:
        assets.append("/Game/DBA/AbilitySets/DA_{}".format(name))

    for name in [
        "OPT_Projectile",
        "OPT_Effect",
        "OPT_UI",
    ]:
        assets.append("/Game/DBA/Data/PoolConfigs/DA_{}".format(name))

    for name in [
        "FSG_Standard_5v5_1",
        "FSG_Standard_5v5_2",
        "FSG_Standard_5v5_3",
        "FSG_Standard_5v5_4",
        "FSG_Standard_5v5_5",
    ]:
        assets.append("/Game/DBA/Data/SkillGroups/DA_{}".format(name))

    for asset_path in assets:
        exists = unreal.EditorAssetLibrary.does_asset_exist(asset_path)
        status = "存在" if exists else "不存在"
        log_info("[{}] {}".format(status, asset_path))

    log_info("=" * 60)


# ============================================
# 主函数
# ============================================

def main():
    log_info("=" * 60)
    log_info("开始创建所有 DataAsset")
    log_info("=" * 60)

    log_warn("注意：本脚本会在创建资产前删除同名 DataAsset。")
    log_warn("注意：本脚本必须使用非抽象 C++ DataAsset 类。")
    log_warn("当前默认类: /Script/{}.{}".format(PROJECT_MODULE_NAME, DEFAULT_NATIVE_DATA_ASSET_CLASS_NAME))

    create_folders()

    create_five_camp_assets()
    create_element_assets()
    create_zodiac_assets()
    create_hero_assets()
    create_ability_set_assets()
    create_pool_config_assets()
    create_skill_group_assets()

    print_summary()

    log_info("=" * 60)
    log_info("DataAsset 创建完成!")
    log_info("=" * 60)


# ============================================
# 入口
# ============================================

if __name__ == "__main__":
    main()
