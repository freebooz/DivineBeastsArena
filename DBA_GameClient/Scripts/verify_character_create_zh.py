# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

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
