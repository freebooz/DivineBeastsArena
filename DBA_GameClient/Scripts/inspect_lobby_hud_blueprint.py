# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

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
