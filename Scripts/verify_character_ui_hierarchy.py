import unreal


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


def slot_summary(widget):
    if not widget:
        return "missing"
    slot = widget.get_editor_property("slot")
    if slot is None:
        return "root-slot-none"
    try:
        canvas_slot = unreal.CanvasPanelSlot.cast(slot)
    except TypeError:
        return f"slot={slot.get_class().get_name()}"
    if not canvas_slot:
        return "no-canvas-slot"

    anchors = canvas_slot.get_anchors()
    offsets = canvas_slot.get_offsets()
    align = canvas_slot.get_alignment()
    return (
        f"anchors=({anchors.minimum.x:.2f},{anchors.minimum.y:.2f})-({anchors.maximum.x:.2f},{anchors.maximum.y:.2f}), "
        f"offsets=({offsets.left:.1f},{offsets.top:.1f},{offsets.right:.1f},{offsets.bottom:.1f}), "
        f"align=({align.x:.2f},{align.y:.2f})"
    )


def verify_one(asset_path: str, expected_names):
    wb = cast_widget_bp(asset_path)
    unreal.log(f"[VerifyCharacterUI] ---- {asset_path} ----")
    missing = []
    for name in expected_names:
        widget = find_widget(wb, name)
        if not widget:
            missing.append(name)
        unreal.log(f"[VerifyCharacterUI] {name}: {slot_summary(widget)}")
    if missing:
        raise RuntimeError(f"missing widgets in {asset_path}: {missing}")


verify_one(
    "/Game/DBA/UI/Frontend/Character/WBP_DBA_CharacterSelect",
    [
        "RootCanvas_Auto",
        "CharacterPreviewHost",
        "CharacterListText",
        "RefreshButton",
        "ConfirmButton",
        "CreateButton",
        "StatusText",
    ],
)

verify_one(
    "/Game/DBA/UI/Frontend/Character/WBP_DBA_CharacterCreate",
    [
        "RootCanvas_Auto",
        "CharacterPreviewHost",
        "CharacterNameInput",
        "ZodiacButton",
        "ElementButton",
        "FiveCampButton",
        "CreateButton",
        "BackButton",
        "ZodiacText",
        "ElementText",
        "FiveCampText",
        "ValidationText",
    ],
)

unreal.log("[VerifyCharacterUI] verification passed")
