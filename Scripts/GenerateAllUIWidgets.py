# Copyright Freebooz Studio. All Rights Reserved.
#
# GenerateAllUIBlueprints.py
#
# UE5 用户界面 Widget Blueprint / WidgetController Blueprint 全量生成脚本
#
# 在 UE Editor 的 Python Console 中执行：
# py D:/DivineBeastsArena/Scripts/GenerateAllUIBlueprints.py
#
# 重要说明：
# 1. 本脚本会创建 UI Widget Blueprint 和 WidgetController Blueprint。
# 2. 创建资产前会删除同路径同名资产，避免 Asset already exists。
# 3. Widget Blueprint 父类必须是 UUserWidget 子类。
#    如果 C++ 类不存在或不是 UUserWidget 子类，默认回退到 unreal.UserWidget。
# 4. WidgetController Blueprint 父类如果不存在或不可用，默认回退到 unreal.Object。
# 5. 本脚本只创建蓝图资产骨架，不创建 Designer 内部控件树。
# 6. 默认不编译蓝图，避免空 Widget Blueprint 因 BindWidget 缺少同名控件而报错。
# 7. 默认不递归保存 /Game/UI，避免旧 Widget Blueprint 被触发编译并报 BindWidget 错误。
# 8. 如果你已经手动编辑过这些蓝图，请先备份。
#

import unreal


# ============================================
# 基础配置
# ============================================

CONTENT_PATH = "/Game"
PROJECT_NAME = "DivineBeastsArena"
PROJECT_MODULE = "DivineBeastsArena"

# 创建资产前是否删除同路径同名资产
DELETE_EXISTING_ASSETS_BEFORE_CREATE = True

# 删除资产后是否执行 GC
COLLECT_GARBAGE_AFTER_DELETE = True

# 创建后是否编译蓝图
# 初始化阶段建议 False，因为空 Widget Blueprint 缺少 BindWidget 子控件时会编译失败。
COMPILE_BLUEPRINT_AFTER_CREATE = False

# 是否在最后递归保存 /Game/UI
# 初始化阶段建议 False，因为递归保存 /Game/UI 会触发旧 Widget Blueprint 编译。
SAVE_UI_DIRECTORY_AT_END = False

# 如果 Widget 父类加载失败或不是 UUserWidget 子类，是否回退到 unreal.UserWidget
FALLBACK_WIDGET_PARENT_TO_USER_WIDGET = True

# 如果 WidgetController 父类加载失败或不可用，是否回退到 unreal.Object
FALLBACK_CONTROLLER_PARENT_TO_OBJECT = True


# ============================================
# 日志工具
# ============================================

def log_info(msg):
    unreal.log("[GenerateAllUIBlueprints][INFO] {}".format(msg))


def log_warn(msg):
    unreal.log_warning("[GenerateAllUIBlueprints][WARN] {}".format(msg))


def log_error(msg):
    unreal.log_error("[GenerateAllUIBlueprints][ERROR] {}".format(msg))


# ============================================
# 路径工具
# ============================================

def normalize_path(path):
    """
    规范化 UE 内容目录路径。

    输入:
        UI/Frontend
        /Game/UI/Frontend/
    输出:
        /Game/UI/Frontend
    """

    if not path:
        return CONTENT_PATH

    if not path.startswith("/"):
        path = "/" + path

    return path.rstrip("/")


def make_asset_path(package_path, asset_name):
    """
    拼接 UE 资产路径。
    """

    return "{}/{}".format(normalize_path(package_path), asset_name)


def make_native_class_path(class_name_without_prefix):
    """
    根据 C++ 反射类名生成 /Script 路径。

    注意:
    如果 C++ 类名是 UDBALoginWidgetBase，
    这里传入 DBALoginWidgetBase。
    """

    return "/Script/{}.{}".format(PROJECT_MODULE, class_name_without_prefix)


def ensure_folder_exists(folder_path):
    """
    确保 Content Browser 目录存在。
    """

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
# GC / 删除资产
# ============================================

def collect_garbage():
    """
    删除资产后执行 GC，降低对象残留导致的重建失败概率。
    """

    if not COLLECT_GARBAGE_AFTER_DELETE:
        return

    try:
        unreal.SystemLibrary.collect_garbage()
    except Exception as e:
        log_warn("GC 失败，可忽略: {}".format(str(e)))


def delete_asset_if_exists(asset_path):
    """
    创建前删除同名资产，避免 create_asset 因资产已存在失败。
    """

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
# 类加载工具
# ============================================

def try_load_class(class_path):
    """
    安静尝试加载 C++ / 蓝图生成类。
    """

    if not class_path:
        return None

    try:
        return unreal.load_class(None, class_path)
    except Exception:
        return None


def build_fixed_class_path(class_path):
    """
    尝试修正误带 U/A 前缀的反射路径。

    错误示例:
        /Script/DivineBeastsArena.UDBALoginWidgetBase

    正确示例:
        /Script/DivineBeastsArena.DBALoginWidgetBase
    """

    if not class_path or "." not in class_path:
        return None

    prefix, class_name = class_path.rsplit(".", 1)

    if len(class_name) > 1 and class_name[0] in ["U", "A"]:
        return "{}.{}".format(prefix, class_name[1:])

    return None


def load_native_class(class_path):
    """
    加载 C++ 反射类。
    如果加载失败，会尝试修正 U/A 前缀。
    """

    if not class_path:
        log_warn("类路径为空")
        return None

    loaded = try_load_class(class_path)

    if loaded:
        log_info("加载类成功: {}".format(class_path))
        return loaded

    log_warn("加载类失败: {}".format(class_path))

    fixed_class_path = build_fixed_class_path(class_path)

    if fixed_class_path:
        log_warn("尝试修正类路径: {} -> {}".format(class_path, fixed_class_path))

        fixed_loaded = try_load_class(fixed_class_path)

        if fixed_loaded:
            log_info("修正后加载类成功: {}".format(fixed_class_path))
            return fixed_loaded

        log_warn("修正后仍加载失败: {}".format(fixed_class_path))

    return None


# ============================================
# 编译 / 保存工具
# ============================================

def compile_blueprint_safely(blueprint, asset_path):
    """
    尝试编译 Blueprint。

    默认 COMPILE_BLUEPRINT_AFTER_CREATE = False。
    因为空 Widget Blueprint 没有 Designer 子控件时，
    如果 C++ 使用 BindWidget，会发生编译错误。
    """

    if not blueprint:
        return False

    if not COMPILE_BLUEPRINT_AFTER_CREATE:
        log_info("跳过 Blueprint 编译: {}".format(asset_path))
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
    """
    保存已加载资产。
    """

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


def save_directory_safely(directory_path):
    """
    保存指定目录。
    """

    directory_path = normalize_path(directory_path)

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
        log_warn("保存目录异常: {} - {}".format(directory_path, str(e)))
        return False


# ============================================
# Blueprint 创建工具
# ============================================

def create_widget_blueprint(package_path, asset_name, parent_class_path, desc=""):
    """
    创建 Widget Blueprint。

    如果 parent_class_path 对应类无法作为 WidgetBlueprintFactory.parent_class，
    会回退到 unreal.UserWidget，确保资产仍然创建。
    """

    package_path = normalize_path(package_path)
    asset_path = make_asset_path(package_path, asset_name)

    log_info("-" * 80)
    log_info("准备创建 Widget Blueprint: {} ({})".format(asset_path, desc))

    if not ensure_folder_exists(package_path):
        log_error("目录不存在且创建失败，跳过: {}".format(package_path))
        return None

    # 创建前删除
    if not delete_asset_if_exists(asset_path):
        log_error("删除已有 Widget Blueprint 失败，跳过创建: {}".format(asset_path))
        return None

    parent_class = load_native_class(parent_class_path)

    if not parent_class:
        if FALLBACK_WIDGET_PARENT_TO_USER_WIDGET:
            log_warn("Widget 父类加载失败，回退到 unreal.UserWidget: {}".format(parent_class_path))
            parent_class = unreal.UserWidget
        else:
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
            log_warn("设置 Widget 父类失败: {}".format(parent_class_path))
            log_warn("原因: {}".format(str(parent_error)))

            if FALLBACK_WIDGET_PARENT_TO_USER_WIDGET:
                log_warn("尝试回退到 unreal.UserWidget 创建: {}".format(asset_path))
                factory.set_editor_property("parent_class", unreal.UserWidget)
            else:
                log_error("父类不是 UUserWidget 子类，跳过创建: {}".format(asset_path))
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
    """
    创建 WidgetController Blueprint。

    WidgetController 通常是 UObject 派生类，不使用 WidgetBlueprintFactory。
    如果父类加载失败，默认回退到 unreal.Object。
    """

    package_path = normalize_path(package_path)
    asset_path = make_asset_path(package_path, asset_name)

    log_info("-" * 80)
    log_info("准备创建 WidgetController Blueprint: {} ({})".format(asset_path, desc))

    if not ensure_folder_exists(package_path):
        log_error("目录不存在且创建失败，跳过: {}".format(package_path))
        return None

    # 创建前删除
    if not delete_asset_if_exists(asset_path):
        log_error("删除已有 WidgetController Blueprint 失败，跳过创建: {}".format(asset_path))
        return None

    parent_class = load_native_class(parent_class_path)

    if not parent_class:
        if FALLBACK_CONTROLLER_PARENT_TO_OBJECT:
            log_warn("WidgetController 父类加载失败，回退到 unreal.Object: {}".format(parent_class_path))
            parent_class = unreal.Object
        else:
            log_error("WidgetController 父类加载失败，跳过创建: {}".format(parent_class_path))
            return None

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    try:
        factory = unreal.BlueprintFactory()

        try:
            factory.set_editor_property("parent_class", parent_class)

        except Exception as parent_error:
            log_warn("设置 WidgetController 父类失败: {}".format(parent_class_path))
            log_warn("原因: {}".format(str(parent_error)))

            if FALLBACK_CONTROLLER_PARENT_TO_OBJECT:
                log_warn("尝试回退到 unreal.Object 创建: {}".format(asset_path))
                factory.set_editor_property("parent_class", unreal.Object)
            else:
                return None

        bp = asset_tools.create_asset(
            asset_name=asset_name,
            package_path=package_path,
            asset_class=unreal.Blueprint,
            factory=factory
        )

        if not bp:
            log_error("创建 WidgetController Blueprint 失败: {}".format(asset_path))
            return None

        log_info("创建 WidgetController Blueprint 成功: {}".format(asset_path))

        compile_blueprint_safely(bp, asset_path)
        save_loaded_asset_safely(bp, asset_path)

        return bp

    except Exception as e:
        log_error("创建 WidgetController Blueprint 异常: {} - {}".format(asset_path, str(e)))
        return None


# ============================================
# UI 清单
# ============================================

UI_WIDGET_BLUEPRINTS = [
    # UI Framework
    ("UI/Common", "WBP_DBA_UIRootLayout", "DBAUIRootLayoutWidget", "UI 根布局"),
    ("UI/Common", "WBP_DBA_Tooltip", "DBATooltipWidgetBase", "通用 Tooltip"),
    ("UI/Common", "WBP_DBA_Notification", "DBANotificationWidgetBase", "通用通知"),
    ("UI/Common", "WBP_DBA_UnitFrame", "DBAUnitFrameWidgetBase", "通用单位框"),
    ("UI/Common", "WBP_DBA_PartyFrame", "DBAPartyFrameWidgetBase", "通用队伍框架"),
    ("UI/Common", "WBP_DBA_RaidFrame", "DBARaidFrameWidgetBase", "通用团队框架"),
    ("UI/Common", "WBP_DBA_ActionBar", "DBAActionBarWidgetBase", "通用动作条"),
    ("UI/Common", "WBP_DBA_CastBar", "DBACastBarWidgetBase", "通用施法条"),
    ("UI/Common", "WBP_DBA_BuffIcon", "DBABuffIconWidgetBase", "通用 Buff 图标"),
    ("UI/Common", "WBP_DBA_BuffBar_Generic", "DBABuffBarWidgetBase", "通用 Buff 栏"),
    ("UI/Common", "WBP_DBA_CombatText", "DBACombatTextWidgetBase", "通用战斗文字"),

    # Frontend A
    ("UI/Frontend/Startup", "WBP_DBA_StartupScreen", "DBAStartupScreenWidgetBase", "启动界面"),
    ("UI/Frontend/Login", "WBP_DBA_Login", "DBALoginWidgetBase", "登录界面"),
    ("UI/Frontend/Login", "WBP_DBA_Register", "DBARegisterWidgetBase", "注册界面"),
    ("UI/Frontend/Login", "WBP_DBA_GuestLoginEntry", "DBAGuestLoginEntryWidgetBase", "游客登录入口"),
    ("UI/Frontend/Character", "WBP_DBA_CharacterCreate", "DBACharacterCreateWidgetBase", "角色创建界面"),
    ("UI/Frontend/Character", "WBP_DBA_CharacterSelect", "DBACharacterSelectWidgetBase", "角色选择界面"),
    ("UI/Frontend/Common", "WBP_DBA_ModalDialog", "DBAModalDialogWidgetBase", "模态对话框"),
    ("UI/Frontend/Common", "WBP_DBA_SystemToast", "DBASystemToastWidgetBase", "系统 Toast"),
    ("UI/Frontend/Common", "WBP_DBA_ErrorBanner", "DBAErrorBannerWidgetBase", "错误横幅"),
    ("UI/Frontend/Common", "WBP_DBA_FrontendTooltip", "DBAFrontendTooltipWidgetBase", "前台 Tooltip"),
    ("UI/Frontend/Common", "WBP_DBA_MobileInfoSheet", "DBAMobileInfoSheetWidgetBase", "移动端信息抽屉"),

    # Frontend B
    ("UI/Frontend/MainLobby", "WBP_DBA_MainLobby", "DBAMainLobbyWidgetBase", "主大厅界面"),
    ("UI/Frontend/NewbieVillage", "WBP_DBA_NewbieVillageMain", "DBANewbieVillageMainWidgetBase", "新手村主界面"),
    ("UI/Frontend/NewbieVillage", "WBP_DBA_NewbieTaskTracker", "DBANewbieTaskTrackerWidgetBase", "新手任务追踪"),
    ("UI/Frontend/Interaction", "WBP_DBA_InteractionPrompt", "DBAInteractionPromptWidgetBase", "交互提示"),
    ("UI/Frontend/Portal", "WBP_DBA_PortalConfirm", "DBAPortalConfirmWidgetBase", "传送门确认"),
    ("UI/Frontend/Party", "WBP_DBA_PartyPanel", "DBAPartyPanelWidgetBase", "队伍面板"),
    ("UI/Frontend/Party", "WBP_DBA_InvitePanel", "DBAInvitePanelWidgetBase", "邀请面板"),
    ("UI/Frontend/Queue", "WBP_DBA_QueueModeSelect", "DBAQueueModeSelectWidgetBase", "匹配模式选择"),
    ("UI/Frontend/Queue", "WBP_DBA_QueueStatus", "DBAQueueStatusWidgetBase", "匹配状态"),
    ("UI/Frontend/Queue", "WBP_DBA_MatchFound", "DBAMatchFoundWidgetBase", "匹配成功"),
    ("UI/Frontend/ReadyCheck", "WBP_DBA_ReadyCheck", "DBAReadyCheckWidgetBase", "准备确认"),

    # Frontend C
    ("UI/Frontend/HeroSelect", "WBP_DBA_HeroSelect", "DBAHeroSelectWidgetBase", "英雄选择"),
    ("UI/Frontend/ElementSelect", "WBP_DBA_ElementSelect", "DBAElementSelectWidgetBase", "自然元素之力选择"),
    ("UI/Frontend/FiveCampSelect", "WBP_DBA_FiveCampSelect", "DBAFiveCampSelectWidgetBase", "五大阵营选择"),
    ("UI/Frontend/HeroSelect", "WBP_DBA_FixedSkillGroupPreview", "DBAFixedSkillGroupPreviewWidgetBase", "固定技能组预览"),
    ("UI/Frontend/HeroSelect", "WBP_DBA_HeroInfoPanel", "DBAHeroInfoPanelWidgetBase", "英雄信息面板"),
    ("UI/Frontend/ElementSelect", "WBP_DBA_ElementInfoPanel", "DBAElementInfoPanelWidgetBase", "自然元素之力信息面板"),
    ("UI/Frontend/FiveCampSelect", "WBP_DBA_FiveCampInfoPanel", "DBAFiveCampInfoPanelWidgetBase", "五大阵营信息面板"),
    ("UI/Frontend/Common", "WBP_DBA_BuildValidationHint", "DBABuildValidationHintWidgetBase", "构筑校验提示"),
    ("UI/Frontend/Common", "WBP_DBA_QueueRuleTooltip", "DBAQueueRuleTooltipWidgetBase", "匹配规则 Tooltip"),
    ("UI/Frontend/Loading", "WBP_DBA_LoadingScreen", "DBALoadingScreenWidgetBase", "加载界面"),
    ("UI/Frontend/Result", "WBP_DBA_Result", "DBAResultWidgetBase", "结果界面"),
    ("UI/Frontend/Result", "WBP_DBA_PostMatch", "DBAPostMatchWidgetBase", "赛后界面"),
    ("UI/Frontend/Settings", "WBP_DBA_SettingsRoot", "DBASettingsRootWidgetBase", "设置根界面"),
    ("UI/Frontend/Settings", "WBP_DBA_SettingsGraphics", "DBASettingsGraphicsWidgetBase", "图形设置"),
    ("UI/Frontend/Settings", "WBP_DBA_SettingsAudio", "DBASettingsAudioWidgetBase", "音频设置"),
    ("UI/Frontend/Settings", "WBP_DBA_SettingsInput", "DBASettingsInputWidgetBase", "输入设置"),
    ("UI/Frontend/Settings", "WBP_DBA_SettingsGameplay", "DBASettingsGameplayWidgetBase", "玩法设置"),
    ("UI/Frontend/Settings", "WBP_DBA_SettingsAndroid", "DBASettingsAndroidWidgetBase", "Android 设置"),
    ("UI/Frontend/Settings", "WBP_DBA_SettingsNetwork", "DBASettingsNetworkWidgetBase", "网络设置"),
    ("UI/Frontend/Settings", "WBP_DBA_SettingsAccessibility", "DBASettingsAccessibilityWidgetBase", "无障碍设置"),

    # Arena A
    ("UI/Arena/HUD", "WBP_DBA_ArenaHUDRoot", "DBAArenaHUDRootWidgetBase", "竞技场 HUD 根节点"),
    ("UI/Arena/HUD", "WBP_DBA_PlayerUnitFrame", "DBAPlayerUnitFrameWidgetBase", "玩家单位框"),
    ("UI/Arena/AbilityBar", "WBP_DBA_AbilityBar", "DBAAbilityBarWidgetBase", "技能栏"),
    ("UI/Arena/AbilityBar", "WBP_DBA_PassiveAndResonancePanel", "DBAPassiveAndResonancePanelWidgetBase", "被动与共鸣面板"),
    ("UI/Arena/Buff", "WBP_DBA_BuffBar", "DBABuffBarWidgetBase", "Buff 栏"),
    ("UI/Arena/Buff", "WBP_DBA_DebuffBar", "DBADebuffBarWidgetBase", "Debuff 栏"),
    ("UI/Arena/Buff", "WBP_DBA_CCBar", "DBACCBarWidgetBase", "控制效果栏"),
    ("UI/Arena/Cast", "WBP_DBA_SelfCastBar", "DBASelfCastBarWidgetBase", "自身施法条"),
    ("UI/Arena/HUD", "WBP_DBA_MomentumPanel", "DBAMomentumPanelWidgetBase", "Momentum 面板"),
    ("UI/Arena/HUD", "WBP_DBA_ChainUltimatePanel", "DBAChainUltimatePanelWidgetBase", "连锁与大招面板"),
    ("UI/Arena/Combat", "WBP_DBA_CombatAnnouncement", "DBACombatAnnouncementWidgetBase", "战斗播报"),
    ("UI/Arena/HUD", "WBP_DBA_CriticalStateHint", "DBACriticalStateHintWidgetBase", "危险状态提示"),
    ("UI/Arena/Buff", "WBP_DBA_AuraSummaryPanel", "DBAAuraSummaryPanelWidgetBase", "光环汇总面板"),
    ("UI/Arena/AbilityBar", "WBP_DBA_CooldownDetailTooltip", "DBACooldownDetailTooltipWidgetBase", "冷却详情 Tooltip"),
    ("UI/Arena/HUD", "WBP_DBA_UltimateReadyPrompt", "DBAUltimateReadyPromptWidgetBase", "大招就绪提示"),
    ("UI/Arena/HUD", "WBP_DBA_ConnectionWarning", "DBAConnectionWarningWidgetBase", "连接警告"),
    ("UI/Arena/HUD", "WBP_DBA_ArenaObjectiveTracker", "DBAArenaObjectiveTrackerWidgetBase", "竞技场目标追踪"),

    # Arena B
    ("UI/Arena/Target", "WBP_DBA_TargetFrame", "DBATargetFrameWidgetBase", "目标框"),
    ("UI/Arena/Target", "WBP_DBA_TargetInfo", "DBATargetInfoWidgetBase", "目标信息"),
    ("UI/Arena/Target", "WBP_DBA_TargetCastBar", "DBATargetCastBarWidgetBase", "目标施法条"),
    ("UI/Arena/Target", "WBP_DBA_FocusFrame", "DBAFocusFrameWidgetBase", "焦点目标框"),
    ("UI/Arena/Team", "WBP_DBA_AllyTeamFrame", "DBAAllyTeamFrameWidgetBase", "友方团队框架"),
    ("UI/Arena/Team", "WBP_DBA_PartyRaidStyleMemberSlot", "DBAPartyRaidStyleMemberSlotWidgetBase", "团队成员槽位"),
    ("UI/Arena/KillFeed", "WBP_DBA_KillFeed", "DBAKillFeedWidgetBase", "击杀信息"),
    ("UI/Arena/Scoreboard", "WBP_DBA_Scoreboard", "DBAScoreboardWidgetBase", "计分板"),
    ("UI/Arena/Minimap", "WBP_DBA_Minimap", "DBAMinimapWidgetBase", "小地图"),
    ("UI/Arena/Minimap", "WBP_DBA_ExpandedMap", "DBAExpandedMapWidgetBase", "展开地图"),
    ("UI/Arena/Minimap", "WBP_DBA_MinimapPingLegend", "DBAMinimapPingLegendWidgetBase", "小地图 Ping 图例"),
    ("UI/Arena/Respawn", "WBP_DBA_Respawn", "DBARespawnWidgetBase", "复活界面"),
    ("UI/Arena/Overhead", "WBP_DBA_OverheadBar", "DBAOverheadBarWidgetBase", "头顶血条"),
    ("UI/Arena/Overhead", "WBP_DBA_OverheadStatusIcons", "DBAOverheadStatusIconsWidgetBase", "头顶状态图标"),
    ("UI/Arena/CombatText", "WBP_DBA_CombatTextCanvas", "DBACombatTextCanvasWidgetBase", "战斗文字画布"),
    ("UI/Arena/CombatText", "WBP_DBA_DamageNumber", "DBADamageNumberWidgetBase", "伤害数字"),
    ("UI/Arena/Respawn", "WBP_DBA_DeathRecap", "DBADeathRecapWidgetBase", "死亡回放摘要"),
    ("UI/Arena/Ping", "WBP_DBA_PingWheel", "DBAPingWheelWidgetBase", "Ping 轮盘"),
    ("UI/Arena/Chat", "WBP_DBA_QuickMessage", "DBAQuickMessageWidgetBase", "快捷消息"),
    ("UI/Arena/Chat", "WBP_DBA_QuickChatHistory", "DBAQuickChatHistoryWidgetBase", "快捷聊天历史"),
    ("UI/Arena/Chat", "WBP_DBA_ChatOverlay", "DBAChatOverlayWidgetBase", "聊天覆盖层"),
    ("UI/Arena/Android", "WBP_DBA_AndroidHUDLayoutEditor", "DBAAndroidHUDLayoutEditorWidgetBase", "Android HUD 布局编辑器"),
    ("UI/Arena/Android", "WBP_DBA_AndroidHUDSafeZonePreview", "DBAAndroidHUDSafeZonePreviewWidgetBase", "Android 安全区预览"),
]


UI_WIDGET_CONTROLLER_BLUEPRINTS = [
    # Frontend
    ("UI/WidgetControllers/Frontend", "WBP_DBA_LoginWidgetController", "DBALoginWidgetController", "登录控制器"),
    ("UI/WidgetControllers/Frontend", "WBP_DBA_CharacterCreateWidgetController", "DBACharacterCreateWidgetController", "角色创建控制器"),
    ("UI/WidgetControllers/Frontend", "WBP_DBA_CharacterSelectWidgetController", "DBACharacterSelectWidgetController", "角色选择控制器"),
    ("UI/WidgetControllers/Frontend", "WBP_DBA_MainLobbyWidgetController", "DBAMainLobbyWidgetController", "主大厅控制器"),
    ("UI/WidgetControllers/Frontend", "WBP_DBA_QueueWidgetController", "DBAQueueWidgetController", "匹配控制器"),
    ("UI/WidgetControllers/Frontend", "WBP_DBA_HeroSelectWidgetController", "DBAHeroSelectWidgetController", "英雄选择控制器"),
    ("UI/WidgetControllers/Frontend", "WBP_DBA_ElementSelectWidgetController", "DBAElementSelectWidgetController", "自然元素之力选择控制器"),
    ("UI/WidgetControllers/Frontend", "WBP_DBA_FiveCampSelectWidgetController", "DBAFiveCampSelectWidgetController", "五大阵营选择控制器"),
    ("UI/WidgetControllers/Frontend", "WBP_DBA_LoadingWidgetController", "DBALoadingWidgetController", "加载控制器"),
    ("UI/WidgetControllers/Frontend", "WBP_DBA_ResultWidgetController", "DBAResultWidgetController", "结果控制器"),

    # Arena
    ("UI/WidgetControllers/Arena", "WBP_DBA_ArenaHUDWidgetController", "DBAArenaHUDWidgetController", "竞技场 HUD 控制器"),
    ("UI/WidgetControllers/Arena", "WBP_DBA_PlayerUnitFrameWidgetController", "DBAPlayerUnitFrameWidgetController", "玩家单位框控制器"),
    ("UI/WidgetControllers/Arena", "WBP_DBA_AbilityBarWidgetController", "DBAAbilityBarWidgetController", "技能栏控制器"),
    ("UI/WidgetControllers/Arena", "WBP_DBA_AuraWidgetController", "DBAAuraWidgetController", "光环控制器"),
    ("UI/WidgetControllers/Arena", "WBP_DBA_MomentumChainUltimateWidgetController", "DBAMomentumChainUltimateWidgetController", "Momentum / Chain / Ultimate 控制器"),
    ("UI/WidgetControllers/Arena", "WBP_DBA_ObjectiveTrackerWidgetController", "DBAObjectiveTrackerWidgetController", "目标追踪控制器"),
    ("UI/WidgetControllers/Arena", "WBP_DBA_TargetWidgetController", "DBATargetWidgetController", "目标控制器"),
    ("UI/WidgetControllers/Arena", "WBP_DBA_TeamFrameWidgetController", "DBATeamFrameWidgetController", "团队框架控制器"),
    ("UI/WidgetControllers/Arena", "WBP_DBA_ScoreboardWidgetController", "DBAScoreboardWidgetController", "计分板控制器"),
    ("UI/WidgetControllers/Arena", "WBP_DBA_MinimapWidgetController", "DBAMinimapWidgetController", "小地图控制器"),
    ("UI/WidgetControllers/Arena", "WBP_DBA_RespawnWidgetController", "DBARespawnWidgetController", "复活控制器"),
    ("UI/WidgetControllers/Arena", "WBP_DBA_ChatWidgetController", "DBAChatWidgetController", "聊天控制器"),
    ("UI/WidgetControllers/Arena", "WBP_DBA_AndroidHUDLayoutWidgetController", "DBAAndroidHUDLayoutWidgetController", "Android HUD 布局控制器"),
]


# ============================================
# 文件夹结构
# ============================================

def create_all_ui_folders():
    """
    创建所有 UI 相关目录。
    """

    folders = [
        "UI",
        "UI/Common",

        "UI/Frontend",
        "UI/Frontend/Startup",
        "UI/Frontend/Login",
        "UI/Frontend/Character",
        "UI/Frontend/Common",
        "UI/Frontend/MainLobby",
        "UI/Frontend/NewbieVillage",
        "UI/Frontend/Interaction",
        "UI/Frontend/Portal",
        "UI/Frontend/Party",
        "UI/Frontend/Queue",
        "UI/Frontend/ReadyCheck",
        "UI/Frontend/HeroSelect",
        "UI/Frontend/ElementSelect",
        "UI/Frontend/FiveCampSelect",
        "UI/Frontend/Loading",
        "UI/Frontend/Result",
        "UI/Frontend/Settings",

        "UI/Arena",
        "UI/Arena/HUD",
        "UI/Arena/AbilityBar",
        "UI/Arena/Buff",
        "UI/Arena/Cast",
        "UI/Arena/Combat",
        "UI/Arena/Target",
        "UI/Arena/Team",
        "UI/Arena/KillFeed",
        "UI/Arena/Scoreboard",
        "UI/Arena/Minimap",
        "UI/Arena/Respawn",
        "UI/Arena/Overhead",
        "UI/Arena/CombatText",
        "UI/Arena/Ping",
        "UI/Arena/Chat",
        "UI/Arena/Android",

        "UI/WidgetControllers",
        "UI/WidgetControllers/Frontend",
        "UI/WidgetControllers/Arena",
    ]

    log_info("=" * 80)
    log_info("开始创建 UI 文件夹结构")
    log_info("=" * 80)

    for folder in folders:
        ensure_folder_exists("{}/{}".format(CONTENT_PATH, folder))

    log_info("UI 文件夹结构创建完成")


# ============================================
# 批量创建
# ============================================

def create_all_widget_blueprints():
    """
    批量创建所有 Widget Blueprint。
    """

    log_info("=" * 80)
    log_info("开始创建 Widget Blueprint")
    log_info("=" * 80)

    created_count = 0

    for relative_folder, asset_name, parent_class_name, desc in UI_WIDGET_BLUEPRINTS:
        package_path = "{}/{}".format(CONTENT_PATH, relative_folder)
        parent_class_path = make_native_class_path(parent_class_name)

        asset = create_widget_blueprint(
            package_path,
            asset_name,
            parent_class_path,
            desc
        )

        if asset:
            created_count += 1

    log_info("Widget Blueprint 创建流程完成，成功创建或重建数量: {}".format(created_count))


def create_all_widget_controller_blueprints():
    """
    批量创建所有 WidgetController Blueprint。
    """

    log_info("=" * 80)
    log_info("开始创建 WidgetController Blueprint")
    log_info("=" * 80)

    created_count = 0

    for relative_folder, asset_name, parent_class_name, desc in UI_WIDGET_CONTROLLER_BLUEPRINTS:
        package_path = "{}/{}".format(CONTENT_PATH, relative_folder)
        parent_class_path = make_native_class_path(parent_class_name)

        asset = create_widget_controller_blueprint(
            package_path,
            asset_name,
            parent_class_path,
            desc
        )

        if asset:
            created_count += 1

    log_info("WidgetController Blueprint 创建流程完成，成功创建或重建数量: {}".format(created_count))


# ============================================
# 保存与汇总
# ============================================

def save_all_ui_directories():
    """
    可选保存所有 UI 目录。

    默认 SAVE_UI_DIRECTORY_AT_END = False。
    原因:
    1. 本脚本创建每个资产后已经单独保存。
    2. 递归保存 /Game/UI 会触发旧 Widget Blueprint 编译。
    3. 空 Widget Blueprint 缺少 BindWidget 子控件时会产生编译错误。
    """

    if not SAVE_UI_DIRECTORY_AT_END:
        log_info("=" * 80)
        log_info("跳过递归保存 /Game/UI")
        log_info("=" * 80)
        log_info("原因：每个新建资产已单独保存，避免触发旧 Widget Blueprint 的 BindWidget 编译错误。")
        return

    log_info("=" * 80)
    log_info("开始保存 UI 目录")
    log_info("=" * 80)

    directories = [
        "{}/UI/Common".format(CONTENT_PATH),
        "{}/UI/Frontend".format(CONTENT_PATH),
        "{}/UI/Arena".format(CONTENT_PATH),
        "{}/UI/WidgetControllers".format(CONTENT_PATH),
    ]

    for directory in directories:
        save_directory_safely(directory)

    log_info("UI 目录保存完成")


def print_summary():
    """
    打印生成结果清单。
    """

    log_info("=" * 80)
    log_info("UI Blueprint 生成结果汇总")
    log_info("=" * 80)

    expected_assets = []

    for relative_folder, asset_name, parent_class_name, desc in UI_WIDGET_BLUEPRINTS:
        expected_assets.append((
            make_asset_path("{}/{}".format(CONTENT_PATH, relative_folder), asset_name),
            desc,
            "Widget"
        ))

    for relative_folder, asset_name, parent_class_name, desc in UI_WIDGET_CONTROLLER_BLUEPRINTS:
        expected_assets.append((
            make_asset_path("{}/{}".format(CONTENT_PATH, relative_folder), asset_name),
            desc,
            "WidgetController"
        ))

    exists_count = 0
    missing_count = 0

    for asset_path, desc, asset_type in expected_assets:
        exists = unreal.EditorAssetLibrary.does_asset_exist(asset_path)

        if exists:
            exists_count += 1
            log_info("[存在][{}] {} - {}".format(asset_type, asset_path, desc))
        else:
            missing_count += 1
            log_warn("[缺失][{}] {} - {}".format(asset_type, asset_path, desc))

    log_info("=" * 80)
    log_info("预期 UI 资产总数: {}".format(len(expected_assets)))
    log_info("存在数量: {}".format(exists_count))
    log_info("缺失数量: {}".format(missing_count))
    log_info("=" * 80)

    if missing_count > 0:
        log_warn("存在缺失资产，请检查上方创建失败日志。常见原因：")
        log_warn("1. 当前 UE Python 环境缺少 WidgetBlueprintFactory。")
        log_warn("2. 资产正在被编辑器占用，删除失败。")
        log_warn("3. 源控或文件只读导致保存失败。")
        log_warn("4. 父类设置失败且未允许 fallback。")


# ============================================
# 主函数
# ============================================

def main():
    log_info("=" * 80)
    log_info("{} GenerateAllUIBlueprints.py 开始执行".format(PROJECT_NAME))
    log_info("=" * 80)

    log_warn("注意：本脚本会在创建资产前删除同名 UI Blueprint。")
    log_warn("如果你已经手动编辑过这些蓝图，请先备份。")
    log_warn("默认不编译蓝图，避免空 Widget Blueprint 因 BindWidget 缺失报错。")
    log_warn("默认不递归保存 /Game/UI，避免触发旧 Widget Blueprint 编译错误。")

    create_all_ui_folders()
    create_all_widget_blueprints()
    create_all_widget_controller_blueprints()
    save_all_ui_directories()
    print_summary()

    log_info("=" * 80)
    log_info("GenerateAllUIBlueprints.py 执行完成")
    log_info("=" * 80)

    log_info("后续建议：")
    log_info("1. 打开生成的 Widget Blueprint，检查 Parent Class 是否为预期 C++ 类。")
    log_info("2. 如果某些资产回退到了 UserWidget，说明对应 C++ 类不存在或不是 UUserWidget 子类。")
    log_info("3. 如果某些 WidgetController 回退到了 Object，说明对应 C++ 控制器类不存在或不可作为 Blueprint 父类。")
    log_info("4. 对有 BindWidget 要求的 C++ Widget，在 Designer 中补齐同名控件。")
    log_info("5. Designer 控件补齐后，再手动编译相关 Widget Blueprint。")
    log_info("6. 建议生成后执行 Fix Up Redirectors。")


# ============================================
# 脚本入口
# ============================================

if __name__ == "__main__":
    main()
