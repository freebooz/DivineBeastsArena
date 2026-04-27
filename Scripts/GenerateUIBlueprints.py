# Copyright Freebooz Studio. All Rights Reserved.
#
# GenerateUIBlueprints.py
#
# UE5 用户界面 Widget Blueprint 自动生成脚本
#
# 在 UE Editor 的 Python Console 中执行：
# py D:/DivineBeastsArena/Scripts/GenerateUIBlueprints.py
#
# 重要说明：
# 1. 本脚本根据项目中实际的 C++ Widget 基类生成对应 Blueprint。
# 2. 会在创建资产前删除同路径同名资产。
# 3. Widget Controller 会生成普通 Blueprint。
# 4. 如果某个类不是 UUserWidget 子类，则不会用 WidgetBlueprintFactory 创建，会跳过并输出错误。
# 5. 当前脚本保守跳过已知不是 UUserWidget 子类的条目，避免反复报:
#    allowed Class type: 'UserWidget'
#

import unreal


# ============================================
# 基础配置
# ============================================

CONTENT_PATH = "/Game"
PROJECT_NAME = "DivineBeastsArena"
PROJECT_MODULE = "DivineBeastsArena"

COLLECT_GARBAGE_AFTER_DELETE = True

# 是否在创建前删除同名资产
DELETE_EXISTING_ASSETS_BEFORE_CREATE = True

# 是否跳过已知不是 UUserWidget 子类的配置项
SKIP_KNOWN_NON_USER_WIDGET_CLASSES = True


# ============================================
# 已知不是 UUserWidget 子类的类
# ============================================
# 这些类在你的日志中已经被 WidgetBlueprintFactory 拒绝：
#
# TypeError:
# NativizeClass: Cannot nativize 'Class' as 'Class' (allowed Class type: 'UserWidget')
#
# 如果你后续把这些 C++ 类改成继承 UUserWidget，可以从此列表移除。

KNOWN_NON_USER_WIDGET_CLASSES = set([
    "/Script/DivineBeastsArena.DBABuildValidationHintWidgetBase",
    "/Script/DivineBeastsArena.DBAQueueRuleTooltipWidgetBase",
    "/Script/DivineBeastsArena.DBAElementInfoPanelWidgetBase",
    "/Script/DivineBeastsArena.DBAFixedSkillGroupPreviewWidgetBase",
    "/Script/DivineBeastsArena.DBAFiveCampInfoPanelWidgetBase",
    "/Script/DivineBeastsArena.DBAHeroInfoPanelWidgetBase",
])


# ============================================
# C++ Widget 基类映射表
# ============================================
# 格式: (parent_class_path, widget_name, display_name)

ARENA_WIDGETS = [
    ("/Script/DivineBeastsArena.DBAAbilityBarWidgetBase", "WB_AbilityBar", "技能栏"),
    ("/Script/DivineBeastsArena.DBAArenaHUDRootWidgetBase", "WB_ArenaHUDRoot", "竞技场 HUD 根节点"),
    ("/Script/DivineBeastsArena.DBAPassiveAndResonancePanelWidgetBase", "WB_PassiveAndResonancePanel", "被动与共鸣面板"),
    ("/Script/DivineBeastsArena.DBAPlayerUnitFrameWidgetBase", "WB_PlayerUnitFrame", "玩家单位框"),
    ("/Script/DivineBeastsArena.DBAArenaObjectiveTrackerWidgetBase", "WB_ArenaObjectiveTracker", "竞技场目标追踪"),
    ("/Script/DivineBeastsArena.DBAAuraSummaryPanelWidgetBase", "WB_AuraSummaryPanel", "光环汇总面板"),
    ("/Script/DivineBeastsArena.DBABuffBarWidgetBase", "WB_BuffBar", "增益栏"),
    ("/Script/DivineBeastsArena.DBACCBarWidgetBase", "WB_CCBar", "控制效果栏"),
    ("/Script/DivineBeastsArena.DBAChainUltimatePanelWidgetBase", "WB_ChainUltimatePanel", "连锁大招面板"),
    ("/Script/DivineBeastsArena.DBACombatAnnouncementWidgetBase", "WB_CombatAnnouncement", "战斗播报"),
    ("/Script/DivineBeastsArena.DBAConnectionWarningWidgetBase", "WB_ConnectionWarning", "连接警告"),
    ("/Script/DivineBeastsArena.DBACriticalStateHintWidgetBase", "WB_CriticalStateHint", "危险状态提示"),
    ("/Script/DivineBeastsArena.DBADebuffBarWidgetBase", "WB_DebuffBar", "减益栏"),
    ("/Script/DivineBeastsArena.DBAMomentumPanelWidgetBase", "WB_MomentumPanel", "动效面板"),
    ("/Script/DivineBeastsArena.DBASelfCastBarWidgetBase", "WB_SelfCastBar", "自施法栏"),
    ("/Script/DivineBeastsArena.DBAUltimateReadyPromptWidgetBase", "WB_UltimateReadyPrompt", "大招就绪提示"),
]

ARENA_WIDGET_CONTROLLERS = [
    ("/Script/DivineBeastsArena.DBAArenaHUDWidgetController", "WC_ArenaHUD", "竞技场 HUD 控制器"),
    ("/Script/DivineBeastsArena.DBAPlayerUnitFrameWidgetController", "WC_PlayerUnitFrame", "玩家单位框控制器"),
]

FRONTEND_WIDGETS = [
    # 以下两个类在日志中不是 UUserWidget 子类，默认会被跳过。
    ("/Script/DivineBeastsArena.DBABuildValidationHintWidgetBase", "WB_BuildValidationHint", "构架验证提示"),
    ("/Script/DivineBeastsArena.DBAQueueRuleTooltipWidgetBase", "WB_QueueRuleTooltip", "匹配规则提示"),

    ("/Script/DivineBeastsArena.DBAInteractionPromptWidgetBase", "WB_InteractionPrompt", "交互提示"),
    ("/Script/DivineBeastsArena.DBAInvitePanelWidgetBase", "WB_InvitePanel", "邀请面板"),
    ("/Script/DivineBeastsArena.DBAMainLobbyWidgetBase", "WB_MainLobby", "主大厅"),
    ("/Script/DivineBeastsArena.DBAMatchFoundWidgetBase", "WB_MatchFound", "匹配成功"),
    ("/Script/DivineBeastsArena.DBANewbieTaskTrackerWidgetBase", "WB_NewbieTaskTracker", "新手任务追踪"),
    ("/Script/DivineBeastsArena.DBANewbieVillageMainWidgetBase", "WB_NewbieVillageMain", "新手村主界面"),
    ("/Script/DivineBeastsArena.DBAPartyPanelWidgetBase", "WB_PartyPanel", "队伍面板"),
    ("/Script/DivineBeastsArena.DBAPortalConfirmWidgetBase", "WB_PortalConfirm", "传送门确认"),
    ("/Script/DivineBeastsArena.DBAQueueModeSelectWidgetBase", "WB_QueueModeSelect", "匹配模式选择"),
    ("/Script/DivineBeastsArena.DBAQueueStatusWidgetBase", "WB_QueueStatus", "匹配状态"),
    ("/Script/DivineBeastsArena.DBAReadyCheckWidgetBase", "WB_ReadyCheck", "准备确认"),
]

FRONTEND_WIDGET_CONTROLLERS = [
    ("/Script/DivineBeastsArena.DBAMainLobbyWidgetController", "WC_MainLobby", "主大厅控制器"),
    ("/Script/DivineBeastsArena.DBAQueueWidgetController", "WC_Queue", "匹配控制器"),
    ("/Script/DivineBeastsArena.DBAElementSelectWidgetController", "WC_ElementSelect", "元素选择控制器"),
    ("/Script/DivineBeastsArena.DBAFiveCampSelectWidgetController", "WC_FiveCampSelect", "阵营选择控制器"),
    ("/Script/DivineBeastsArena.DBAHeroSelectWidgetController", "WC_HeroSelect", "英雄选择控制器"),
    ("/Script/DivineBeastsArena.DBALoadingWidgetController", "WC_Loading", "加载控制器"),
]

ELEMENT_SELECT_WIDGETS = [
    # 以下两个类在日志中不是 UUserWidget 子类，默认会被跳过。
    ("/Script/DivineBeastsArena.DBAElementInfoPanelWidgetBase", "WB_ElementInfoPanel", "元素信息面板"),
    ("/Script/DivineBeastsArena.DBAElementSelectWidgetBase", "WB_ElementSelect", "元素选择"),
    ("/Script/DivineBeastsArena.DBAFixedSkillGroupPreviewWidgetBase", "WB_FixedSkillGroupPreview", "固定技能组预览"),
]

FIVE_CAMP_SELECT_WIDGETS = [
    # 以下类在日志中不是 UUserWidget 子类，默认会被跳过。
    ("/Script/DivineBeastsArena.DBAFiveCampInfoPanelWidgetBase", "WB_FiveCampInfoPanel", "阵营信息面板"),
    ("/Script/DivineBeastsArena.DBAFiveCampSelectWidgetBase", "WB_FiveCampSelect", "阵营选择"),
]

HERO_SELECT_WIDGETS = [
    # 以下类在日志中不是 UUserWidget 子类，默认会被跳过。
    ("/Script/DivineBeastsArena.DBAHeroInfoPanelWidgetBase", "WB_HeroInfoPanel", "英雄信息面板"),
    ("/Script/DivineBeastsArena.DBAHeroSelectWidgetBase", "WB_HeroSelect", "英雄选择"),
]

LOADING_WIDGETS = [
    ("/Script/DivineBeastsArena.DBALoadingScreenWidgetBase", "WB_LoadingScreen", "加载屏幕"),
]


# ============================================
# 日志工具
# ============================================

def log_info(msg):
    unreal.log("[GenerateUIBlueprints][INFO] {}".format(msg))


def log_warn(msg):
    unreal.log_warning("[GenerateUIBlueprints][WARN] {}".format(msg))


def log_error(msg):
    unreal.log_error("[GenerateUIBlueprints][ERROR] {}".format(msg))


# ============================================
# 路径工具
# ============================================

def normalize_path(path):
    if not path:
        return CONTENT_PATH

    if not path.startswith("/"):
        path = "/" + path

    return path.rstrip("/")


def make_asset_path(package_path, asset_name):
    return "{}/{}".format(normalize_path(package_path), asset_name)


def ensure_folder_exists(folder_path):
    folder_path = normalize_path(folder_path)

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
# GC / 删除
# ============================================

def collect_garbage():
    if not COLLECT_GARBAGE_AFTER_DELETE:
        return

    try:
        unreal.SystemLibrary.collect_garbage()
    except Exception as e:
        log_warn("GC 失败，可忽略: {}".format(str(e)))


def delete_asset_if_exists(asset_path):
    if not DELETE_EXISTING_ASSETS_BEFORE_CREATE:
        return True

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


# ============================================
# 类加载
# ============================================

def try_load_class(class_path):
    if not class_path:
        return None

    try:
        return unreal.load_class(None, class_path)
    except Exception:
        return None


def build_fixed_class_path(class_path):
    if not class_path or "." not in class_path:
        return None

    prefix, class_name = class_path.rsplit(".", 1)

    if len(class_name) > 1 and class_name[0] in ["U", "A"]:
        return "{}.{}".format(prefix, class_name[1:])

    return None


def load_native_class(class_path):
    if not class_path:
        log_error("类路径为空")
        return None

    cls = try_load_class(class_path)

    if cls:
        log_info("加载类成功: {}".format(class_path))
        return cls

    log_warn("加载类失败: {}".format(class_path))

    fixed_class_path = build_fixed_class_path(class_path)

    if fixed_class_path:
        log_warn("尝试修正类路径: {} -> {}".format(class_path, fixed_class_path))

        fixed_cls = try_load_class(fixed_class_path)

        if fixed_cls:
            log_info("修正后加载类成功: {}".format(fixed_class_path))
            return fixed_cls

        log_warn("修正后仍加载失败: {}".format(fixed_class_path))

    log_error("最终加载类失败: {}".format(class_path))
    return None


# ============================================
# Blueprint 编译 / 保存
# ============================================

def compile_blueprint_safely(blueprint, asset_path):
    """
    兼容不同 UE Python API。
    当前你的环境中没有 unreal.KismetEditorUtilities，因此这里不会再报异常，只提示跳过。
    """

    if not blueprint:
        return False

    try:
        if hasattr(unreal, "KismetEditorUtilities"):
            unreal.KismetEditorUtilities.compile_blueprint(blueprint)
            log_info("编译 Blueprint 成功: {}".format(asset_path))
            return True

        if hasattr(unreal, "BlueprintEditorLibrary"):
            try:
                unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
                log_info("编译 Blueprint 成功: {}".format(asset_path))
                return True
            except Exception as e:
                log_warn("BlueprintEditorLibrary.compile_blueprint 调用失败: {} - {}".format(asset_path, str(e)))
                return False

        log_warn("当前 UE Python 环境没有可用的 Blueprint 编译接口，跳过编译: {}".format(asset_path))
        return False

    except Exception as e:
        log_warn("编译 Blueprint 失败或跳过: {} - {}".format(asset_path, str(e)))
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

        log_error("保存资产失败: {}".format(asset_path))
        return False

    except Exception as e:
        log_error("保存资产异常: {} - {}".format(asset_path, str(e)))
        return False


# ============================================
# Widget 创建前检查
# ============================================

def is_known_non_user_widget_class(parent_class_path):
    if not SKIP_KNOWN_NON_USER_WIDGET_CLASSES:
        return False

    return parent_class_path in KNOWN_NON_USER_WIDGET_CLASSES


def should_skip_widget_blueprint(parent_class_path, asset_path):
    if is_known_non_user_widget_class(parent_class_path):
        log_warn("跳过 Widget Blueprint 创建: {}".format(asset_path))
        log_warn("原因: 父类已知不是 UUserWidget 子类: {}".format(parent_class_path))
        log_warn("如果该类应作为 Widget Blueprint 父类，请将 C++ 改为继承 UUserWidget 并从 KNOWN_NON_USER_WIDGET_CLASSES 移除。")
        return True

    return False


# ============================================
# Blueprint 创建工具
# ============================================

def create_widget_blueprint(package_path, asset_name, parent_class_path, desc=""):
    package_path = normalize_path(package_path)
    asset_path = make_asset_path(package_path, asset_name)

    log_info("-" * 60)
    log_info("准备创建 Widget Blueprint: {} ({})".format(asset_path, desc))

    if should_skip_widget_blueprint(parent_class_path, asset_path):
        return None

    if not ensure_folder_exists(package_path):
        log_error("目录不存在且创建失败，跳过: {}".format(package_path))
        return None

    if not delete_asset_if_exists(asset_path):
        log_error("删除已有 Widget Blueprint 失败，跳过创建: {}".format(asset_path))
        return None

    parent_class = load_native_class(parent_class_path)

    if not parent_class:
        log_error("Widget 父类加载失败，跳过创建: {}".format(parent_class_path))
        return None

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    try:
        if not hasattr(unreal, "WidgetBlueprintFactory"):
            log_error("当前 UE Python 环境没有 WidgetBlueprintFactory，无法创建 Widget Blueprint")
            return None

        if not hasattr(unreal, "WidgetBlueprint"):
            log_error("当前 UE Python 环境没有 WidgetBlueprint 类型，无法创建 Widget Blueprint")
            return None

        factory = unreal.WidgetBlueprintFactory()

        try:
            factory.set_editor_property("parent_class", parent_class)
        except Exception as parent_error:
            log_error("设置 Widget 父类失败: {}".format(parent_class_path))
            log_error("原因: {}".format(str(parent_error)))
            log_error("该类不是 UUserWidget 子类，不能创建 Widget Blueprint。")
            log_error("要求 C++ 类必须类似: class UXXX : public UUserWidget")
            return None

        widget_bp = asset_tools.create_asset(
            asset_name=asset_name,
            package_path=package_path,
            asset_class=unreal.WidgetBlueprint,
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


def create_widget_controller_blueprint(package_path, asset_name, parent_class_path, desc=""):
    package_path = normalize_path(package_path)
    asset_path = make_asset_path(package_path, asset_name)

    log_info("-" * 60)
    log_info("准备创建 Widget Controller Blueprint: {} ({})".format(asset_path, desc))

    if not ensure_folder_exists(package_path):
        log_error("目录不存在且创建失败，跳过: {}".format(package_path))
        return None

    if not delete_asset_if_exists(asset_path):
        log_error("删除已有 Controller Blueprint 失败，跳过创建: {}".format(asset_path))
        return None

    parent_class = load_native_class(parent_class_path)

    if not parent_class:
        log_error("Widget Controller 父类加载失败，跳过创建: {}".format(parent_class_path))
        return None

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    try:
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", parent_class)

        bp = asset_tools.create_asset(
            asset_name=asset_name,
            package_path=package_path,
            asset_class=unreal.Blueprint,
            factory=factory
        )

        if not bp:
            log_error("创建 Controller Blueprint 失败: {}".format(asset_path))
            return None

        log_info("创建 Controller Blueprint 成功: {}".format(asset_path))

        compile_blueprint_safely(bp, asset_path)
        save_loaded_asset_safely(bp, asset_path)

        return bp

    except Exception as e:
        log_error("创建 Controller Blueprint 异常: {} - {}".format(asset_path, str(e)))
        return None


# ============================================
# 文件夹结构
# ============================================

def create_folder_structure():
    log_info("=" * 60)
    log_info("开始创建文件夹结构...")
    log_info("=" * 60)

    folders = [
        "{}/UI/Arena".format(CONTENT_PATH),
        "{}/UI/Arena/AbilityBar".format(CONTENT_PATH),
        "{}/UI/Arena/HUD".format(CONTENT_PATH),
        "{}/UI/Arena/PlayerFrame".format(CONTENT_PATH),
        "{}/UI/Arena/Buff".format(CONTENT_PATH),
        "{}/UI/Arena/Combat".format(CONTENT_PATH),

        "{}/UI/Frontend".format(CONTENT_PATH),
        "{}/UI/Frontend/Common".format(CONTENT_PATH),
        "{}/UI/Frontend/MainLobby".format(CONTENT_PATH),
        "{}/UI/Frontend/Party".format(CONTENT_PATH),
        "{}/UI/Frontend/Queue".format(CONTENT_PATH),
        "{}/UI/Frontend/ReadyCheck".format(CONTENT_PATH),
        "{}/UI/Frontend/Invite".format(CONTENT_PATH),
        "{}/UI/Frontend/Portal".format(CONTENT_PATH),
        "{}/UI/Frontend/Newbie".format(CONTENT_PATH),
        "{}/UI/Frontend/Interaction".format(CONTENT_PATH),
        "{}/UI/Frontend/ElementSelect".format(CONTENT_PATH),
        "{}/UI/Frontend/FiveCampSelect".format(CONTENT_PATH),
        "{}/UI/Frontend/HeroSelect".format(CONTENT_PATH),
        "{}/UI/Frontend/Loading".format(CONTENT_PATH),

        "{}/UI/WidgetControllers".format(CONTENT_PATH),
        "{}/UI/WidgetControllers/Arena".format(CONTENT_PATH),
        "{}/UI/WidgetControllers/Frontend".format(CONTENT_PATH),
    ]

    for folder in folders:
        ensure_folder_exists(folder)

    log_info("文件夹结构创建完成!")


# ============================================
# 创建函数
# ============================================

def create_arena_widgets():
    log_info("=" * 60)
    log_info("开始创建 Arena UI Widgets...")
    log_info("=" * 60)

    for parent_class, asset_name, desc in ARENA_WIDGETS:
        create_widget_blueprint(
            "{}/UI/Arena".format(CONTENT_PATH),
            asset_name,
            parent_class,
            desc
        )

    log_info("Arena UI Widgets 创建完成!")


def create_arena_widget_controllers():
    log_info("=" * 60)
    log_info("开始创建 Arena Widget Controllers...")
    log_info("=" * 60)

    for parent_class, asset_name, desc in ARENA_WIDGET_CONTROLLERS:
        create_widget_controller_blueprint(
            "{}/UI/WidgetControllers/Arena".format(CONTENT_PATH),
            asset_name,
            parent_class,
            desc
        )

    log_info("Arena Widget Controllers 创建完成!")


def create_frontend_widgets():
    log_info("=" * 60)
    log_info("开始创建 Frontend UI Widgets...")
    log_info("=" * 60)

    for parent_class, asset_name, desc in FRONTEND_WIDGETS:
        create_widget_blueprint(
            "{}/UI/Frontend".format(CONTENT_PATH),
            asset_name,
            parent_class,
            desc
        )

    log_info("Frontend UI Widgets 创建完成!")


def create_frontend_widget_controllers():
    log_info("=" * 60)
    log_info("开始创建 Frontend Widget Controllers...")
    log_info("=" * 60)

    for parent_class, asset_name, desc in FRONTEND_WIDGET_CONTROLLERS:
        create_widget_controller_blueprint(
            "{}/UI/WidgetControllers/Frontend".format(CONTENT_PATH),
            asset_name,
            parent_class,
            desc
        )

    log_info("Frontend Widget Controllers 创建完成!")


def create_element_select_widgets():
    log_info("=" * 60)
    log_info("开始创建 Element Select Widgets...")
    log_info("=" * 60)

    for parent_class, asset_name, desc in ELEMENT_SELECT_WIDGETS:
        create_widget_blueprint(
            "{}/UI/Frontend/ElementSelect".format(CONTENT_PATH),
            asset_name,
            parent_class,
            desc
        )

    log_info("Element Select Widgets 创建完成!")


def create_five_camp_select_widgets():
    log_info("=" * 60)
    log_info("开始创建 FiveCamp Select Widgets...")
    log_info("=" * 60)

    for parent_class, asset_name, desc in FIVE_CAMP_SELECT_WIDGETS:
        create_widget_blueprint(
            "{}/UI/Frontend/FiveCampSelect".format(CONTENT_PATH),
            asset_name,
            parent_class,
            desc
        )

    log_info("FiveCamp Select Widgets 创建完成!")


def create_hero_select_widgets():
    log_info("=" * 60)
    log_info("开始创建 Hero Select Widgets...")
    log_info("=" * 60)

    for parent_class, asset_name, desc in HERO_SELECT_WIDGETS:
        create_widget_blueprint(
            "{}/UI/Frontend/HeroSelect".format(CONTENT_PATH),
            asset_name,
            parent_class,
            desc
        )

    log_info("Hero Select Widgets 创建完成!")


def create_loading_widgets():
    log_info("=" * 60)
    log_info("开始创建 Loading Widgets...")
    log_info("=" * 60)

    for parent_class, asset_name, desc in LOADING_WIDGETS:
        create_widget_blueprint(
            "{}/UI/Frontend/Loading".format(CONTENT_PATH),
            asset_name,
            parent_class,
            desc
        )

    log_info("Loading Widgets 创建完成!")


# ============================================
# 保存目录
# ============================================

def save_directory_safely(directory):
    directory = normalize_path(directory)

    if not unreal.EditorAssetLibrary.does_directory_exist(directory):
        log_warn("目录不存在，跳过保存: {}".format(directory))
        return False

    try:
        saved = unreal.EditorAssetLibrary.save_directory(
            directory,
            only_if_is_dirty=False,
            recursive=True
        )

        if saved:
            log_info("保存目录成功: {}".format(directory))
            return True

        log_warn("保存目录返回 False，请检查 Output Log: {}".format(directory))
        return False

    except Exception as e:
        log_warn("保存目录失败: {} - {}".format(directory, str(e)))
        return False


def save_all_directories():
    log_info("=" * 60)
    log_info("开始保存所有目录...")
    log_info("=" * 60)

    directories = [
        "{}/UI/Arena".format(CONTENT_PATH),
        "{}/UI/Frontend".format(CONTENT_PATH),
        "{}/UI/Frontend/ElementSelect".format(CONTENT_PATH),
        "{}/UI/Frontend/FiveCampSelect".format(CONTENT_PATH),
        "{}/UI/Frontend/HeroSelect".format(CONTENT_PATH),
        "{}/UI/Frontend/Loading".format(CONTENT_PATH),
        "{}/UI/WidgetControllers".format(CONTENT_PATH),
        "{}/UI/WidgetControllers/Arena".format(CONTENT_PATH),
        "{}/UI/WidgetControllers/Frontend".format(CONTENT_PATH),
    ]

    for directory in directories:
        save_directory_safely(directory)

    log_info("目录保存完成!")


# ============================================
# 汇总
# ============================================

def print_summary():
    log_info("=" * 60)
    log_info("生成资产清单")
    log_info("=" * 60)

    all_assets = []
    skipped_assets = []

    def add_asset(parent_class_path, path):
        if parent_class_path in KNOWN_NON_USER_WIDGET_CLASSES:
            skipped_assets.append(path)
        else:
            all_assets.append(path)

    for parent_class, asset_name, _ in ARENA_WIDGETS:
        add_asset(parent_class, "{}/UI/Arena/{}".format(CONTENT_PATH, asset_name))

    for _, asset_name, _ in ARENA_WIDGET_CONTROLLERS:
        all_assets.append("{}/UI/WidgetControllers/Arena/{}".format(CONTENT_PATH, asset_name))

    for parent_class, asset_name, _ in FRONTEND_WIDGETS:
        add_asset(parent_class, "{}/UI/Frontend/{}".format(CONTENT_PATH, asset_name))

    for _, asset_name, _ in FRONTEND_WIDGET_CONTROLLERS:
        all_assets.append("{}/UI/WidgetControllers/Frontend/{}".format(CONTENT_PATH, asset_name))

    for parent_class, asset_name, _ in ELEMENT_SELECT_WIDGETS:
        add_asset(parent_class, "{}/UI/Frontend/ElementSelect/{}".format(CONTENT_PATH, asset_name))

    for parent_class, asset_name, _ in FIVE_CAMP_SELECT_WIDGETS:
        add_asset(parent_class, "{}/UI/Frontend/FiveCampSelect/{}".format(CONTENT_PATH, asset_name))

    for parent_class, asset_name, _ in HERO_SELECT_WIDGETS:
        add_asset(parent_class, "{}/UI/Frontend/HeroSelect/{}".format(CONTENT_PATH, asset_name))

    for parent_class, asset_name, _ in LOADING_WIDGETS:
        add_asset(parent_class, "{}/UI/Frontend/Loading/{}".format(CONTENT_PATH, asset_name))

    log_info("已尝试创建资产:")

    for asset_path in all_assets:
        exists = unreal.EditorAssetLibrary.does_asset_exist(asset_path)
        status = "存在" if exists else "不存在"
        log_info("[{}] {}".format(status, asset_path))

    if skipped_assets:
        log_warn("-" * 60)
        log_warn("以下资产因父类不是 UUserWidget 子类而跳过:")

        for asset_path in skipped_assets:
            log_warn("[跳过] {}".format(asset_path))

    log_info("=" * 60)
    log_info("已尝试创建 {} 个资产，跳过 {} 个非 UserWidget 项。".format(len(all_assets), len(skipped_assets)))


# ============================================
# 主函数
# ============================================

def main():
    log_info("=" * 60)
    log_info("{} GenerateUIBlueprints.py 开始执行".format(PROJECT_NAME))
    log_info("=" * 60)

    log_warn("注意：本脚本会在创建资产前删除同名资产。")
    log_warn("如果你已经手动编辑过这些蓝图，请先备份。")
    log_warn("已知非 UUserWidget 子类会被跳过，避免 WidgetBlueprintFactory 报错。")

    create_folder_structure()

    create_arena_widgets()
    create_arena_widget_controllers()

    create_frontend_widgets()
    create_frontend_widget_controllers()

    create_element_select_widgets()
    create_five_camp_select_widgets()
    create_hero_select_widgets()
    create_loading_widgets()

    save_all_directories()
    print_summary()

    log_info("=" * 60)
    log_info("GenerateUIBlueprints.py 执行完成")
    log_info("=" * 60)

    log_info("后续建议：")
    log_info("1. 打开每个 Widget Blueprint 检查父类是否正确。")
    log_info("2. 对有 BindWidget 要求的 C++ 类，在 Widget Blueprint Designer 中创建对应名字的子控件。")
    log_info("3. 当前被跳过的类如果确实要作为 Widget Blueprint 父类，请让它们继承 UUserWidget。")
    log_info("4. 当前 UE Python 环境没有 KismetEditorUtilities，编译步骤会跳过，建议在 Editor 中手动 Compile All Blueprints。")


# ============================================
# 入口
# ============================================

if __name__ == "__main__":
    main()
