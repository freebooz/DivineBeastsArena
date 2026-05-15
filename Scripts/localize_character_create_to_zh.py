import unreal


TARGET_ASSETS = [
    "/Game/DBA/UI/Frontend/Character/WBP_DBA_CharacterCreate",
    "/Game/DBA/UI/Lobby/Character/WBP_DBA_CharacterCreate",
]


def log(msg: str):
    unreal.log(f"[LocalizeCharacterCreateZH] {msg}")


def cast_widget_bp(asset_path: str):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        raise RuntimeError(f"load failed: {asset_path}")
    cast_result = unreal.EditorUtilityLibrary.cast_to_widget_blueprint(asset)
    widget_bp = cast_result[1] if isinstance(cast_result, tuple) else cast_result
    if not widget_bp:
        raise RuntimeError(f"cast widget blueprint failed: {asset_path}")
    return asset, widget_bp


def find_widget(widget_bp, widget_name: str):
    return unreal.EditorUtilityLibrary.find_source_widget_by_name(widget_bp, unreal.Name(widget_name))


def set_textblock(widget_bp, name: str, zh_text: str):
    widget = find_widget(widget_bp, name)
    if not widget:
        log(f"skip missing text block: {name}")
        return
    tb = unreal.TextBlock.cast(widget)
    if not tb:
        log(f"skip non-textblock: {name}")
        return
    tb.set_text(unreal.Text(zh_text))
    log(f"{name} -> {zh_text}")


def set_input_hint(widget_bp, name: str, zh_text: str):
    widget = find_widget(widget_bp, name)
    if not widget:
        log(f"skip missing input: {name}")
        return
    etb = unreal.EditableTextBox.cast(widget)
    if not etb:
        log(f"skip non-editabletext: {name}")
        return
    etb.set_hint_text(unreal.Text(zh_text))
    log(f"{name}.Hint -> {zh_text}")


def set_generic_text_if_english(widget_bp, name: str, zh_text: str):
    widget = find_widget(widget_bp, name)
    if not widget:
        return
    tb = unreal.TextBlock.cast(widget)
    if not tb:
        return
    current = str(tb.get_text())
    if any(("A" <= ch <= "Z") or ("a" <= ch <= "z") for ch in current):
        tb.set_text(unreal.Text(zh_text))
        log(f"{name}: '{current}' -> '{zh_text}'")


def localize_one(asset_path: str):
    asset, widget_bp = cast_widget_bp(asset_path)
    log(f"processing: {asset_path}")

    # Main create UI texts
    set_input_hint(widget_bp, "CharacterNameInput", "请输入角色名称")
    set_textblock(widget_bp, "ZodiacText", "生肖：鼠")
    set_textblock(widget_bp, "ElementText", "元素：水")
    set_textblock(widget_bp, "FiveCampText", "阵营：无")

    # Button labels created by automation script
    set_textblock(widget_bp, "CreateButtonLabel", "创建并进入")
    set_textblock(widget_bp, "BackButtonLabel", "返回角色选择")

    # Fallback: if these still carry English in some variant layout
    set_generic_text_if_english(widget_bp, "ValidationText", "")
    set_generic_text_if_english(widget_bp, "NativeCreateTitle", "创建角色")

    saved = unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)
    if not saved:
        raise RuntimeError(f"save failed: {asset_path}")
    log(f"saved: {asset_path}")


def main():
    for path in TARGET_ASSETS:
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            localize_one(path)
        else:
            log(f"skip missing asset: {path}")
    log("done")


main()
