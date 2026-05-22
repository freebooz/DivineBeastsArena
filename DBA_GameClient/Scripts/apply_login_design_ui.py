# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

import unreal


ASSET_DIR = "/Game/DBA/UI/Frontend/Login"
ASSET_NAME = "WBP_DBA_Login"
ASSET_PATH = f"{ASSET_DIR}/{ASSET_NAME}"
PARENT_CLASS_NAME = "DBALoginFlowWidgetBase"

BACKGROUND_TEXTURE = "/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginForestSanctuary.T_DBA_LoginForestSanctuary"
PANEL_TEXTURE = "/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginPanel_StoneGold.T_DBA_LoginPanel_StoneGold"
BUTTON_TEXTURE = "/Game/DBA/UI/Lobby/Login/Textures/T_DBA_LoginButton_ParchmentGold.T_DBA_LoginButton_ParchmentGold"


def log(msg: str):
    unreal.log(f"[ApplyLoginDesignUI] {msg}")


def ensure_parent_class():
    parent_class = getattr(unreal, PARENT_CLASS_NAME, None)
    if not parent_class:
        raise RuntimeError(f"Parent class not found in Python API: {PARENT_CLASS_NAME}")
    return parent_class


def load_texture(path: str):
    texture = unreal.EditorAssetLibrary.load_asset(path)
    if not texture:
        log(f"texture missing: {path}")
    return texture


def backup_and_delete_asset():
    if not unreal.EditorAssetLibrary.does_asset_exist(ASSET_PATH):
        return

    backup_path = f"{ASSET_PATH}__DesignBackup"
    if not unreal.EditorAssetLibrary.does_asset_exist(backup_path):
        if not unreal.EditorAssetLibrary.duplicate_asset(ASSET_PATH, backup_path):
            raise RuntimeError(f"Failed to backup asset: {ASSET_PATH}")
        log(f"backup created: {backup_path}")

    if not unreal.EditorAssetLibrary.delete_asset(ASSET_PATH):
        raise RuntimeError(f"Failed to delete original asset before rebuild: {ASSET_PATH}")
    log(f"deleted original asset: {ASSET_PATH}")


def create_widget_blueprint():
    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", ensure_parent_class())
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset = asset_tools.create_asset(ASSET_NAME, ASSET_DIR, unreal.WidgetBlueprint, factory)
    if not asset:
        raise RuntimeError(f"Failed to create widget blueprint: {ASSET_PATH}")
    cast_result = unreal.EditorUtilityLibrary.cast_to_widget_blueprint(asset)
    widget_bp = cast_result[1] if isinstance(cast_result, tuple) else cast_result
    if not widget_bp:
        raise RuntimeError(f"Failed to cast widget blueprint: {ASSET_PATH}")
    return asset, widget_bp


def add_widget(widget_bp, widget_class, name: str, parent_name: str = ""):
    parent = unreal.Name(parent_name) if parent_name else unreal.Name("")
    widget = unreal.EditorUtilityLibrary.add_source_widget(widget_bp, widget_class, name, parent)
    if not widget:
        raise RuntimeError(f"Failed to add widget '{name}' under '{parent_name or '<root>'}'")
    return widget


def set_as_variable(widget):
    for prop_name in ("bIsVariable", "b_is_variable"):
        try:
            widget.set_editor_property(prop_name, True)
            return
        except Exception:
            pass


def set_canvas_slot(widget, min_x, min_y, max_x, max_y, left, top, width, height, align_x=0.0, align_y=0.0):
    slot = widget.get_editor_property("slot")
    canvas_slot = unreal.CanvasPanelSlot.cast(slot)
    if not canvas_slot:
        raise RuntimeError(f"Widget '{widget.get_name()}' does not use CanvasPanelSlot")
    canvas_slot.set_anchors(unreal.Anchors(minimum=unreal.Vector2D(min_x, min_y), maximum=unreal.Vector2D(max_x, max_y)))
    canvas_slot.set_offsets(unreal.Margin(left, top, width, height))
    canvas_slot.set_alignment(unreal.Vector2D(align_x, align_y))


def set_text(widget, text: str, size: int = 24, color=None, justify=unreal.TextJustify.CENTER):
    widget.set_text(unreal.Text(text))
    try:
        widget.set_justification(justify)
    except Exception:
        try:
            widget.set_editor_property("justification", justify)
        except Exception:
            pass
    if color is not None:
        try:
            widget.set_color_and_opacity(unreal.SlateColor(color))
        except Exception:
            try:
                widget.set_editor_property("color_and_opacity", unreal.SlateColor(color))
            except Exception:
                pass
    try:
        font = widget.get_font()
        font.size = size
        widget.set_font(font)
    except Exception:
        pass


def tint_border(widget, color):
    try:
        widget.set_brush_color(color)
    except Exception:
        pass


def set_image_texture(widget, texture, color=unreal.LinearColor.WHITE):
    if texture:
        widget.set_brush_from_texture(texture, True)
    widget.set_color_and_opacity(color)


def add_text(widget_bp, name, parent_name, text, size, color, justify=unreal.TextJustify.CENTER):
    text_block = add_widget(widget_bp, unreal.TextBlock, name, parent_name)
    set_text(text_block, text, size, color, justify)
    return text_block


def add_label_button(widget_bp, button_name, label_name, parent_name, label, size=26, color=unreal.LinearColor(0.95, 0.77, 0.36, 1.0)):
    button = add_widget(widget_bp, unreal.Button, button_name, parent_name)
    set_as_variable(button)
    label_widget = add_text(widget_bp, label_name, button_name, label, size, color)
    set_as_variable(label_widget)
    return button


def build_login_widget(widget_bp):
    background_texture = load_texture(BACKGROUND_TEXTURE)
    panel_texture = load_texture(PANEL_TEXTURE)
    button_texture = load_texture(BUTTON_TEXTURE)

    gold = unreal.LinearColor(0.95, 0.77, 0.36, 1.0)
    pale_gold = unreal.LinearColor(1.0, 0.88, 0.55, 1.0)
    muted_gold = unreal.LinearColor(0.76, 0.62, 0.36, 1.0)
    dark_panel = unreal.LinearColor(0.015, 0.055, 0.075, 0.82)
    blue = unreal.LinearColor(0.17, 0.55, 0.95, 1.0)
    green = unreal.LinearColor(0.39, 1.0, 0.30, 1.0)

    root = add_widget(widget_bp, unreal.CanvasPanel, "RootCanvas_Auto")
    set_as_variable(root)

    bg = add_widget(widget_bp, unreal.Image, "ForestBackgroundImage", "RootCanvas_Auto")
    set_image_texture(bg, background_texture)
    set_canvas_slot(bg, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0)

    vignette = add_widget(widget_bp, unreal.Border, "DarkVignetteOverlay", "RootCanvas_Auto")
    tint_border(vignette, unreal.LinearColor(0.0, 0.015, 0.018, 0.34))
    set_canvas_slot(vignette, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0)

    for name, anchor, offsets in [
        ("FrameTopLine", (0.0, 0.0, 1.0, 0.0), (6.0, 6.0, -12.0, 2.0)),
        ("FrameBottomLine", (0.0, 1.0, 1.0, 1.0), (6.0, -8.0, -12.0, 2.0)),
        ("FrameLeftLine", (0.0, 0.0, 0.0, 1.0), (6.0, 6.0, 2.0, -12.0)),
        ("FrameRightLine", (1.0, 0.0, 1.0, 1.0), (-8.0, 6.0, 2.0, -12.0)),
    ]:
        line = add_widget(widget_bp, unreal.Border, name, "RootCanvas_Auto")
        tint_border(line, unreal.LinearColor(0.88, 0.67, 0.28, 0.88))
        set_canvas_slot(line, *anchor, *offsets)

    title_back = add_widget(widget_bp, unreal.Border, "TitleBackplate", "RootCanvas_Auto")
    tint_border(title_back, unreal.LinearColor(0.02, 0.075, 0.10, 0.50))
    set_canvas_slot(title_back, 0.5, 0.035, 0.5, 0.035, 0.0, 0.0, 820.0, 160.0, 0.5, 0.0)

    title = add_text(widget_bp, "TitleText", "RootCanvas_Auto", "神兽竞技场", 92, pale_gold)
    set_as_variable(title)
    set_canvas_slot(title, 0.5, 0.06, 0.5, 0.06, 0.0, 0.0, 760.0, 120.0, 0.5, 0.0)

    panel = add_widget(widget_bp, unreal.Border, "LoginPanel", "RootCanvas_Auto")
    set_as_variable(panel)
    if panel_texture:
        panel.set_brush_from_texture(panel_texture)
        panel.set_brush_color(unreal.LinearColor(1.0, 1.0, 1.0, 0.96))
    else:
        tint_border(panel, dark_panel)
    try:
        panel.set_padding(unreal.Margin(0.0, 0.0, 0.0, 0.0))
    except Exception:
        pass
    set_canvas_slot(panel, 0.5, 0.32, 0.5, 0.32, 0.0, 0.0, 700.0, 540.0, 0.5, 0.0)

    form = add_widget(widget_bp, unreal.CanvasPanel, "LoginFormCanvas", "LoginPanel")
    set_as_variable(form)

    server_label = add_text(widget_bp, "ServerLabelText", "LoginFormCanvas", "服务器：", 24, pale_gold, unreal.TextJustify.RIGHT)
    set_canvas_slot(server_label, 0.0, 0.0, 0.0, 0.0, 74.0, 44.0, 90.0, 36.0)

    server_button = add_widget(widget_bp, unreal.Button, "ServerButton", "LoginFormCanvas")
    set_as_variable(server_button)
    set_canvas_slot(server_button, 0.0, 0.0, 0.0, 0.0, 182.0, 32.0, 320.0, 54.0)
    server_name = add_text(widget_bp, "ServerNameText", "ServerButton", "苍穹之森", 24, pale_gold)
    set_as_variable(server_name)

    server_arrow = add_text(widget_bp, "ServerArrowText", "LoginFormCanvas", "▼", 22, pale_gold)
    set_canvas_slot(server_arrow, 0.0, 0.0, 0.0, 0.0, 462.0, 43.0, 34.0, 34.0)

    server_status = add_text(widget_bp, "ServerStatusText", "LoginFormCanvas", "● 流畅", 22, green, unreal.TextJustify.LEFT)
    set_canvas_slot(server_status, 0.0, 0.0, 0.0, 0.0, 520.0, 44.0, 130.0, 36.0)

    account_row = add_widget(widget_bp, unreal.Border, "AccountRowFrame", "LoginFormCanvas")
    tint_border(account_row, unreal.LinearColor(0.02, 0.045, 0.075, 0.84))
    set_canvas_slot(account_row, 0.0, 0.0, 0.0, 0.0, 76.0, 112.0, 560.0, 58.0)
    add_text(widget_bp, "AccountIconText", "LoginFormCanvas", "●", 30, pale_gold)
    set_canvas_slot(find_widget(widget_bp, "AccountIconText"), 0.0, 0.0, 0.0, 0.0, 96.0, 124.0, 36.0, 36.0)
    account_label = add_text(widget_bp, "AccountLabelText", "LoginFormCanvas", "账号", 24, pale_gold, unreal.TextJustify.LEFT)
    set_canvas_slot(account_label, 0.0, 0.0, 0.0, 0.0, 142.0, 124.0, 64.0, 36.0)

    email_host = add_widget(widget_bp, unreal.CanvasPanel, "EmailInputHost", "LoginFormCanvas")
    set_as_variable(email_host)
    set_canvas_slot(email_host, 0.0, 0.0, 0.0, 0.0, 220.0, 118.0, 392.0, 46.0)

    pass_row = add_widget(widget_bp, unreal.Border, "PasswordRowFrame", "LoginFormCanvas")
    tint_border(pass_row, unreal.LinearColor(0.02, 0.045, 0.075, 0.84))
    set_canvas_slot(pass_row, 0.0, 0.0, 0.0, 0.0, 76.0, 188.0, 560.0, 58.0)
    add_text(widget_bp, "PasswordIconText", "LoginFormCanvas", "■", 28, pale_gold)
    set_canvas_slot(find_widget(widget_bp, "PasswordIconText"), 0.0, 0.0, 0.0, 0.0, 98.0, 199.0, 34.0, 36.0)
    password_label = add_text(widget_bp, "PasswordLabelText", "LoginFormCanvas", "密码", 24, pale_gold, unreal.TextJustify.LEFT)
    set_canvas_slot(password_label, 0.0, 0.0, 0.0, 0.0, 142.0, 200.0, 64.0, 36.0)

    password_host = add_widget(widget_bp, unreal.CanvasPanel, "PasswordInputHost", "LoginFormCanvas")
    set_as_variable(password_host)
    set_canvas_slot(password_host, 0.0, 0.0, 0.0, 0.0, 220.0, 194.0, 392.0, 46.0)

    eye = add_text(widget_bp, "PasswordEyeText", "LoginFormCanvas", "◇", 28, pale_gold)
    set_canvas_slot(eye, 0.0, 0.0, 0.0, 0.0, 594.0, 200.0, 30.0, 34.0)

    remember = add_text(widget_bp, "RememberText", "LoginFormCanvas", "□ 记住我", 22, pale_gold, unreal.TextJustify.LEFT)
    set_canvas_slot(remember, 0.0, 0.0, 0.0, 0.0, 82.0, 264.0, 160.0, 34.0)
    forgot = add_text(widget_bp, "ForgotPasswordText", "LoginFormCanvas", "忘记密码", 22, pale_gold, unreal.TextJustify.RIGHT)
    set_canvas_slot(forgot, 0.0, 0.0, 0.0, 0.0, 470.0, 264.0, 150.0, 34.0)

    login_button = add_label_button(widget_bp, "LoginButton", "LoginButtonLabel", "LoginFormCanvas", "登录", 42, unreal.LinearColor(0.28, 0.13, 0.02, 1.0))
    if button_texture:
        pass
    set_canvas_slot(login_button, 0.0, 0.0, 0.0, 0.0, 130.0, 314.0, 440.0, 82.0)

    guest_button = add_label_button(widget_bp, "GuestLoginButton", "GuestLoginButtonLabel", "LoginFormCanvas", "游客登录", 30, pale_gold)
    set_canvas_slot(guest_button, 0.0, 0.0, 0.0, 0.0, 214.0, 414.0, 270.0, 58.0)

    register = add_text(widget_bp, "RegisterAccountText", "LoginFormCanvas", "注册账号", 24, pale_gold)
    set_canvas_slot(register, 0.0, 0.0, 0.0, 0.0, 270.0, 492.0, 160.0, 36.0)

    error = add_text(widget_bp, "ErrorText", "LoginFormCanvas", "", 18, unreal.LinearColor(1.0, 0.35, 0.24, 1.0))
    set_as_variable(error)
    set_canvas_slot(error, 0.0, 0.0, 0.0, 0.0, 86.0, 472.0, 530.0, 28.0)

    status = add_text(widget_bp, "StatusText", "LoginFormCanvas", "", 18, unreal.LinearColor(0.7, 1.0, 0.72, 1.0))
    set_as_variable(status)
    set_canvas_slot(status, 0.0, 0.0, 0.0, 0.0, 86.0, 500.0, 530.0, 28.0)

    right_tools = add_widget(widget_bp, unreal.CanvasPanel, "RightToolPanel", "RootCanvas_Auto")
    set_as_variable(right_tools)
    set_canvas_slot(right_tools, 0.94, 0.42, 0.94, 0.42, 0.0, 0.0, 118.0, 360.0, 0.5, 0.0)
    for index, (button_name, label_name, label) in enumerate([
        ("NoticeToolButton", "NoticeToolLabel", "公告"),
        ("SupportToolButton", "SupportToolLabel", "客服"),
        ("RepairToolButton", "RepairToolLabel", "修复"),
    ]):
        add_label_button(widget_bp, button_name, label_name, "RightToolPanel", label, 26, pale_gold)
        set_canvas_slot(find_widget(widget_bp, button_name), 0.0, 0.0, 0.0, 0.0, 0.0, float(index * 120), 118.0, 96.0)

    age_panel = add_widget(widget_bp, unreal.Border, "AgeRatingPanel", "RootCanvas_Auto")
    tint_border(age_panel, unreal.LinearColor(1.0, 0.74, 0.22, 0.92))
    set_canvas_slot(age_panel, 0.03, 0.82, 0.03, 0.82, 0.0, 0.0, 100.0, 132.0, 0.0, 0.0)
    age_text = add_text(widget_bp, "AgeRatingText", "AgeRatingPanel", "16+\nCADPA\n适龄提示", 22, unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    set_as_variable(age_text)

    agreement = add_text(widget_bp, "AgreementText", "RootCanvas_Auto", "☑ 我已详细阅读并同意《用户协议》和《隐私政策》", 22, unreal.LinearColor(0.92, 0.88, 0.74, 1.0))
    set_as_variable(agreement)
    set_canvas_slot(agreement, 0.5, 0.91, 0.5, 0.91, 0.0, 0.0, 760.0, 42.0, 0.5, 0.0)


def find_widget(widget_bp, name: str):
    return unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_bp, unreal.Name(name))


def main():
    backup_and_delete_asset()
    asset, widget_bp = create_widget_blueprint()
    build_login_widget(widget_bp)
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"save failed: {ASSET_PATH}")
    log(f"rebuilt widget blueprint: {ASSET_PATH}")


main()
