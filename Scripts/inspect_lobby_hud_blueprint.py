import unreal


ASSET_PATH = "/Game/DBA/UI/Lobby/HUD/WBP_DBA_LobbyPlayerHUD.WBP_DBA_LobbyPlayerHUD"


asset = unreal.EditorAssetLibrary.load_asset(ASSET_PATH)
if not asset:
    unreal.log_error(f"missing asset: {ASSET_PATH}")
    raise SystemExit(1)

generated_class = unreal.EditorAssetLibrary.load_blueprint_class(ASSET_PATH)
cdo = unreal.get_default_object(generated_class) if generated_class else None
unreal.log(f"HUD blueprint: {ASSET_PATH}")
unreal.log(f"generated class: {generated_class.get_name() if generated_class else '<none>'}")
if cdo:
    for prop_name in ("avatar_panel_size", "skill_slot_size", "minimap_size"):
        try:
            unreal.log(f"CDO {prop_name}: {cdo.get_editor_property(prop_name)}")
        except Exception as exc:
            unreal.log_warning(f"CDO {prop_name}: unavailable ({exc})")

widget_tree = asset.get_editor_property("widget_tree")
root = widget_tree.get_editor_property("root_widget") if widget_tree else None
unreal.log(f"root widget: {root.get_name() if root else '<none>'}")

if widget_tree:
    widgets = widget_tree.get_all_widgets()
    for widget in widgets:
        name = widget.get_name()
        if "Avatar" not in name and "Skill" not in name and "LobbyHUD" not in name:
            continue
        slot = widget.get_editor_property("slot") if hasattr(widget, "get_editor_property") else None
        size_info = []
        for prop_name in ("width_override", "height_override", "desired_size_override"):
            try:
                value = widget.get_editor_property(prop_name)
                size_info.append(f"{prop_name}={value}")
            except Exception:
                pass
        if slot:
            for prop_name in ("size", "padding", "position", "anchors", "alignment"):
                try:
                    value = slot.get_editor_property(prop_name)
                    size_info.append(f"slot.{prop_name}={value}")
                except Exception:
                    pass
        unreal.log(f"widget {name} ({widget.get_class().get_name()}): " + "; ".join(size_info))
