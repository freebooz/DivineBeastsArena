# Copyright FreeboozStudio. All Rights Reserved.
# 神兽竞技场 - UI 蓝图创建脚本 (WBP_DBA_* 命名规范)
# 在 UE 编辑器中执行: py "%PROJECT_DIR%/Scripts/CreateUIBlueprints.py"

import unreal

# 蓝图创建函数
def delete_asset(asset_path):
    """删除已存在的资源"""
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        try:
            unreal.EditorAssetLibrary.delete_asset(asset_path)
            unreal.log(f"Deleted: {asset_path}")
            return True
        except Exception as e:
            unreal.log_warning(f"Failed to delete {asset_path}: {e}")
            return False
    return True  # 不存在也返回 True


def create_blueprint(parent_class_path, blueprint_path, delete_if_exists=False):
    """创建蓝图类"""
    # 检查是否已存在
    if unreal.EditorAssetLibrary.does_asset_exist(blueprint_path):
        if delete_if_exists:
            delete_asset(blueprint_path)
        else:
            unreal.log_warning(f"Blueprint already exists: {blueprint_path}")
            return True

    try:
        # 获取 AssetTools 实例
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        if asset_tools is None:
            unreal.log_error("AssetTools not found")
            return False

        unreal.log(f"AssetTools found: {asset_tools}")

        # 从完整路径提取目录和名称
        # blueprint_path 格式: /Game/Blueprints/UI/DBA/Arena/WBP_DBA_AbilityBar
        path_parts = blueprint_path.rsplit('/', 1)
        if len(path_parts) != 2:
            unreal.log_error(f"Invalid blueprint path format: {blueprint_path}")
            return False

        asset_dir = path_parts[0]  # /Game/Blueprints/UI/DBA/Arena
        asset_name = path_parts[1]  # WBP_DBA_AbilityBar

        # 获取父类
        parent_class = unreal.load_class(None, parent_class_path)
        if not parent_class:
            unreal.log_error(f"Parent class not found: {parent_class_path}")
            return False

        # 使用 BlueprintFactory 创建蓝图
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", parent_class)

        result = asset_tools.create_asset(
            asset_name=asset_name,
            package_path=asset_dir,
            asset_class=unreal.Blueprint,
            factory=factory
        )

        if result:
            unreal.log(f"Created blueprint: {blueprint_path}")
            return True
        else:
            unreal.log_error(f"Failed to create blueprint: {blueprint_path}")
            return False

    except Exception as e:
        unreal.log_error(f"Failed to create blueprint {blueprint_path}: {e}")
        return False


# ============================================================
# 0. 旧版蓝图路径 (需要删除的)
# ============================================================
OLD_BLUEPRINTS = [
    # Arena UI 旧路径
    "/Game/Blueprints/UI/Arena/WBP_AbilityBar",
    "/Game/Blueprints/UI/Arena/WBP_ArenaHUD_Root",
    "/Game/Blueprints/UI/Arena/WBP_PassiveAndResonancePanel",
    "/Game/Blueprints/UI/Arena/WBP_PlayerUnitFrame",
    "/Game/Blueprints/UI/Arena/CTRL_PlayerUnitFrame",
    "/Game/Blueprints/UI/Arena/CTRL_ArenaHUD",
    "/Game/Blueprints/UI/Arena/WBP_ArenaObjectiveTracker",
    "/Game/Blueprints/UI/Arena/WBP_AuraSummaryPanel",
    "/Game/Blueprints/UI/Arena/WBP_BuffBar",
    "/Game/Blueprints/UI/Arena/WBP_CCBar",
    "/Game/Blueprints/UI/Arena/WBP_ChainUltimatePanel",
    "/Game/Blueprints/UI/Arena/WBP_CombatAnnouncement",
    "/Game/Blueprints/UI/Arena/WBP_ConnectionWarning",
    "/Game/Blueprints/UI/Arena/WBP_CriticalStateHint",
    "/Game/Blueprints/UI/Arena/WBP_DebuffBar",
    "/Game/Blueprints/UI/Arena/WBP_MomentumPanel",
    "/Game/Blueprints/UI/Arena/WBP_SelfCastBar",
    "/Game/Blueprints/UI/Arena/WBP_UltimateReadyPrompt",
    # Frontend UI 旧路径
    "/Game/Blueprints/UI/Frontend/Common/WBP_BuildValidationHint",
    "/Game/Blueprints/UI/Frontend/Common/WBP_QueueRuleTooltip",
    "/Game/Blueprints/UI/Frontend/Common/WBP_InteractionPrompt",
    "/Game/Blueprints/UI/Frontend/WBP_InvitePanel",
    "/Game/Blueprints/UI/Frontend/WBP_MainLobby",
    "/Game/Blueprints/UI/Frontend/CTRL_MainLobby",
    "/Game/Blueprints/UI/Frontend/WBP_MatchFound",
    "/Game/Blueprints/UI/Frontend/WBP_NewbieTaskTracker",
    "/Game/Blueprints/UI/Frontend/WBP_NewbieVillageMain",
    "/Game/Blueprints/UI/Frontend/WBP_PartyPanel",
    "/Game/Blueprints/UI/Frontend/WBP_PortalConfirm",
    "/Game/Blueprints/UI/Frontend/WBP_QueueModeSelect",
    "/Game/Blueprints/UI/Frontend/WBP_QueueStatus",
    "/Game/Blueprints/UI/Frontend/CTRL_Queue",
    "/Game/Blueprints/UI/Frontend/WBP_ReadyCheck",
    "/Game/Blueprints/UI/Frontend/ElementSelect/WBP_ElementInfoPanel",
    "/Game/Blueprints/UI/Frontend/ElementSelect/WBP_ElementSelect",
    "/Game/Blueprints/UI/Frontend/ElementSelect/CTRL_ElementSelect",
    "/Game/Blueprints/UI/Frontend/ElementSelect/WBP_FixedSkillGroupPreview",
    "/Game/Blueprints/UI/Frontend/FiveCampSelect/WBP_FiveCampInfoPanel",
    "/Game/Blueprints/UI/Frontend/FiveCampSelect/WBP_FiveCampSelect",
    "/Game/Blueprints/UI/Frontend/FiveCampSelect/CTRL_FiveCampSelect",
    "/Game/Blueprints/UI/Frontend/HeroSelect/WBP_HeroSelect",
    "/Game/Blueprints/UI/Frontend/HeroSelect/CTRL_HeroSelect",
    "/Game/Blueprints/UI/Frontend/HeroSelect/WBP_HeroInfoPanel",
    "/Game/Blueprints/UI/Frontend/Loading/WBP_LoadingScreen",
    "/Game/Blueprints/UI/Frontend/Loading/CTRL_Loading",
    # Startup UI 旧路径
    "/Game/Blueprints/UI/Startup/WBP_StartupVideo",
]


# ============================================================
# 1. 通用 UI 框架 (Common UI Framework)
# ============================================================
COMMON_UI = [
    # 基础组件
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_UIRootLayout"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_Tooltip"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_Notification"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_UnitFrame"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_PartyFrame"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_RaidFrame"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_ActionBar"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_CastBar"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_BuffIcon"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_BuffBar_Generic"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_CombatText"),
    # 弹窗和提示
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_ModalDialog"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_SystemToast"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_ErrorBanner"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_FrontendTooltip"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_MobileInfoSheet"),
]

# ============================================================
# 2. 前台 UI A：启动 / 登录 / 注册 / 角色
# ============================================================
STARTUP_LOGIN_UI = [
    ("/Script/DivineBeastsArena.DBAStartupVideoWidget", "Blueprints/UI/DBA/Startup/WBP_DBA_StartupScreen"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Login/WBP_DBA_Login"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Login/WBP_DBA_Register"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Login/WBP_DBA_GuestLoginEntry"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Login/WBP_DBA_CharacterCreate"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Login/WBP_DBA_CharacterSelect"),
]

# ============================================================
# 3. 前台 UI B：大厅 / 新手村 / 队伍 / 匹配
# ============================================================
LOBBY_PARTY_UI = [
    ("/Script/DivineBeastsArena.DBAMainLobbyWidgetBase", "Blueprints/UI/DBA/Lobby/WBP_DBA_MainLobby"),
    ("/Script/DivineBeastsArena.DBAMainLobbyWidgetController", "Blueprints/UI/DBA/Lobby/CTRL_DBA_MainLobby"),
    ("/Script/DivineBeastsArena.DBANewbieVillageMainWidgetBase", "Blueprints/UI/DBA/Lobby/WBP_DBA_NewbieVillageMain"),
    ("/Script/DivineBeastsArena.DBANewbieTaskTrackerWidgetBase", "Blueprints/UI/DBA/Lobby/WBP_DBA_NewbieTaskTracker"),
    ("/Script/DivineBeastsArena.DBAInteractionPromptWidgetBase", "Blueprints/UI/DBA/Lobby/WBP_DBA_InteractionPrompt"),
    ("/Script/DivineBeastsArena.DBAPortalConfirmWidgetBase", "Blueprints/UI/DBA/Lobby/WBP_DBA_PortalConfirm"),
    ("/Script/DivineBeastsArena.DBAPartyPanelWidgetBase", "Blueprints/UI/DBA/Lobby/WBP_DBA_PartyPanel"),
    ("/Script/DivineBeastsArena.DBAInvitePanelWidgetBase", "Blueprints/UI/DBA/Lobby/WBP_DBA_InvitePanel"),
    ("/Script/DivineBeastsArena.DBAQueueModeSelectWidgetBase", "Blueprints/UI/DBA/Lobby/WBP_DBA_QueueModeSelect"),
    ("/Script/DivineBeastsArena.DBAQueueStatusWidgetBase", "Blueprints/UI/DBA/Lobby/WBP_DBA_QueueStatus"),
    ("/Script/DivineBeastsArena.DBAQueueWidgetController", "Blueprints/UI/DBA/Lobby/CTRL_DBA_Queue"),
    ("/Script/DivineBeastsArena.DBAMatchFoundWidgetBase", "Blueprints/UI/DBA/Lobby/WBP_DBA_MatchFound"),
    ("/Script/DivineBeastsArena.DBAReadyCheckWidgetBase", "Blueprints/UI/DBA/Lobby/WBP_DBA_ReadyCheck"),
]

# ============================================================
# 4. 前台 UI C：英雄 / 元素 / 阵营选择
# ============================================================
HERO_ELEMENT_UI = [
    ("/Script/DivineBeastsArena.DBAHeroSelectWidgetBase", "Blueprints/UI/DBA/HeroSelect/WBP_DBA_HeroSelect"),
    ("/Script/DivineBeastsArena.DBAHeroSelectWidgetController", "Blueprints/UI/DBA/HeroSelect/CTRL_DBA_HeroSelect"),
    ("/Script/DivineBeastsArena.DBAHeroInfoPanelWidgetBase", "Blueprints/UI/DBA/HeroSelect/WBP_DBA_HeroInfoPanel"),
    ("/Script/DivineBeastsArena.DBAElementSelectWidgetBase", "Blueprints/UI/DBA/ElementSelect/WBP_DBA_ElementSelect"),
    ("/Script/DivineBeastsArena.DBAElementSelectWidgetController", "Blueprints/UI/DBA/ElementSelect/CTRL_DBA_ElementSelect"),
    ("/Script/DivineBeastsArena.DBAElementInfoPanelWidgetBase", "Blueprints/UI/DBA/ElementSelect/WBP_DBA_ElementInfoPanel"),
    ("/Script/DivineBeastsArena.DBAFixedSkillGroupPreviewWidgetBase", "Blueprints/UI/DBA/ElementSelect/WBP_DBA_FixedSkillGroupPreview"),
    ("/Script/DivineBeastsArena.DBAFiveCampSelectWidgetBase", "Blueprints/UI/DBA/FiveCampSelect/WBP_DBA_FiveCampSelect"),
    ("/Script/DivineBeastsArena.DBAFiveCampSelectWidgetController", "Blueprints/UI/DBA/FiveCampSelect/CTRL_DBA_FiveCampSelect"),
    ("/Script/DivineBeastsArena.DBAFiveCampInfoPanelWidgetBase", "Blueprints/UI/DBA/FiveCampSelect/WBP_DBA_FiveCampInfoPanel"),
    ("/Script/DivineBeastsArena.DBABuildValidationHintWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_BuildValidationHint"),
    ("/Script/DivineBeastsArena.DBAQueueRuleTooltipWidgetBase", "Blueprints/UI/DBA/Common/WBP_DBA_QueueRuleTooltip"),
]

# ============================================================
# 5. 前台 UI：加载 / 结算 / 设置
# ============================================================
LOADING_RESULT_UI = [
    ("/Script/DivineBeastsArena.DBALoadingScreenWidgetBase", "Blueprints/UI/DBA/Loading/WBP_DBA_LoadingScreen"),
    ("/Script/DivineBeastsArena.DBALoadingWidgetController", "Blueprints/UI/DBA/Loading/CTRL_DBA_Loading"),
    # Result 和 PostMatch 暂用 UserWidgetBase
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Result/WBP_DBA_Result"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Result/WBP_DBA_PostMatch"),
    # Settings 系列暂用 UserWidgetBase
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Settings/WBP_DBA_SettingsRoot"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Settings/WBP_DBA_SettingsGraphics"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Settings/WBP_DBA_SettingsAudio"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Settings/WBP_DBA_SettingsInput"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Settings/WBP_DBA_SettingsGameplay"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Settings/WBP_DBA_SettingsAndroid"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Settings/WBP_DBA_SettingsNetwork"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Settings/WBP_DBA_SettingsAccessibility"),
]

# ============================================================
# 6. 对局 UI A：主 HUD (Arena HUD)
# ============================================================
ARENA_HUD_UI = [
    ("/Script/DivineBeastsArena.DBAArenaHUDRootWidgetBase", "Blueprints/UI/DBA/ArenaHUD/WBP_DBA_ArenaHUDRoot"),
    ("/Script/DivineBeastsArena.DBAArenaHUDWidgetController", "Blueprints/UI/DBA/ArenaHUD/CTRL_DBA_ArenaHUD"),
    ("/Script/DivineBeastsArena.DBAPlayerUnitFrameWidgetBase", "Blueprints/UI/DBA/ArenaHUD/WBP_DBA_PlayerUnitFrame"),
    ("/Script/DivineBeastsArena.DBAPlayerUnitFrameWidgetController", "Blueprints/UI/DBA/ArenaHUD/CTRL_DBA_PlayerUnitFrame"),
    ("/Script/DivineBeastsArena.DBAAbilityBarWidgetBase", "Blueprints/UI/DBA/ArenaHUD/WBP_DBA_AbilityBar"),
    ("/Script/DivineBeastsArena.DBAPassiveAndResonancePanelWidgetBase", "Blueprints/UI/DBA/ArenaHUD/WBP_DBA_PassiveAndResonancePanel"),
    ("/Script/DivineBeastsArena.DBABuffBarWidgetBase", "Blueprints/UI/DBA/ArenaHUD/WBP_DBA_BuffBar"),
    ("/Script/DivineBeastsArena.DBADebuffBarWidgetBase", "Blueprints/UI/DBA/ArenaHUD/WBP_DBA_DebuffBar"),
    ("/Script/DivineBeastsArena.DBACCBarWidgetBase", "Blueprints/UI/DBA/ArenaHUD/WBP_DBA_CCBar"),
    ("/Script/DivineBeastsArena.DBASelfCastBarWidgetBase", "Blueprints/UI/DBA/ArenaHUD/WBP_DBA_SelfCastBar"),
    ("/Script/DivineBeastsArena.DBAMomentumPanelWidgetBase", "Blueprints/UI/DBA/ArenaHUD/WBP_DBA_MomentumPanel"),
    ("/Script/DivineBeastsArena.DBAChainUltimatePanelWidgetBase", "Blueprints/UI/DBA/ArenaHUD/WBP_DBA_ChainUltimatePanel"),
    ("/Script/DivineBeastsArena.DBACombatAnnouncementWidgetBase", "Blueprints/UI/DBA/ArenaHUD/WBP_DBA_CombatAnnouncement"),
    ("/Script/DivineBeastsArena.DBACriticalStateHintWidgetBase", "Blueprints/UI/DBA/ArenaHUD/WBP_DBA_CriticalStateHint"),
    ("/Script/DivineBeastsArena.DBAAuraSummaryPanelWidgetBase", "Blueprints/UI/DBA/ArenaHUD/WBP_DBA_AuraSummaryPanel"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/ArenaHUD/WBP_DBA_CooldownDetailTooltip"),
    ("/Script/DivineBeastsArena.DBAUltimateReadyPromptWidgetBase", "Blueprints/UI/DBA/ArenaHUD/WBP_DBA_UltimateReadyPrompt"),
    ("/Script/DivineBeastsArena.DBAConnectionWarningWidgetBase", "Blueprints/UI/DBA/ArenaHUD/WBP_DBA_ConnectionWarning"),
    ("/Script/DivineBeastsArena.DBAArenaObjectiveTrackerWidgetBase", "Blueprints/UI/DBA/ArenaHUD/WBP_DBA_ArenaObjectiveTracker"),
]

# ============================================================
# 7. 对局 UI B：目标 / 队伍 / 小地图 / 计分板
# ============================================================
ARENA_TARGET_UI = [
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Arena/WBP_DBA_TargetFrame"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Arena/WBP_DBA_TargetInfo"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Arena/WBP_DBA_TargetCastBar"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Arena/WBP_DBA_FocusFrame"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Arena/WBP_DBA_AllyTeamFrame"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Arena/WBP_DBA_PartyRaidStyleMemberSlot"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Arena/WBP_DBA_KillFeed"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Arena/WBP_DBA_Scoreboard"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Arena/WBP_DBA_Minimap"),
    ("/Script/DivineBeastsArena.DBAUserWidgetBase", "Blueprints/UI/DBA/Arena/WBP_DBA_ExpandedMap"),
]


def main():
    unreal.log("=== Creating UI Blueprints for DivineBeastsArena (WBP_DBA_ Naming) ===")

    all_ui = (
        COMMON_UI +
        STARTUP_LOGIN_UI +
        LOBBY_PARTY_UI +
        HERO_ELEMENT_UI +
        LOADING_RESULT_UI +
        ARENA_HUD_UI +
        ARENA_TARGET_UI
    )

    total = len(all_ui)
    unreal.log(f"Total blueprints to create: {total}")

    success_count = 0
    fail_count = 0

    # ========================================
    # Step 0: 删除旧版蓝图
    # ========================================
    unreal.log("\n=== Deleting Old Blueprints ===")
    old_deleted = 0
    old_failed = 0
    for old_path in OLD_BLUEPRINTS:
        if unreal.EditorAssetLibrary.does_asset_exist(old_path):
            try:
                unreal.EditorAssetLibrary.delete_asset(old_path)
                unreal.log(f"Deleted: {old_path}")
                old_deleted += 1
            except Exception as e:
                unreal.log_warning(f"Failed to delete {old_path}: {e}")
                old_failed += 1
        else:
            unreal.log(f"Already gone: {old_path}")
    unreal.log(f"Old blueprints deleted: {old_deleted}, failed: {old_failed}")

    # 创建 Common UI
    unreal.log("\n=== Creating Common UI ===")
    for parent_class, bp_path in COMMON_UI:
        full_path = f"/Game/{bp_path}"
        if create_blueprint(parent_class, full_path):
            success_count += 1
        else:
            fail_count += 1

    # 创建 Startup/Login UI
    unreal.log("\n=== Creating Startup/Login UI ===")
    for parent_class, bp_path in STARTUP_LOGIN_UI:
        full_path = f"/Game/{bp_path}"
        if create_blueprint(parent_class, full_path):
            success_count += 1
        else:
            fail_count += 1

    # 创建 Lobby/Party UI
    unreal.log("\n=== Creating Lobby/Party UI ===")
    for parent_class, bp_path in LOBBY_PARTY_UI:
        full_path = f"/Game/{bp_path}"
        if create_blueprint(parent_class, full_path):
            success_count += 1
        else:
            fail_count += 1

    # 创建 Hero/Element UI
    unreal.log("\n=== Creating Hero/Element UI ===")
    for parent_class, bp_path in HERO_ELEMENT_UI:
        full_path = f"/Game/{bp_path}"
        if create_blueprint(parent_class, full_path):
            success_count += 1
        else:
            fail_count += 1

    # 创建 Loading/Result UI
    unreal.log("\n=== Creating Loading/Result UI ===")
    for parent_class, bp_path in LOADING_RESULT_UI:
        full_path = f"/Game/{bp_path}"
        if create_blueprint(parent_class, full_path):
            success_count += 1
        else:
            fail_count += 1

    # 创建 Arena HUD UI
    unreal.log("\n=== Creating Arena HUD UI ===")
    for parent_class, bp_path in ARENA_HUD_UI:
        full_path = f"/Game/{bp_path}"
        if create_blueprint(parent_class, full_path):
            success_count += 1
        else:
            fail_count += 1

    # 创建 Arena Target UI
    unreal.log("\n=== Creating Arena Target UI ===")
    for parent_class, bp_path in ARENA_TARGET_UI:
        full_path = f"/Game/{bp_path}"
        if create_blueprint(parent_class, full_path):
            success_count += 1
        else:
            fail_count += 1

    # 总结
    unreal.log("\n=== Summary ===")
    unreal.log(f"Success: {success_count}")
    unreal.log(f"Failed: {fail_count}")
    unreal.log("Done!")


if __name__ == "__main__":
    main()
