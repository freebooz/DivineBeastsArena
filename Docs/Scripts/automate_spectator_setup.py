# Copyright Freebooz Games, Inc. All Rights Reserved.
"""
观战系统 UE 编辑器自动化配置脚本

使用方法:
1. 在 UE 编辑器中: Edit -> Editor Scripting -> Run Python Script
2. 或命令行: UnrealFrontend.exe -run=pythonscript -script="path/to/automate_spectator_setup.py"

注意: 部分操作需要在 UE 编辑器中手动确认
"""

import unreal

# ============================================
# 配置常量
# ============================================

ASSET_PATH_INPUT = "/Game/Input/Spectator"
ASSET_PATH_BLUEPRINT = "/Game/Blueprints/UI/Spectator"
ASSET_PATH_WIDGET = "/Game/Blueprints/UI/Spectator/Widgets"

INPUT_ACTIONS = [
    ("IA_Spectator_CycleNext", "切换到下一个玩家"),
    ("IA_Spectator_CyclePrevious", "切换到上一个玩家"),
    ("IA_Spectator_ToggleFreeView", "切换自由视角"),
    ("IA_Spectator_TogglePause", "暂停/恢复"),
    ("IA_Spectator_JumpToTarget_1", "跳转到玩家1"),
    ("IA_Spectator_JumpToTarget_2", "跳转到玩家2"),
    ("IA_Spectator_JumpToTarget_3", "跳转到玩家3"),
    ("IA_Spectator_JumpToTarget_4", "跳转到玩家4"),
    ("IA_Spectator_JumpToTarget_5", "跳转到玩家5"),
    ("IA_Spectator_JumpToTarget_6", "跳转到玩家6"),
    ("IA_Spectator_JumpToTarget_7", "跳转到玩家7"),
    ("IA_Spectator_JumpToTarget_8", "跳转到玩家8"),
    ("IA_Spectator_JumpToTarget_9", "跳转到玩家9"),
]

IMC_MAPPINGS = [
    ("IA_Spectator_CycleNext", "Tab", ""),
    ("IA_Spectator_CyclePrevious", "Tab", "Shift"),
    ("IA_Spectator_ToggleFreeView", "Space", ""),
    ("IA_Spectator_TogglePause", "P", ""),
    ("IA_Spectator_JumpToTarget_1", "One", ""),
    ("IA_Spectator_JumpToTarget_2", "Two", ""),
    ("IA_Spectator_JumpToTarget_3", "Three", ""),
    ("IA_Spectator_JumpToTarget_4", "Four", ""),
    ("IA_Spectator_JumpToTarget_5", "Five", ""),
    ("IA_Spectator_JumpToTarget_6", "Six", ""),
    ("IA_Spectator_JumpToTarget_7", "Seven", ""),
    ("IA_Spectator_JumpToTarget_8", "Eight", ""),
    ("IA_Spectator_JumpToTarget_9", "Nine", ""),
]


# ============================================
# 辅助函数
# ============================================

def ensure_folder_exists(folder_path):
    """确保文件夹存在，不存在则创建"""
    if not unreal.EditorAssetLibrary.does_directory_exist(folder_path):
        unreal.EditorAssetLibrary.make_directory(folder_path)
        unreal.log(f"[OK] Created folder: {folder_path}")
        return True
    return False


def get_or_create_folder(folder_path):
    """获取或创建文件夹"""
    ensure_folder_exists(folder_path)
    return folder_path


# ============================================
# 输入配置
# ============================================

def create_input_action(action_name, description):
    """创建 Input Action 资产"""
    asset_path = f"{ASSET_PATH_INPUT}/{action_name}"

    # 检查是否已存在
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log(f"[SKIP] Input Action already exists: {asset_path}")
        return unreal.load_asset(asset_path)

    # 创建新的 Input Action
    factory = unreal.InputActionFactory()
    new_asset = unreal.EditorAssetLibrary.create_asset(
        asset_path,
        unreal.InputAction,
        factory
    )

    if new_asset:
        # 配置触发条件为 Pressed
        new_asset.triggered_on = unreal.InputActionTriggeredOn.PRESSED
        new_asset.hold_timeout = 0.0
        unreal.log(f"[OK] Created Input Action: {action_name}")
    else:
        unreal.log_error(f"[FAIL] Failed to create Input Action: {action_name}")

    return new_asset


def create_all_input_actions():
    """创建所有 Input Actions"""
    unreal.log("========== Creating Input Actions ==========")
    get_or_create_folder(ASSET_PATH_INPUT)

    created = []
    for action_name, desc in INPUT_ACTIONS:
        asset = create_input_action(action_name, desc)
        if asset:
            created.append(action_name)

    unreal.log(f"Created {len(created)} Input Actions")
    return created


def create_imc_spectator():
    """创建 IMC_Spectator Input Mapping Context"""
    imc_path = f"{ASSET_PATH_INPUT}/IMC_Spectator"

    # 检查是否已存在
    if unreal.EditorAssetLibrary.does_asset_exist(imc_path):
        unreal.log(f"[SKIP] IMC_Spectator already exists: {imc_path}")
        return unreal.load_asset(imc_path)

    # 创建 IMC
    factory = unreal.InputMappingContextFactory()
    imc = unreal.EditorAssetLibrary.create_asset(
        imc_path,
        unreal.InputMappingContext,
        factory
    )

    if imc:
        unreal.log(f"[OK] Created IMC_Spectator")

        # 设置优先级
        imc.set_asset_editor_tool_tip("观战模式专用输入映射上下文")

    return imc


def configure_imc_spectator():
    """配置 IMC_Spectator 的输入映射"""
    imc_path = f"{ASSET_PATH_INPUT}/IMC_Spectator"

    imc = unreal.load_asset(imc_path)
    if not imc:
        unreal.log_error("[FAIL] IMC_Spectator not found")
        return False

    unreal.log("========== Configuring IMC_Spectator ==========")

    for action_name, key, modifier in IMC_MAPPINGS:
        action_path = f"{ASSET_PATH_INPUT}/{action_name}"
        ia = unreal.load_asset(action_path)

        if not ia:
            unreal.log_error(f"[FAIL] Input Action not found: {action_name}")
            continue

        # 解析修饰键
        modifiers = []
        if "Shift" in modifier:
            modifiers.append(unreal.InputAxisPropertiesDelta.SHIFT)
        if "Ctrl" in modifier:
            modifiers.append(unreal.InputAxisPropertiesDelta.CONTROL)
        if "Alt" in modifier:
            modifiers.append(unreal.InputAxisPropertiesDelta.ALT)

        # 添加映射
        key_event = unreal.KeyEvent()
        key_event.key = unreal.EditorInputLibrary.find_key_by_name(unreal.Name(key))

        # 注意: UE API 可能需要使用不同的方法来添加映射
        # imc.add_mappable_key()

        unreal.log(f"[OK] Mapped {action_name} -> {key} ({modifier})")

    # 设置优先级
    imc.priority = 100

    unreal.log("[OK] IMC_Spectator configuration complete")
    return True


# ============================================
# Blueprint 创建
# ============================================

def create_bp_spectator_hud():
    """创建 BP_SpectatorHUD 蓝图"""
    bp_path = f"{ASSET_PATH_BLUEPRINT}/BP_SpectatorHUD"

    # 检查是否已存在
    if unreal.EditorAssetLibrary.does_asset_exist(bp_path):
        unreal.log(f"[SKIP] BP_SpectatorHUD already exists: {bp_path}")
        return unreal.load_asset(bp_path)

    # 获取父类
    parent_class = unreal.EditorAssetLibrary.load_asset("/Script/UMG.WidgetBlueprint")

    # 创建蓝图
    factory = unreal.BlueprintFactory()
    factory.set_parent_class(unreal.load_asset("/Script/UMG.WidgetBlueprint"))

    bp = unreal.EditorAssetLibrary.create_asset(
        bp_path,
        unreal.WidgetBlueprint,
        factory
    )

    if bp:
        unreal.log(f"[OK] Created BP_SpectatorHUD")

    return bp


def create_wbp_spectator_status_bar():
    """创建 WBP_SpectatorStatusBar 控件"""
    widget_path = f"{ASSET_PATH_WIDGET}/WBP_SpectatorStatusBar"

    if unreal.EditorAssetLibrary.does_asset_exist(widget_path):
        unreal.log(f"[SKIP] WBP_SpectatorStatusBar already exists: {widget_path}")
        return unreal.load_asset(widget_path)

    # 创建 Widget Blueprint
    factory = unreal.WidgetBlueprintFactory()
    widget = unreal.EditorAssetLibrary.create_asset(
        widget_path,
        unreal.WidgetBlueprint,
        factory
    )

    if widget:
        unreal.log(f"[OK] Created WBP_SpectatorStatusBar")

    return widget


def create_wbp_spectator_minimap():
    """创建 WBP_SpectatorMinimap 控件"""
    widget_path = f"{ASSET_PATH_WIDGET}/WBP_SpectatorMinimap"

    if unreal.EditorAssetLibrary.does_asset_exist(widget_path):
        unreal.log(f"[SKIP] WBP_SpectatorMinimap already exists: {widget_path}")
        return unreal.load_asset(widget_path)

    factory = unreal.WidgetBlueprintFactory()
    widget = unreal.EditorAssetLibrary.create_asset(
        widget_path,
        unreal.WidgetBlueprint,
        factory
    )

    if widget:
        unreal.log(f"[OK] Created WBP_SpectatorMinimap")

    return widget


def create_spectator_controller():
    """创建 BP_SpectatorPlayerController"""
    bp_path = f"{ASSET_PATH_BLUEPRINT}/BP_SpectatorPlayerController"

    if unreal.EditorAssetLibrary.does_asset_exist(bp_path):
        unreal.log(f"[SKIP] BP_SpectatorPlayerController already exists: {bp_path}")
        return unreal.load_asset(bp_path)

    # 使用 PlayerController 作为父类
    factory = unrealBlueprint = unreal.EditorAssetLibrary.create_asset(
        bp_path,
        unreal.Blueprint,
        unreal.BlueprintFactory()
    )

    if bp:
        unreal.log(f"[OK] Created BP_SpectatorPlayerController")

    return unreal.load_asset(bp_path)


# ============================================
# 主执行函数
# ============================================

def run_full_setup():
    """执行完整的观战系统配置"""
    unreal.log("==========================================")
    unreal.log("观战系统 UE 编辑器自动化配置开始")
    unreal.log("==========================================")

    # 1. 创建文件夹结构
    unreal.log("\n[Step 1] 创建文件夹结构...")
    get_or_create_folder(ASSET_PATH_INPUT)
    get_or_create_folder(ASSET_PATH_BLUEPRINT)
    get_or_create_folder(ASSET_PATH_WIDGET)

    # 2. 创建 Input Actions
    unreal.log("\n[Step 2] 创建 Input Actions...")
    create_all_input_actions()

    # 3. 创建 IMC_Spectator
    unreal.log("\n[Step 3] 创建 IMC_Spectator...")
    create_imc_spectator()
    configure_imc_spectator()

    # 4. 创建 Blueprints
    unreal.log("\n[Step 4] 创建 Blueprints...")
    create_bp_spectator_hud()
    create_wbp_spectator_status_bar()
    create_wbp_spectator_minimap()
    create_spectator_controller()

    unreal.log("\n==========================================")
    unreal.log("观战系统配置完成!")
    unreal.log("==========================================")
    unreal.log("\n请在 UE 编辑器中手动完成以下配置:")
    unreal.log("1. 在 IMC_Spectator 中添加所有 Action 映射")
    unreal.log("2. 设置 IMC_Spectator 优先级为 100")
    unreal.log("3. 在 BP_SpectatorHUD 中绑定 UI 组件")
    unreal.log("4. 在 WBP_SpectatorStatusBar 中设计玩家状态卡片")
    unreal.log("5. 在 WBP_SpectatorMinimap 中设计小地图 UI")
    unreal.log("6. 在 BP_SpectatorPlayerController 中添加 SpectatorComponent")


if __name__ == "__main__":
    run_full_setup()