# Copyright FreeboozStudio. All Rights Reserved.
#
# GenerateBlueprints.py
#
# UE5 Blueprint / WidgetBlueprint / Map 自动生成脚本
#
# 在 UE Editor 的 Python Console 中执行：
# py D:/DivineBeastsArena/Scripts/GenerateBlueprints.py
#
# 重要说明：
# 1. 本脚本会在创建资产前删除同路径同名资产。
# 2. 如果你已经手动编辑过这些蓝图或地图，请先备份。
# 3. 如果你有自定义 C++ 父类，请修改下方“父类配置”区域。
#

import unreal


# ============================================
# 基础配置
# ============================================

CONTENT_PATH = "/Game"
PROJECT_NAME = "DivineBeastsArena"

# 是否创建地图
CREATE_MAPS = True

# 是否创建普通 Blueprint
CREATE_BLUEPRINTS = True

# 是否创建 Widget Blueprint
CREATE_WIDGET_BLUEPRINTS = True

# 删除已存在资产后是否执行 GC
COLLECT_GARBAGE_AFTER_DELETE = True


# ============================================
# 日志工具
# ============================================

def log_info(msg):
    unreal.log("[GenerateBlueprints][INFO] {}".format(msg))


def log_warn(msg):
    unreal.log_warning("[GenerateBlueprints][WARN] {}".format(msg))


def log_error(msg):
    unreal.log_error("[GenerateBlueprints][ERROR] {}".format(msg))


# ============================================
# 路径工具
# ============================================

def normalize_directory_path(path):
    """规范化 UE 目录路径，例如 /Game/Blueprints"""
    if not path:
        return "/Game"

    if not path.startswith("/"):
        path = "/" + path

    return path.rstrip("/")


def make_asset_path(package_path, asset_name):
    """拼接资产路径，例如 /Game/Blueprints/GM_Lobby"""
    package_path = normalize_directory_path(package_path)
    return "{}/{}".format(package_path, asset_name)


def ensure_folder_exists(path):
    """确保目录存在"""
    path = normalize_directory_path(path)

    if unreal.EditorAssetLibrary.does_directory_exist(path):
        return True

    result = unreal.EditorAssetLibrary.make_directory(path)

    if result:
        log_info("创建文件夹: {}".format(path))
        return True

    log_error("创建文件夹失败: {}".format(path))
    return False


def ensure_all_folders():
    """创建本脚本需要的全部文件夹"""

    folders = [
        "{}/Maps".format(CONTENT_PATH),
        "{}/Maps/Frontend".format(CONTENT_PATH),
        "{}/Maps/Lobby".format(CONTENT_PATH),
        "{}/Maps/Arena".format(CONTENT_PATH),

        "{}/Blueprints".format(CONTENT_PATH),

        "{}/DBA".format(CONTENT_PATH),
        "{}/DBA/Heroes".format(CONTENT_PATH),

        "{}/UI".format(CONTENT_PATH),
        "{}/UI/Frontend".format(CONTENT_PATH),
        "{}/UI/Frontend/MainMenu".format(CONTENT_PATH),
        "{}/UI/Frontend/FiveCampSelect".format(CONTENT_PATH),
        "{}/UI/Frontend/PartyPanel".format(CONTENT_PATH),
        "{}/UI/Frontend/QueueStatus".format(CONTENT_PATH),
    ]

    log_info("开始创建文件夹结构...")

    for folder in folders:
        ensure_folder_exists(folder)

    log_info("文件夹结构创建完成。")


# ============================================
# 资产删除工具
# ============================================

def collect_garbage():
    """执行垃圾回收，减少删除后对象残留"""
    if not COLLECT_GARBAGE_AFTER_DELETE:
        return

    try:
        unreal.SystemLibrary.collect_garbage()
    except Exception as e:
        log_warn("执行 GC 失败，可忽略: {}".format(str(e)))


def delete_asset_if_exists(asset_path):
    """
    如果资产存在，则删除。

    asset_path 示例：
        /Game/Blueprints/GM_Lobby
        /Game/UI/Frontend/MainMenu/WB_MainMenu
        /Game/Maps/Lobby/LobbyMap
    """

    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        return True

    log_warn("资产已存在，准备删除: {}".format(asset_path))

    try:
        # 尝试加载一下，确保路径有效
        loaded_asset = unreal.EditorAssetLibrary.load_asset(asset_path)

        if loaded_asset:
            log_info("已加载待删除资产: {}".format(asset_path))

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


# ============================================
# 父类配置
# ============================================

def load_class_or_fallback(class_path, fallback_class):
    """
    尝试加载 C++ 类或蓝图类，失败则使用 fallback_class。

    C++ 类路径示例：
        /Script/DivineBeastsArena.DBALobbyGameMode

    蓝图类路径示例：
        /Game/Blueprints/BP_MyParent.BP_MyParent_C
    """

    if not class_path:
        return fallback_class

    try:
        cls = unreal.load_class(None, class_path)

        if cls:
            log_info("加载类成功: {}".format(class_path))
            return cls

        log_warn("加载类失败，使用默认父类: {}".format(class_path))
        return fallback_class

    except Exception as e:
        log_warn("加载类异常，使用默认父类: {} - {}".format(class_path, str(e)))
        return fallback_class


# 如果你有自己的 C++ 父类，可以在这里修改。
# 例如：
# LOBBY_GAMEMODE_PARENT_CLASS = load_class_or_fallback(
#     "/Script/DivineBeastsArena.DBALobbyGameMode",
#     unreal.GameModeBase
# )

LOBBY_GAMEMODE_PARENT_CLASS = load_class_or_fallback(
    None,
    unreal.GameModeBase
)

ARENA_GAMEMODE_PARENT_CLASS = load_class_or_fallback(
    None,
    unreal.GameModeBase
)

MAIN_PLAYER_CONTROLLER_PARENT_CLASS = load_class_or_fallback(
    None,
    unreal.PlayerController
)

HOST_PLAYER_CONTROLLER_PARENT_CLASS = load_class_or_fallback(
    None,
    unreal.PlayerController
)

GAME_INSTANCE_PARENT_CLASS = load_class_or_fallback(
    None,
    unreal.GameInstance
)

HERO_CHARACTER_PARENT_CLASS = load_class_or_fallback(
    None,
    unreal.Character
)

WIDGET_PARENT_CLASS = load_class_or_fallback(
    None,
    unreal.UserWidget
)


# ============================================
# Blueprint 创建工具
# ============================================

def compile_blueprint_safely(blueprint, asset_path):
    """安全编译蓝图"""
    if not blueprint:
        return False

    try:
        unreal.KismetEditorUtilities.compile_blueprint(blueprint)
        log_info("编译 Blueprint 成功: {}".format(asset_path))
        return True
    except Exception as e:
        log_warn("编译 Blueprint 失败或跳过: {} - {}".format(asset_path, str(e)))
        return False


def save_loaded_asset_safely(asset, asset_path):
    """安全保存已加载资产"""
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

        log_error("保存资产失败: {}".format(asset_path))
        return False

    except Exception as e:
        log_error("保存资产异常: {} - {}".format(asset_path, str(e)))
        return False


def create_blueprint_asset(package_path, asset_name, parent_class):
    """
    创建普通 Blueprint。

    创建前会先删除同名资产。

    示例：
        create_blueprint_asset("/Game/Blueprints", "GM_Lobby", unreal.GameModeBase)
    """

    package_path = normalize_directory_path(package_path)
    asset_path = make_asset_path(package_path, asset_name)

    log_info("-" * 60)
    log_info("准备创建 Blueprint: {}".format(asset_path))

    if not ensure_folder_exists(package_path):
        log_error("目录不存在且创建失败，跳过: {}".format(package_path))
        return None

    # 重点：创建前先删除
    if not delete_asset_if_exists(asset_path):
        log_error("删除已有 Blueprint 失败，跳过创建: {}".format(asset_path))
        return None

    if parent_class is None:
        log_error("父类为空，无法创建 Blueprint: {}".format(asset_path))
        return None

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    try:
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", parent_class)

        blueprint = asset_tools.create_asset(
            asset_name=asset_name,
            package_path=package_path,
            asset_class=unreal.Blueprint,
            factory=factory
        )

        if not blueprint:
            log_error("创建 Blueprint 失败: {}".format(asset_path))
            return None

        log_info("创建 Blueprint 成功: {}".format(asset_path))

        compile_blueprint_safely(blueprint, asset_path)
        save_loaded_asset_safely(blueprint, asset_path)

        return blueprint

    except Exception as e:
        log_error("创建 Blueprint 异常: {} - {}".format(asset_path, str(e)))
        return None


def create_widget_blueprint_asset(package_path, asset_name, parent_class=None):
    """
    创建 Widget Blueprint。

    创建前会先删除同名资产。

    示例：
        create_widget_blueprint_asset("/Game/UI/Frontend/MainMenu", "WB_MainMenu")
    """

    package_path = normalize_directory_path(package_path)
    asset_path = make_asset_path(package_path, asset_name)

    if parent_class is None:
        parent_class = WIDGET_PARENT_CLASS

    log_info("-" * 60)
    log_info("准备创建 Widget Blueprint: {}".format(asset_path))

    if not ensure_folder_exists(package_path):
        log_error("目录不存在且创建失败，跳过: {}".format(package_path))
        return None

    # 重点：创建前先删除
    if not delete_asset_if_exists(asset_path):
        log_error("删除已有 Widget Blueprint 失败，跳过创建: {}".format(asset_path))
        return None

    if parent_class is None:
        log_error("Widget 父类为空，无法创建: {}".format(asset_path))
        return None

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    try:
        # 优先使用 WidgetBlueprintFactory
        if hasattr(unreal, "WidgetBlueprintFactory") and hasattr(unreal, "WidgetBlueprint"):
            factory = unreal.WidgetBlueprintFactory()
            factory.set_editor_property("parent_class", parent_class)
            asset_class = unreal.WidgetBlueprint
        else:
            # 备用方案：使用普通 BlueprintFactory + UserWidget
            log_warn("当前 UE Python 未找到 WidgetBlueprintFactory，使用 BlueprintFactory 备用方案。")
            factory = unreal.BlueprintFactory()
            factory.set_editor_property("parent_class", parent_class)
            asset_class = unreal.Blueprint

        widget_bp = asset_tools.create_asset(
            asset_name=asset_name,
            package_path=package_path,
            asset_class=asset_class,
            factory=factory
        )

        if not widget_bp:
            log_error("创建 Widget Blueprint 失败: {}".format(asset_path))
            return None

        log_info("创建 Widget Blueprint 成功: {}".format(asset_path))

        compile_blueprint_safely(widget_bp, asset_path)
        save_loaded_asset_safely(widget_bp, asset_path)

        return widget_bp

    except Exception as e:
        log_error("创建 Widget Blueprint 异常: {} - {}".format(asset_path, str(e)))
        return None


# ============================================
# Blueprint 创建列表
# ============================================

def create_game_blueprints():
    """创建 GameMode / PlayerController / GameInstance 蓝图"""

    log_info("=" * 60)
    log_info("开始创建基础游戏 Blueprint...")
    log_info("=" * 60)

    create_blueprint_asset(
        "{}/Blueprints".format(CONTENT_PATH),
        "GM_Lobby",
        LOBBY_GAMEMODE_PARENT_CLASS
    )

    create_blueprint_asset(
        "{}/Blueprints".format(CONTENT_PATH),
        "GM_Arena",
        ARENA_GAMEMODE_PARENT_CLASS
    )

    create_blueprint_asset(
        "{}/Blueprints".format(CONTENT_PATH),
        "PC_Main",
        MAIN_PLAYER_CONTROLLER_PARENT_CLASS
    )

    create_blueprint_asset(
        "{}/Blueprints".format(CONTENT_PATH),
        "PC_Host",
        HOST_PLAYER_CONTROLLER_PARENT_CLASS
    )

    create_blueprint_asset(
        "{}/Blueprints".format(CONTENT_PATH),
        "GI_Main",
        GAME_INSTANCE_PARENT_CLASS
    )

    log_info("基础游戏 Blueprint 创建完成。")


def create_hero_blueprints():
    """创建英雄 Character 蓝图"""

    log_info("=" * 60)
    log_info("开始创建英雄 Character Blueprint...")
    log_info("=" * 60)

    hero_blueprints = [
        ("BP_Hero_FireLion", "火狮英雄蓝图"),
        ("BP_Hero_WaterDragon", "水龙英雄蓝图"),
        ("BP_Hero_EarthBear", "土熊英雄蓝图"),
        ("BP_Hero_GoldPhoenix", "金凤英雄蓝图"),
        ("BP_Hero_WoodCrane", "木鹤英雄蓝图"),
    ]

    package_path = "{}/DBA/Heroes".format(CONTENT_PATH)

    for asset_name, desc in hero_blueprints:
        log_info("创建英雄蓝图: {} - {}".format(asset_name, desc))
        create_blueprint_asset(
            package_path,
            asset_name,
            HERO_CHARACTER_PARENT_CLASS
        )

    log_info("英雄 Character Blueprint 创建完成。")


def create_ui_widget_blueprints():
    """创建 UI Widget Blueprint"""

    log_info("=" * 60)
    log_info("开始创建 UI Widget Blueprint...")
    log_info("=" * 60)

    widget_blueprints = [
        # MainMenu
        ("{}/UI/Frontend/MainMenu".format(CONTENT_PATH), "WB_MainMenu", "主菜单"),
        ("{}/UI/Frontend/MainMenu".format(CONTENT_PATH), "WB_MainMenuButton", "主菜单按钮"),

        # FiveCampSelect
        ("{}/UI/Frontend/FiveCampSelect".format(CONTENT_PATH), "WB_FiveCampSelect", "阵营选择"),
        ("{}/UI/Frontend/FiveCampSelect".format(CONTENT_PATH), "WB_FiveCampInfoPanel", "阵营信息面板"),
        ("{}/UI/Frontend/FiveCampSelect".format(CONTENT_PATH), "WB_FiveCampList", "阵营列表"),

        # PartyPanel
        ("{}/UI/Frontend/PartyPanel".format(CONTENT_PATH), "WB_PartyPanel", "队伍面板"),
        ("{}/UI/Frontend/PartyPanel".format(CONTENT_PATH), "WB_PartyMember", "队伍成员"),

        # QueueStatus
        ("{}/UI/Frontend/QueueStatus".format(CONTENT_PATH), "WB_QueueStatus", "队列状态"),
        ("{}/UI/Frontend/QueueStatus".format(CONTENT_PATH), "WB_QueueTimer", "队列计时器"),
    ]

    for package_path, asset_name, desc in widget_blueprints:
        log_info("创建 Widget 蓝图: {} - {}".format(asset_name, desc))
        create_widget_blueprint_asset(
            package_path,
            asset_name,
            WIDGET_PARENT_CLASS
        )

    log_info("UI Widget Blueprint 创建完成。")


def create_all_blueprints():
    """创建全部 Blueprint"""

    if not CREATE_BLUEPRINTS:
        log_warn("CREATE_BLUEPRINTS = False，跳过普通 Blueprint 创建。")
    else:
        create_game_blueprints()
        create_hero_blueprints()

    if not CREATE_WIDGET_BLUEPRINTS:
        log_warn("CREATE_WIDGET_BLUEPRINTS = False，跳过 Widget Blueprint 创建。")
    else:
        create_ui_widget_blueprints()


# ============================================
# 地图创建工具
# ============================================

def create_empty_map(package_path, map_name):
    """
    创建空地图。

    创建前会先删除同名地图资产。

    注意：
    1. 如果当前打开的地图正是要删除的地图，可能删除失败。
    2. 如删除失败，请先手动打开其他地图，再重新运行脚本。
    """

    package_path = normalize_directory_path(package_path)
    asset_path = make_asset_path(package_path, map_name)

    log_info("-" * 60)
    log_info("准备创建地图: {}".format(asset_path))

    if not ensure_folder_exists(package_path):
        log_error("地图目录不存在且创建失败，跳过: {}".format(package_path))
        return False

    # 重点：创建前先删除
    if not delete_asset_if_exists(asset_path):
        log_error("删除已有地图失败，跳过创建: {}".format(asset_path))
        return False

    try:
        level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

        if not level_subsystem:
            log_error("无法获取 LevelEditorSubsystem，创建地图失败: {}".format(asset_path))
            return False

        result = level_subsystem.new_level(asset_path)

        if not result:
            log_error("new_level 返回失败: {}".format(asset_path))
            return False

        save_result = level_subsystem.save_current_level()

        if save_result:
            log_info("创建并保存地图成功: {}".format(asset_path))
            return True

        log_warn("地图创建成功，但保存当前地图返回 False: {}".format(asset_path))
        return False

    except Exception as e:
        log_error("创建地图异常: {} - {}".format(asset_path, str(e)))
        return False


def create_project_maps():
    """创建项目地图"""

    if not CREATE_MAPS:
        log_warn("CREATE_MAPS = False，跳过地图创建。")
        return

    log_info("=" * 60)
    log_info("开始创建地图...")
    log_info("=" * 60)

    maps = [
        ("{}/Maps/Frontend".format(CONTENT_PATH), "FrontendMap", "前端主界面地图"),
        ("{}/Maps/Lobby".format(CONTENT_PATH), "LobbyMap", "游戏大厅地图"),
        ("{}/Maps/Arena".format(CONTENT_PATH), "Arena_5v5", "5v5 竞技场地图"),
        ("{}/Maps".format(CONTENT_PATH), "TestMap", "测试地图"),
    ]

    for package_path, map_name, desc in maps:
        log_info("创建地图: {} - {}".format(map_name, desc))
        create_empty_map(package_path, map_name)

    log_info("地图创建完成。")


# ============================================
# 保存目录
# ============================================

def save_directory_safely(directory_path):
    """安全保存目录"""
    directory_path = normalize_directory_path(directory_path)

    if not unreal.EditorAssetLibrary.does_directory_exist(directory_path):
        log_warn("目录不存在，跳过保存: {}".format(directory_path))
        return False

    try:
        saved = unreal.EditorAssetLibrary.save_directory(
            directory_path,
            only_if_is_dirty=False,
            recursive=True
        )

        if saved:
            log_info("保存目录成功: {}".format(directory_path))
            return True

        log_warn("保存目录返回 False，请检查 Output Log: {}".format(directory_path))
        return False

    except Exception as e:
        log_error("保存目录异常: {} - {}".format(directory_path, str(e)))
        return False


def save_generated_directories():
    """保存本脚本涉及到的目录"""

    log_info("=" * 60)
    log_info("开始保存生成目录...")
    log_info("=" * 60)

    directories = [
        "{}/Blueprints".format(CONTENT_PATH),
        "{}/DBA/Heroes".format(CONTENT_PATH),
        "{}/UI".format(CONTENT_PATH),
        "{}/Maps".format(CONTENT_PATH),
    ]

    for directory in directories:
        save_directory_safely(directory)

    log_info("生成目录保存完成。")


# ============================================
# 汇总打印
# ============================================

def print_created_assets_summary():
    """打印本脚本生成的资产清单"""

    log_info("=" * 60)
    log_info("生成资产清单")
    log_info("=" * 60)

    assets = [
        # Game Blueprints
        "/Game/Blueprints/GM_Lobby",
        "/Game/Blueprints/GM_Arena",
        "/Game/Blueprints/PC_Main",
        "/Game/Blueprints/PC_Host",
        "/Game/Blueprints/GI_Main",

        # Widget Blueprints
        "/Game/UI/Frontend/MainMenu/WB_MainMenu",
        "/Game/UI/Frontend/MainMenu/WB_MainMenuButton",
        "/Game/UI/Frontend/FiveCampSelect/WB_FiveCampSelect",
        "/Game/UI/Frontend/FiveCampSelect/WB_FiveCampInfoPanel",
        "/Game/UI/Frontend/FiveCampSelect/WB_FiveCampList",
        "/Game/UI/Frontend/PartyPanel/WB_PartyPanel",
        "/Game/UI/Frontend/PartyPanel/WB_PartyMember",
        "/Game/UI/Frontend/QueueStatus/WB_QueueStatus",
        "/Game/UI/Frontend/QueueStatus/WB_QueueTimer",

        # Hero Blueprints
        "/Game/DBA/Heroes/BP_Hero_FireLion",
        "/Game/DBA/Heroes/BP_Hero_WaterDragon",
        "/Game/DBA/Heroes/BP_Hero_EarthBear",
        "/Game/DBA/Heroes/BP_Hero_GoldPhoenix",
        "/Game/DBA/Heroes/BP_Hero_WoodCrane",

        # Maps
        "/Game/Maps/Frontend/FrontendMap",
        "/Game/Maps/Lobby/LobbyMap",
        "/Game/Maps/Arena/Arena_5v5",
        "/Game/Maps/TestMap",
    ]

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
    log_info("{} GenerateBlueprints.py 开始执行".format(PROJECT_NAME))
    log_info("=" * 60)

    log_warn("注意：本脚本会在创建资产前删除同名资产。")
    log_warn("如果你已经手动编辑过这些蓝图或地图，请先备份。")

    # 1. 创建文件夹
    ensure_all_folders()

    # 2. 创建蓝图
    create_all_blueprints()

    # 3. 创建地图
    create_project_maps()

    # 4. 保存相关目录
    save_generated_directories()

    # 5. 打印结果
    print_created_assets_summary()

    log_info("=" * 60)
    log_info("GenerateBlueprints.py 执行完成")
    log_info("=" * 60)

    log_info("后续建议：")
    log_info("1. 打开每个 Blueprint 检查父类是否正确。")
    log_info("2. 打开 Widget Blueprint，补充 UI 层级。")
    log_info("3. 打开地图，放置 PlayerStart、Camera、UI 初始化逻辑等。")
    log_info("4. 如果使用自定义 C++ 父类，请修改脚本顶部父类配置后重新生成。")
    log_info("5. 如果地图删除失败，请先打开一个临时空地图，再重新运行脚本。")


# ============================================
# 脚本入口
# ============================================

if __name__ == "__main__":
    main()
