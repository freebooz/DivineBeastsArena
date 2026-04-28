# Copyright FreeboozStudio. All Rights Reserved.
# 神兽竞技场 - UI 蓝图创建脚本
# 在 UE 编辑器中执行: py "%PROJECT_DIR%/Scripts/CreateUIBlueprints.py"

import unreal

# 蓝图创建函数
def create_blueprint(parent_class_path, blueprint_path):
    """创建蓝图类"""
    # 检查是否已存在
    if unreal.EditorAssetLibrary.does_asset_exist(blueprint_path):
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
        # blueprint_path 格式: /Game/Blueprints/UI/Arena/WBP_AbilityBar
        path_parts = blueprint_path.rsplit('/', 1)
        if len(path_parts) != 2:
            unreal.log_error(f"Invalid blueprint path format: {blueprint_path}")
            return False

        asset_dir = path_parts[0]  # /Game/Blueprints/UI/Arena
        asset_name = path_parts[1]  # WBP_AbilityBar

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

# Arena UI 蓝图定义
ARENA_UI = [
    ("/Script/DivineBeastsArena.DBAAbilityBarWidgetBase", "Blueprints/UI/Arena/WBP_AbilityBar"),
    ("/Script/DivineBeastsArena.DBAArenaHUDRootWidgetBase", "Blueprints/UI/Arena/WBP_ArenaHUD_Root"),
    ("/Script/DivineBeastsArena.DBAPassiveAndResonancePanelWidgetBase", "Blueprints/UI/Arena/WBP_PassiveAndResonancePanel"),
    ("/Script/DivineBeastsArena.DBAPlayerUnitFrameWidgetBase", "Blueprints/UI/Arena/WBP_PlayerUnitFrame"),
    ("/Script/DivineBeastsArena.DBAPlayerUnitFrameWidgetController", "Blueprints/UI/Arena/CTRL_PlayerUnitFrame"),
    ("/Script/DivineBeastsArena.DBAArenaHUDWidgetController", "Blueprints/UI/Arena/CTRL_ArenaHUD"),
    ("/Script/DivineBeastsArena.DBAArenaObjectiveTrackerWidgetBase", "Blueprints/UI/Arena/WBP_ArenaObjectiveTracker"),
    ("/Script/DivineBeastsArena.DBAAuraSummaryPanelWidgetBase", "Blueprints/UI/Arena/WBP_AuraSummaryPanel"),
    ("/Script/DivineBeastsArena.DBABuffBarWidgetBase", "Blueprints/UI/Arena/WBP_BuffBar"),
    ("/Script/DivineBeastsArena.DBACCBarWidgetBase", "Blueprints/UI/Arena/WBP_CCBar"),
    ("/Script/DivineBeastsArena.DBAChainUltimatePanelWidgetBase", "Blueprints/UI/Arena/WBP_ChainUltimatePanel"),
    ("/Script/DivineBeastsArena.DBACombatAnnouncementWidgetBase", "Blueprints/UI/Arena/WBP_CombatAnnouncement"),
    ("/Script/DivineBeastsArena.DBAConnectionWarningWidgetBase", "Blueprints/UI/Arena/WBP_ConnectionWarning"),
    ("/Script/DivineBeastsArena.DBACriticalStateHintWidgetBase", "Blueprints/UI/Arena/WBP_CriticalStateHint"),
    ("/Script/DivineBeastsArena.DBADebuffBarWidgetBase", "Blueprints/UI/Arena/WBP_DebuffBar"),
    ("/Script/DivineBeastsArena.DBAMomentumPanelWidgetBase", "Blueprints/UI/Arena/WBP_MomentumPanel"),
    ("/Script/DivineBeastsArena.DBASelfCastBarWidgetBase", "Blueprints/UI/Arena/WBP_SelfCastBar"),
    ("/Script/DivineBeastsArena.DBAUltimateReadyPromptWidgetBase", "Blueprints/UI/Arena/WBP_UltimateReadyPrompt"),
]

# Frontend UI 蓝图定义
FRONTEND_UI = [
    ("/Script/DivineBeastsArena.DBABuildValidationHintWidgetBase", "Blueprints/UI/Frontend/Common/WBP_BuildValidationHint"),
    ("/Script/DivineBeastsArena.DBAQueueRuleTooltipWidgetBase", "Blueprints/UI/Frontend/Common/WBP_QueueRuleTooltip"),
    ("/Script/DivineBeastsArena.DBAInteractionPromptWidgetBase", "Blueprints/UI/Frontend/Common/WBP_InteractionPrompt"),
    ("/Script/DivineBeastsArena.DBAInvitePanelWidgetBase", "Blueprints/UI/Frontend/WBP_InvitePanel"),
    ("/Script/DivineBeastsArena.DBAMainLobbyWidgetBase", "Blueprints/UI/Frontend/WBP_MainLobby"),
    ("/Script/DivineBeastsArena.DBAMainLobbyWidgetController", "Blueprints/UI/Frontend/CTRL_MainLobby"),
    ("/Script/DivineBeastsArena.DBAMatchFoundWidgetBase", "Blueprints/UI/Frontend/WBP_MatchFound"),
    ("/Script/DivineBeastsArena.DBANewbieTaskTrackerWidgetBase", "Blueprints/UI/Frontend/WBP_NewbieTaskTracker"),
    ("/Script/DivineBeastsArena.DBANewbieVillageMainWidgetBase", "Blueprints/UI/Frontend/WBP_NewbieVillageMain"),
    ("/Script/DivineBeastsArena.DBAPartyPanelWidgetBase", "Blueprints/UI/Frontend/WBP_PartyPanel"),
    ("/Script/DivineBeastsArena.DBAPortalConfirmWidgetBase", "Blueprints/UI/Frontend/WBP_PortalConfirm"),
    ("/Script/DivineBeastsArena.DBAQueueModeSelectWidgetBase", "Blueprints/UI/Frontend/WBP_QueueModeSelect"),
    ("/Script/DivineBeastsArena.DBAQueueStatusWidgetBase", "Blueprints/UI/Frontend/WBP_QueueStatus"),
    ("/Script/DivineBeastsArena.DBAQueueWidgetController", "Blueprints/UI/Frontend/CTRL_Queue"),
    ("/Script/DivineBeastsArena.DBAReadyCheckWidgetBase", "Blueprints/UI/Frontend/WBP_ReadyCheck"),
    ("/Script/DivineBeastsArena.DBAElementInfoPanelWidgetBase", "Blueprints/UI/Frontend/ElementSelect/WBP_ElementInfoPanel"),
    ("/Script/DivineBeastsArena.DBAElementSelectWidgetBase", "Blueprints/UI/Frontend/ElementSelect/WBP_ElementSelect"),
    ("/Script/DivineBeastsArena.DBAElementSelectWidgetController", "Blueprints/UI/Frontend/ElementSelect/CTRL_ElementSelect"),
    ("/Script/DivineBeastsArena.DBAFixedSkillGroupPreviewWidgetBase", "Blueprints/UI/Frontend/ElementSelect/WBP_FixedSkillGroupPreview"),
    ("/Script/DivineBeastsArena.DBAFiveCampInfoPanelWidgetBase", "Blueprints/UI/Frontend/FiveCampSelect/WBP_FiveCampInfoPanel"),
    ("/Script/DivineBeastsArena.DBAFiveCampSelectWidgetBase", "Blueprints/UI/Frontend/FiveCampSelect/WBP_FiveCampSelect"),
    ("/Script/DivineBeastsArena.DBAFiveCampSelectWidgetController", "Blueprints/UI/Frontend/FiveCampSelect/CTRL_FiveCampSelect"),
    ("/Script/DivineBeastsArena.DBAHeroSelectWidgetBase", "Blueprints/UI/Frontend/HeroSelect/WBP_HeroSelect"),
    ("/Script/DivineBeastsArena.DBAHeroSelectWidgetController", "Blueprints/UI/Frontend/HeroSelect/CTRL_HeroSelect"),
    ("/Script/DivineBeastsArena.DBAHeroInfoPanelWidgetBase", "Blueprints/UI/Frontend/HeroSelect/WBP_HeroInfoPanel"),
    ("/Script/DivineBeastsArena.DBALoadingScreenWidgetBase", "Blueprints/UI/Frontend/Loading/WBP_LoadingScreen"),
    ("/Script/DivineBeastsArena.DBALoadingWidgetController", "Blueprints/UI/Frontend/Loading/CTRL_Loading"),
]

# Startup UI 蓝图定义
STARTUP_UI = [
    ("/Script/DivineBeastsArena.DBAStartupVideoWidget", "Blueprints/UI/Startup/WBP_StartupVideo"),
]

def main():
    unreal.log("=== Creating UI Blueprints for DivineBeastsArena ===")

    total = len(ARENA_UI) + len(FRONTEND_UI) + len(STARTUP_UI)
    unreal.log(f"Total blueprints to create: {total}")

    success_count = 0
    fail_count = 0

    # 创建 Arena UI
    unreal.log("\n=== Creating Arena UI ===")
    for parent_class, bp_path in ARENA_UI:
        full_path = f"/Game/{bp_path}"
        if create_blueprint(parent_class, full_path):
            success_count += 1
        else:
            fail_count += 1

    # 创建 Frontend UI
    unreal.log("\n=== Creating Frontend UI ===")
    for parent_class, bp_path in FRONTEND_UI:
        full_path = f"/Game/{bp_path}"
        if create_blueprint(parent_class, full_path):
            success_count += 1
        else:
            fail_count += 1

    # 创建 Startup UI
    unreal.log("\n=== Creating Startup UI ===")
    for parent_class, bp_path in STARTUP_UI:
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
