# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：项目工具脚本，用于资产整理、模拟服务或本地开发辅助流程。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


PANEL_TEXTURE = "/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginPanel_StoneGold.T_DBA_LoginPanel_StoneGold"


def log(message):
    unreal.log(f"[CreateCorePanelWidgetBlueprints] {message}")


def parent_class(name):
    cls = getattr(unreal, name, None)
    if not cls:
        raise RuntimeError(f"Parent class not found: {name}")
    return cls


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def rebuild_widget(asset_dir, asset_name, parent_name):
    ensure_dir(asset_dir)
    asset_path = f"{asset_dir}/{asset_name}"
    backup_path = f"{asset_path}__AutoBackup"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        if not unreal.EditorAssetLibrary.does_asset_exist(backup_path):
            unreal.EditorAssetLibrary.duplicate_asset(asset_path, backup_path)
        unreal.EditorAssetLibrary.delete_asset(asset_path)

    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class(parent_name))
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(asset_name, asset_dir, unreal.WidgetBlueprint, factory)
    if not asset:
        raise RuntimeError(f"Failed to create {asset_path}")

    cast_result = unreal.EditorUtilityLibrary.cast_to_widget_blueprint(asset)
    widget_bp = cast_result[1] if isinstance(cast_result, tuple) else cast_result
    if not widget_bp:
        raise RuntimeError(f"Failed to cast {asset_path} to WidgetBlueprint")
    return asset, widget_bp


def add(widget_bp, widget_class, name, parent=""):
    widget = unreal.EditorUtilityLibrary.add_source_widget(widget_bp, widget_class, name, unreal.Name(parent) if parent else unreal.Name(""))
    if not widget:
        raise RuntimeError(f"Failed to add widget {name}")
    return widget


def var(widget):
    for prop_name in ("bIsVariable", "b_is_variable"):
        try:
            widget.set_editor_property(prop_name, True)
            return
        except Exception:
            pass


def canvas_slot(widget, min_x, min_y, max_x, max_y, left, top, width, height, align_x=0.0, align_y=0.0):
    slot = unreal.CanvasPanelSlot.cast(widget.get_editor_property("slot"))
    if not slot:
        raise RuntimeError(f"{widget.get_name()} is not in a CanvasPanelSlot")
    slot.set_anchors(unreal.Anchors(minimum=unreal.Vector2D(min_x, min_y), maximum=unreal.Vector2D(max_x, max_y)))
    slot.set_offsets(unreal.Margin(left, top, width, height))
    slot.set_alignment(unreal.Vector2D(align_x, align_y))


def text(widget_bp, name, parent, value, size=22):
    block = add(widget_bp, unreal.TextBlock, name, parent)
    block.set_text(unreal.Text(value))
    try:
        font = block.get_font()
        font.size = size
        block.set_font(font)
    except Exception:
        pass
    return block


def button(widget_bp, button_name, label_name, parent, label):
    btn = add(widget_bp, unreal.Button, button_name, parent)
    var(btn)
    text(widget_bp, label_name, button_name, label, 20)
    return btn


def apply_panel_texture(border):
    texture = unreal.EditorAssetLibrary.load_asset(PANEL_TEXTURE)
    if texture:
        border.set_brush_from_texture(texture)
        border.set_brush_color(unreal.LinearColor(1.0, 1.0, 1.0, 0.96))
    else:
        border.set_brush_color(unreal.LinearColor(0.08, 0.065, 0.045, 0.96))


def build_settings(widget_bp):
    add(widget_bp, unreal.CanvasPanel, "RootCanvas")
    panel = add(widget_bp, unreal.Border, "PanelBackgroundBorder", "RootCanvas")
    var(panel)
    apply_panel_texture(panel)
    panel.set_padding(unreal.Margin(42.0, 38.0, 42.0, 34.0))
    canvas_slot(panel, 0.5, 0.5, 0.5, 0.5, 0.0, 0.0, 680.0, 520.0, 0.5, 0.5)

    root = add(widget_bp, unreal.VerticalBox, "SettingsContent", "PanelBackgroundBorder")
    text(widget_bp, "SettingsTitleText", "SettingsContent", "游戏设置", 30)
    status = text(widget_bp, "StatusText", "SettingsContent", "", 18)
    var(status)

    for slider_name, label in [
        ("MasterVolumeSlider", "主音量"),
        ("MouseSensitivitySlider", "鼠标灵敏度"),
        ("CameraDistanceSlider", "摄像机距离"),
    ]:
        row = add(widget_bp, unreal.HorizontalBox, f"{slider_name}Row", "SettingsContent")
        text(widget_bp, f"{slider_name}Label", row.get_name(), label, 20)
        slider = add(widget_bp, unreal.Slider, slider_name, row.get_name())
        var(slider)
        slider.set_min_value(0.0)
        slider.set_max_value(1.0)
        slider.set_step_size(0.01)

    fullscreen_row = add(widget_bp, unreal.HorizontalBox, "FullscreenRow", "SettingsContent")
    text(widget_bp, "FullscreenLabel", "FullscreenRow", "无边框全屏", 20)
    checkbox = add(widget_bp, unreal.CheckBox, "FullscreenCheckBox", "FullscreenRow")
    var(checkbox)

    row = add(widget_bp, unreal.HorizontalBox, "SettingsButtonRow", "SettingsContent")
    button(widget_bp, "ApplyButton", "ApplyButtonText", "SettingsButtonRow", "应用")
    button(widget_bp, "ResetButton", "ResetButtonText", "SettingsButtonRow", "重置")
    button(widget_bp, "CloseButton", "CloseButtonText", "SettingsButtonRow", "关闭")


def build_inventory(widget_bp):
    add(widget_bp, unreal.CanvasPanel, "RootCanvas")
    panel = add(widget_bp, unreal.Border, "PanelBackgroundBorder", "RootCanvas")
    var(panel)
    apply_panel_texture(panel)
    panel.set_padding(unreal.Margin(42.0, 38.0, 42.0, 34.0))
    canvas_slot(panel, 0.5, 0.5, 0.5, 0.5, 0.0, 0.0, 760.0, 540.0, 0.5, 0.5)

    root = add(widget_bp, unreal.VerticalBox, "InventoryContent", "PanelBackgroundBorder")
    text(widget_bp, "InventoryTitleText", "InventoryContent", "背包", 30)
    status = text(widget_bp, "StatusText", "InventoryContent", "", 18)
    var(status)
    items = text(widget_bp, "ItemsText", "InventoryContent", "暂无物品", 20)
    details = text(widget_bp, "DetailsText", "InventoryContent", "未选择物品", 18)
    var(items)
    var(details)

    row = add(widget_bp, unreal.HorizontalBox, "InventoryButtonRow", "InventoryContent")
    button(widget_bp, "PreviousButton", "PreviousButtonText", "InventoryButtonRow", "上一个")
    button(widget_bp, "NextButton", "NextButtonText", "InventoryButtonRow", "下一个")
    button(widget_bp, "UseButton", "UseButtonText", "InventoryButtonRow", "使用")
    button(widget_bp, "SortButton", "SortButtonText", "InventoryButtonRow", "排序")
    button(widget_bp, "RefreshButton", "RefreshButtonText", "InventoryButtonRow", "刷新")
    button(widget_bp, "CloseButton", "CloseButtonText", "InventoryButtonRow", "关闭")


def main():
    settings_asset, settings_bp = rebuild_widget("/Game/DBA/UI/Lobby/Settings", "WBP_DBA_GameSettings", "DBAGameSettingsWidgetBase")
    build_settings(settings_bp)
    unreal.EditorAssetLibrary.save_loaded_asset(settings_asset, only_if_is_dirty=False)
    log("Saved /Game/DBA/UI/Lobby/Settings/WBP_DBA_GameSettings")

    inventory_asset, inventory_bp = rebuild_widget("/Game/DBA/UI/Lobby/Inventory", "WBP_DBA_Inventory", "DBAInventoryWidgetBase")
    build_inventory(inventory_bp)
    unreal.EditorAssetLibrary.save_loaded_asset(inventory_asset, only_if_is_dirty=False)
    log("Saved /Game/DBA/UI/Lobby/Inventory/WBP_DBA_Inventory")


main()
