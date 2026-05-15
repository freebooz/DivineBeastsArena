import unreal


TARGET_ASSET = "/Game/DBA/UI/Frontend/Character/WBP_DBA_CharacterCreate"

EXPECTED_TEXTS = {
    "ZodiacText": "生肖：鼠",
    "ElementText": "元素：水",
    "FiveCampText": "阵营：无",
    "CreateButtonLabel": "创建并进入",
    "BackButtonLabel": "返回角色选择",
}

EXPECTED_HINTS = {
    "CharacterNameInput": "请输入角色名称",
}


def cast_widget_bp(asset_path: str):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        raise RuntimeError(f"load failed: {asset_path}")
    cast_result = unreal.EditorUtilityLibrary.cast_to_widget_blueprint(asset)
    widget_bp = cast_result[1] if isinstance(cast_result, tuple) else cast_result
    if not widget_bp:
        raise RuntimeError(f"cast widget blueprint failed: {asset_path}")
    return widget_bp


def find_widget(widget_bp, widget_name: str):
    return unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_bp, unreal.Name(widget_name))


def main():
    wb = cast_widget_bp(TARGET_ASSET)
    for name, text in EXPECTED_TEXTS.items():
        widget = find_widget(wb, name)
        if not widget:
            raise RuntimeError(f"missing text widget: {name}")
        tb = unreal.TextBlock.cast(widget)
        if not tb:
            raise RuntimeError(f"not textblock: {name}")
        current = str(tb.get_text())
        unreal.log(f"[VerifyCharacterCreateZH] {name}: {current}")
        if current != text:
            raise RuntimeError(f"text mismatch {name}: {current} != {text}")

    for name, hint in EXPECTED_HINTS.items():
        widget = find_widget(wb, name)
        if not widget:
            raise RuntimeError(f"missing input widget: {name}")
        etb = unreal.EditableTextBox.cast(widget)
        if not etb:
            raise RuntimeError(f"not editable text box: {name}")
        current_hint = str(etb.get_editor_property("hint_text"))
        unreal.log(f"[VerifyCharacterCreateZH] {name}.Hint: {current_hint}")
        if current_hint != hint:
            raise RuntimeError(f"hint mismatch {name}: {current_hint} != {hint}")

    unreal.log("[VerifyCharacterCreateZH] verification passed")


main()
