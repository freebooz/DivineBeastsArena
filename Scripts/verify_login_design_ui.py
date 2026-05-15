import unreal


LOGIN_ASSET = "/Game/DBA/UI/Frontend/Login/WBP_DBA_Login"


def log(msg: str):
    unreal.log(f"[VerifyLoginDesignUI] {msg}")


def cast_widget_bp(asset_path: str):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        raise RuntimeError(f"load failed: {asset_path}")
    cast_result = unreal.EditorUtilityLibrary.cast_to_widget_blueprint(asset)
    widget_bp = cast_result[1] if isinstance(cast_result, tuple) else cast_result
    if not widget_bp:
        raise RuntimeError(f"cast widget blueprint failed: {asset_path}")
    return widget_bp


def find_widget(widget_bp, name: str):
    return unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_bp, unreal.Name(name))


def require_widget(widget_bp, name: str, class_name: str = ""):
    widget = find_widget(widget_bp, name)
    if not widget:
        raise RuntimeError(f"missing widget: {name}")
    if class_name and widget.get_class().get_name() != class_name:
        raise RuntimeError(f"{name} class mismatch: expected {class_name}, got {widget.get_class().get_name()}")
    return widget


def require_text(widget_bp, name: str, expected: str):
    widget = require_widget(widget_bp, name, "TextBlock")
    text = str(widget.get_text())
    if text != expected:
        raise RuntimeError(f"{name} text mismatch: expected '{expected}', got '{text}'")
    log(f"{name}: {text}")


def require_hint(widget_bp, name: str, expected: str):
    widget = require_widget(widget_bp, name, "EditableTextBox")
    try:
        hint = str(widget.get_hint_text())
    except Exception:
        hint = str(widget.get_editor_property("hint_text"))
    if hint != expected:
        raise RuntimeError(f"{name} hint mismatch: expected '{expected}', got '{hint}'")
    log(f"{name}.Hint: {hint}")


def slot_summary(widget):
    slot = widget.get_editor_property("slot")
    canvas_slot = unreal.CanvasPanelSlot.cast(slot)
    if not canvas_slot:
        return "not-canvas-slot"
    anchors = canvas_slot.get_anchors()
    offsets = canvas_slot.get_offsets()
    align = canvas_slot.get_alignment()
    return (
        f"anchors=({anchors.minimum.x:.2f},{anchors.minimum.y:.2f})-"
        f"({anchors.maximum.x:.2f},{anchors.maximum.y:.2f}), "
        f"offsets=({offsets.left:.1f},{offsets.top:.1f},{offsets.right:.1f},{offsets.bottom:.1f}), "
        f"align=({align.x:.2f},{align.y:.2f})"
    )


def require_canvas_slot(widget_bp, name: str):
    widget = require_widget(widget_bp, name)
    slot = widget.get_editor_property("slot")
    canvas_slot = unreal.CanvasPanelSlot.cast(slot)
    if not canvas_slot:
        raise RuntimeError(f"{name} does not have a CanvasPanelSlot")
    log(f"{name}: {slot_summary(widget)}")
    return canvas_slot


def require_anchor(widget_bp, name: str, expected_min_x: float, expected_min_y: float, expected_max_x: float, expected_max_y: float):
    slot = require_canvas_slot(widget_bp, name)
    anchors = slot.get_anchors()
    actual = (round(anchors.minimum.x, 2), round(anchors.minimum.y, 2), round(anchors.maximum.x, 2), round(anchors.maximum.y, 2))
    expected = (expected_min_x, expected_min_y, expected_max_x, expected_max_y)
    if actual != expected:
        raise RuntimeError(f"{name} anchor mismatch: expected {expected}, got {actual}")


def main():
    widget_bp = cast_widget_bp(LOGIN_ASSET)
    log(f"verifying: {LOGIN_ASSET}")

    for name, class_name in [
        ("RootCanvas_Auto", "CanvasPanel"),
        ("ForestBackgroundImage", "Image"),
        ("TitleText", "TextBlock"),
        ("LoginPanel", "Border"),
        ("ServerButton", "Button"),
        ("EmailInputHost", "CanvasPanel"),
        ("PasswordInputHost", "CanvasPanel"),
        ("RememberText", "TextBlock"),
        ("ForgotPasswordText", "TextBlock"),
        ("LoginButton", "Button"),
        ("GuestLoginButton", "Button"),
        ("RegisterAccountText", "TextBlock"),
        ("NoticeToolButton", "Button"),
        ("SupportToolButton", "Button"),
        ("RepairToolButton", "Button"),
        ("AgeRatingText", "TextBlock"),
        ("AgreementText", "TextBlock"),
    ]:
        require_widget(widget_bp, name, class_name)

    require_text(widget_bp, "TitleText", "神兽竞技场")
    require_text(widget_bp, "ServerLabelText", "服务器：")
    require_text(widget_bp, "ServerNameText", "苍穹之森")
    require_text(widget_bp, "ServerStatusText", "● 流畅")
    require_text(widget_bp, "AccountLabelText", "账号")
    require_text(widget_bp, "PasswordLabelText", "密码")
    require_text(widget_bp, "RememberText", "□ 记住我")
    require_text(widget_bp, "ForgotPasswordText", "忘记密码")
    require_text(widget_bp, "LoginButtonLabel", "登录")
    require_text(widget_bp, "GuestLoginButtonLabel", "游客登录")
    require_text(widget_bp, "RegisterAccountText", "注册账号")
    require_text(widget_bp, "NoticeToolLabel", "公告")
    require_text(widget_bp, "SupportToolLabel", "客服")
    require_text(widget_bp, "RepairToolLabel", "修复")
    require_text(widget_bp, "AgeRatingText", "16+\nCADPA\n适龄提示")
    require_text(widget_bp, "AgreementText", "☑ 我已详细阅读并同意《用户协议》和《隐私政策》")

    require_anchor(widget_bp, "ForestBackgroundImage", 0.0, 0.0, 1.0, 1.0)
    require_anchor(widget_bp, "LoginPanel", 0.5, 0.32, 0.5, 0.32)
    require_anchor(widget_bp, "RightToolPanel", 0.94, 0.42, 0.94, 0.42)
    require_anchor(widget_bp, "AgeRatingPanel", 0.03, 0.82, 0.03, 0.82)
    require_anchor(widget_bp, "AgreementText", 0.5, 0.91, 0.5, 0.91)

    log("verification passed")


main()
