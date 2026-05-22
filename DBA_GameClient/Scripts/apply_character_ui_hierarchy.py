# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


FRONTEND_DIR = "/Game/DBA/UI/Frontend/Character"

SELECT_ASSET_NAME = "WBP_DBA_CharacterSelect"
CREATE_ASSET_NAME = "WBP_DBA_CharacterCreate"

SELECT_PARENT_CLASS_NAME = "DBACharacterSelectFlowWidgetBase"
CREATE_PARENT_CLASS_NAME = "DBACharacterCreateFlowWidgetBase"


def log(msg: str):
    unreal.log(f"[ApplyCharacterUIHierarchy] {msg}")


def ensure_parent_class(class_name: str):
    cls = getattr(unreal, class_name, None)
    if not cls:
        raise RuntimeError(f"Parent class not found in Python API: {class_name}")
    return cls


def backup_and_delete_asset(asset_path: str):
    if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        return

    backup_path = f"{asset_path}__AutoBackup"
    if not unreal.EditorAssetLibrary.does_asset_exist(backup_path):
        ok = unreal.EditorAssetLibrary.duplicate_asset(asset_path, backup_path)
        if not ok:
            raise RuntimeError(f"Failed to backup asset: {asset_path} -> {backup_path}")
        log(f"Backup created: {backup_path}")

    ok = unreal.EditorAssetLibrary.delete_asset(asset_path)
    if not ok:
        raise RuntimeError(f"Failed to delete original asset before rebuild: {asset_path}")
    log(f"Deleted original asset: {asset_path}")


def create_widget_blueprint(asset_name: str, parent_class):
    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset = asset_tools.create_asset(asset_name, FRONTEND_DIR, unreal.WidgetBlueprint, factory)
    if not asset:
        raise RuntimeError(f"Failed to create widget blueprint: {asset_name}")
    return asset


def as_widget_blueprint(asset):
    cast_result = unreal.EditorUtilityLibrary.cast_to_widget_blueprint(asset)
    if isinstance(cast_result, tuple):
        widget_bp = cast_result[1]
    else:
        widget_bp = cast_result
    if not widget_bp:
        raise RuntimeError(f"Failed to cast to WidgetBlueprint: {asset.get_name()}")
    return widget_bp


def add_source_widget(widget_bp, widget_class, widget_name: str, parent_name: str = ""):
    parent = unreal.Name(parent_name) if parent_name else unreal.Name("")
    widget = unreal.EditorUtilityLibrary.add_source_widget(widget_bp, widget_class, widget_name, parent)
    if not widget:
        raise RuntimeError(f"Failed to add widget '{widget_name}' under '{parent_name or '<root>'}'")
    return widget


def set_canvas_slot(widget, min_x, min_y, max_x, max_y, left=0.0, top=0.0, right=0.0, bottom=0.0, align_x=0.0, align_y=0.0):
    slot = widget.get_editor_property("slot")
    canvas_slot = unreal.CanvasPanelSlot.cast(slot)
    if not canvas_slot:
        raise RuntimeError(f"Widget '{widget.get_name()}' does not use CanvasPanelSlot")

    anchors = unreal.Anchors(minimum=unreal.Vector2D(min_x, min_y), maximum=unreal.Vector2D(max_x, max_y))
    canvas_slot.set_anchors(anchors)
    canvas_slot.set_offsets(unreal.Margin(left, top, right, bottom))
    canvas_slot.set_alignment(unreal.Vector2D(align_x, align_y))


def set_as_variable(widget):
    try:
        widget.set_editor_property("bIsVariable", True)
    except Exception:
        try:
            widget.set_editor_property("b_is_variable", True)
        except Exception:
            pass


def build_select_widget(widget_bp):
    add_source_widget(widget_bp, unreal.CanvasPanel, "RootCanvas_Auto")

    preview_host = add_source_widget(widget_bp, unreal.CanvasPanel, "CharacterPreviewHost", "RootCanvas_Auto")
    set_canvas_slot(preview_host, 0.24, 0.05, 0.76, 0.95)
    set_as_variable(preview_host)

    character_list_text = add_source_widget(widget_bp, unreal.TextBlock, "CharacterListText", "RootCanvas_Auto")
    character_list_text.set_text(unreal.Text("Characters"))
    set_canvas_slot(character_list_text, 0.03, 0.17, 0.21, 0.74)
    set_as_variable(character_list_text)

    refresh_button = add_source_widget(widget_bp, unreal.Button, "RefreshButton", "RootCanvas_Auto")
    set_canvas_slot(refresh_button, 0.03, 0.78, 0.21, 0.84)
    set_as_variable(refresh_button)
    refresh_label = add_source_widget(widget_bp, unreal.TextBlock, "RefreshButtonLabel", "RefreshButton")
    refresh_label.set_text(unreal.Text("Refresh"))

    confirm_button = add_source_widget(widget_bp, unreal.Button, "ConfirmButton", "RootCanvas_Auto")
    set_canvas_slot(confirm_button, 0.80, 0.73, 0.96, 0.79)
    set_as_variable(confirm_button)
    confirm_label = add_source_widget(widget_bp, unreal.TextBlock, "ConfirmButtonLabel", "ConfirmButton")
    confirm_label.set_text(unreal.Text("Enter Lobby"))

    create_button = add_source_widget(widget_bp, unreal.Button, "CreateButton", "RootCanvas_Auto")
    set_canvas_slot(create_button, 0.80, 0.82, 0.96, 0.88)
    set_as_variable(create_button)
    create_label = add_source_widget(widget_bp, unreal.TextBlock, "CreateButtonLabel", "CreateButton")
    create_label.set_text(unreal.Text("Create Character"))

    status_text = add_source_widget(widget_bp, unreal.TextBlock, "StatusText", "RootCanvas_Auto")
    status_text.set_text(unreal.Text(""))
    set_canvas_slot(status_text, 0.79, 0.90, 0.97, 0.96)
    set_as_variable(status_text)


def build_create_widget(widget_bp):
    add_source_widget(widget_bp, unreal.CanvasPanel, "RootCanvas_Auto")

    preview_host = add_source_widget(widget_bp, unreal.CanvasPanel, "CharacterPreviewHost", "RootCanvas_Auto")
    set_canvas_slot(preview_host, 0.24, 0.05, 0.76, 0.95)
    set_as_variable(preview_host)

    name_input = add_source_widget(widget_bp, unreal.EditableTextBox, "CharacterNameInput", "RootCanvas_Auto")
    name_input.set_hint_text(unreal.Text("Character Name"))
    set_canvas_slot(name_input, 0.78, 0.23, 0.96, 0.29)
    set_as_variable(name_input)

    zodiac_button = add_source_widget(widget_bp, unreal.Button, "ZodiacButton", "RootCanvas_Auto")
    set_canvas_slot(zodiac_button, 0.78, 0.33, 0.96, 0.39)
    set_as_variable(zodiac_button)
    zodiac_text = add_source_widget(widget_bp, unreal.TextBlock, "ZodiacText", "ZodiacButton")
    zodiac_text.set_text(unreal.Text("Zodiac: Rat"))
    set_as_variable(zodiac_text)

    element_button = add_source_widget(widget_bp, unreal.Button, "ElementButton", "RootCanvas_Auto")
    set_canvas_slot(element_button, 0.78, 0.43, 0.96, 0.49)
    set_as_variable(element_button)
    element_text = add_source_widget(widget_bp, unreal.TextBlock, "ElementText", "ElementButton")
    element_text.set_text(unreal.Text("Element: Water"))
    set_as_variable(element_text)

    camp_button = add_source_widget(widget_bp, unreal.Button, "FiveCampButton", "RootCanvas_Auto")
    set_canvas_slot(camp_button, 0.78, 0.53, 0.96, 0.59)
    set_as_variable(camp_button)
    camp_text = add_source_widget(widget_bp, unreal.TextBlock, "FiveCampText", "FiveCampButton")
    camp_text.set_text(unreal.Text("Camp: None"))
    set_as_variable(camp_text)

    create_button = add_source_widget(widget_bp, unreal.Button, "CreateButton", "RootCanvas_Auto")
    set_canvas_slot(create_button, 0.78, 0.70, 0.96, 0.76)
    set_as_variable(create_button)
    create_label = add_source_widget(widget_bp, unreal.TextBlock, "CreateButtonLabel", "CreateButton")
    create_label.set_text(unreal.Text("Create and Enter"))

    back_button = add_source_widget(widget_bp, unreal.Button, "BackButton", "RootCanvas_Auto")
    set_canvas_slot(back_button, 0.78, 0.79, 0.96, 0.85)
    set_as_variable(back_button)
    back_label = add_source_widget(widget_bp, unreal.TextBlock, "BackButtonLabel", "BackButton")
    back_label.set_text(unreal.Text("Back to Select"))

    validation_text = add_source_widget(widget_bp, unreal.TextBlock, "ValidationText", "RootCanvas_Auto")
    validation_text.set_text(unreal.Text(""))
    set_canvas_slot(validation_text, 0.78, 0.87, 0.96, 0.92)
    set_as_variable(validation_text)


def save_asset(asset):
    ok = unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)
    if not ok:
        raise RuntimeError(f"Failed to save asset: {asset.get_name()}")


def rebuild_one(asset_name: str, parent_class_name: str, builder_fn):
    asset_path = f"{FRONTEND_DIR}/{asset_name}"
    backup_and_delete_asset(asset_path)

    parent_class = ensure_parent_class(parent_class_name)
    asset = create_widget_blueprint(asset_name, parent_class)
    widget_bp = as_widget_blueprint(asset)
    builder_fn(widget_bp)
    save_asset(asset)
    log(f"Rebuilt widget blueprint: {asset_path}")


def main():
    rebuild_one(SELECT_ASSET_NAME, SELECT_PARENT_CLASS_NAME, build_select_widget)
    rebuild_one(CREATE_ASSET_NAME, CREATE_PARENT_CLASS_NAME, build_create_widget)
    log("All done")


main()
