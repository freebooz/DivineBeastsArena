# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：生成大厅关卡玩家 HUD 用户蓝图，布局参考传统 MMORPG 战斗 HUD 的头像、技能栏、小地图分区。
# - 修改提示：组件名与 C++ UDBALobbyPlayerHUDWidgetBase 绑定逻辑一致，改名会影响运行时刷新。

import unreal


ASSET_DIR = "/Game/DBA/UI/Lobby/HUD"
ASSET_NAME = "WBP_DBA_LobbyPlayerHUD"
ASSET_PATH = f"{ASSET_DIR}/{ASSET_NAME}"
PARENT_CLASS = "DBALobbyPlayerHUDWidgetBase"

TEX_DIR = "/Game/DBA/UI/Lobby/HUD/Textures"
UNIT_FRAME = f"{TEX_DIR}/T_DBA_LobbyHUD_UnitFrame_512x192.T_DBA_LobbyHUD_UnitFrame_512x192"
PORTRAIT = f"{TEX_DIR}/T_DBA_LobbyHUD_PlayerPortrait_Default_256.T_DBA_LobbyHUD_PlayerPortrait_Default_256"
PORTRAIT_FRAME = f"{TEX_DIR}/T_DBA_LobbyHUD_PortraitFrame_256.T_DBA_LobbyHUD_PortraitFrame_256"
SKILL_BAR = f"{TEX_DIR}/T_DBA_LobbyHUD_SkillBar_1024x160.T_DBA_LobbyHUD_SkillBar_1024x160"
SKILL_SLOT = f"{TEX_DIR}/T_DBA_LobbyHUD_SkillSlot_128.T_DBA_LobbyHUD_SkillSlot_128"
MINIMAP = f"{TEX_DIR}/T_DBA_LobbyHUD_MinimapFrame_512.T_DBA_LobbyHUD_MinimapFrame_512"


def log(message):
    unreal.log(f"[CreateLobbyHUDWidgetBlueprint] {message}")


def fail(message):
    unreal.log_error(f"[CreateLobbyHUDWidgetBlueprint] {message}")
    raise RuntimeError(message)


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def rebuild_widget_blueprint():
    ensure_dir(ASSET_DIR)
    backup_path = f"{ASSET_PATH}__AutoBackup"
    if unreal.EditorAssetLibrary.does_asset_exist(ASSET_PATH):
        if not unreal.EditorAssetLibrary.does_asset_exist(backup_path):
            unreal.EditorAssetLibrary.duplicate_asset(ASSET_PATH, backup_path)
            log(f"Created backup: {backup_path}")
        unreal.EditorAssetLibrary.delete_asset(ASSET_PATH)

    parent_class = getattr(unreal, PARENT_CLASS, None)
    if not parent_class:
        fail(f"Parent class not found: {PARENT_CLASS}")

    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        ASSET_NAME,
        ASSET_DIR,
        unreal.WidgetBlueprint,
        factory,
    )
    if not asset:
        fail(f"Failed to create widget blueprint: {ASSET_PATH}")

    cast_result = unreal.EditorUtilityLibrary.cast_to_widget_blueprint(asset)
    widget_bp = cast_result[1] if isinstance(cast_result, tuple) else cast_result
    if not widget_bp:
        fail(f"Failed to cast {ASSET_PATH} to WidgetBlueprint")
    return asset, widget_bp


def add(widget_bp, widget_class, name, parent=""):
    widget = unreal.EditorUtilityLibrary.add_source_widget(
        widget_bp,
        widget_class,
        name,
        unreal.Name(parent) if parent else unreal.Name(""),
    )
    if not widget:
        fail(f"Failed to add widget {name}")
    return widget


def mark_var(widget):
    for prop_name in ("bIsVariable", "b_is_variable"):
        try:
            widget.set_editor_property(prop_name, True)
            return
        except Exception:
            pass


def canvas_slot(widget, min_x, min_y, max_x, max_y, left, top, width, height, align_x=0.0, align_y=0.0, z=0):
    slot = unreal.CanvasPanelSlot.cast(widget.get_editor_property("slot"))
    if not slot:
        fail(f"{widget.get_name()} is not in a CanvasPanelSlot")
    slot.set_anchors(unreal.Anchors(minimum=unreal.Vector2D(min_x, min_y), maximum=unreal.Vector2D(max_x, max_y)))
    slot.set_offsets(unreal.Margin(left, top, width, height))
    slot.set_alignment(unreal.Vector2D(align_x, align_y))
    try:
        slot.set_z_order(z)
    except Exception:
        pass


def fill_canvas(widget, z=0, inset=0.0):
    canvas_slot(widget, 0.0, 0.0, 1.0, 1.0, inset, inset, -inset * 2.0, -inset * 2.0, 0.0, 0.0, z)


def set_image_texture(widget, texture_path, tint=None):
    texture = unreal.EditorAssetLibrary.load_asset(texture_path)
    if texture:
        widget.set_brush_from_texture(texture)
    else:
        log(f"Texture not loaded: {texture_path}")
    if tint:
        widget.set_color_and_opacity(tint)


def set_border_texture(widget, texture_path, tint=None):
    texture = unreal.EditorAssetLibrary.load_asset(texture_path)
    if texture:
        widget.set_brush_from_texture(texture)
    if tint:
        widget.set_brush_color(tint)


def set_font_size(text_block, size):
    try:
        font = text_block.get_font()
        font.size = size
        text_block.set_font(font)
    except Exception:
        pass


def text(widget_bp, name, parent, value, size, color, justify=unreal.TextJustify.CENTER):
    block = add(widget_bp, unreal.TextBlock, name, parent)
    mark_var(block)
    block.set_text(unreal.Text(value))
    block.set_color_and_opacity(unreal.SlateColor(color))
    try:
        block.set_justification(justify)
    except Exception:
        try:
            block.set_editor_property("justification", justify)
        except Exception:
            pass
    block.set_auto_wrap_text(False)
    set_font_size(block, size)
    return block


def image(widget_bp, name, parent, texture_path=None, tint=None):
    img = add(widget_bp, unreal.Image, name, parent)
    mark_var(img)
    if texture_path:
        set_image_texture(img, texture_path, tint)
    elif tint:
        img.set_color_and_opacity(tint)
    return img


def build_avatar(widget_bp):
    root = add(widget_bp, unreal.Border, "LobbyHUD_AvatarRoot", "LobbyHUDCanvas")
    mark_var(root)
    root.set_padding(unreal.Margin(0.0, 0.0, 0.0, 0.0))
    root.set_brush_color(unreal.LinearColor(0.0, 0.0, 0.0, 0.0))
    canvas_slot(root, 0.0, 0.0, 0.0, 0.0, 18.0, 18.0, 320.0, 120.0, 0.0, 0.0, 40)

    canvas = add(widget_bp, unreal.CanvasPanel, "LobbyHUD_AvatarCanvas", "LobbyHUD_AvatarRoot")

    back = image(widget_bp, "LobbyHUD_AvatarBackdrop", "LobbyHUD_AvatarCanvas", UNIT_FRAME)
    fill_canvas(back, 0)

    portrait = image(widget_bp, "LobbyHUD_AvatarImage", "LobbyHUD_AvatarCanvas", PORTRAIT)
    canvas_slot(portrait, 0.0, 0.0, 0.0, 0.0, 20.0, 18.0, 82.0, 82.0, 0.0, 0.0, 2)

    frame = image(widget_bp, "LobbyHUD_AvatarFrame", "LobbyHUD_AvatarCanvas", PORTRAIT_FRAME)
    canvas_slot(frame, 0.0, 0.0, 0.0, 0.0, 8.0, 6.0, 106.0, 106.0, 0.0, 0.0, 3)

    level = text(widget_bp, "LobbyHUD_AvatarLevelText", "LobbyHUD_AvatarCanvas", "25", 16, unreal.LinearColor(1.0, 0.82, 0.22, 1.0))
    canvas_slot(level, 0.0, 0.0, 0.0, 0.0, 17.0, 85.0, 42.0, 28.0, 0.0, 0.0, 5)

    name = text(widget_bp, "LobbyHUD_AvatarNameText", "LobbyHUD_AvatarCanvas", "艾琳娜", 15, unreal.LinearColor(1.0, 0.84, 0.28, 1.0))
    canvas_slot(name, 0.0, 0.0, 0.0, 0.0, 118.0, 24.0, 168.0, 24.0, 0.0, 0.0, 5)

    health_bg = image(widget_bp, "LobbyHUD_AvatarHealthBg", "LobbyHUD_AvatarCanvas", None, unreal.LinearColor(0.12, 0.02, 0.02, 0.92))
    canvas_slot(health_bg, 0.0, 0.0, 0.0, 0.0, 120.0, 55.0, 180.0, 18.0, 0.0, 0.0, 3)
    health = image(widget_bp, "LobbyHUD_AvatarHealthFill", "LobbyHUD_AvatarCanvas", None, unreal.LinearColor(0.08, 0.78, 0.18, 0.95))
    canvas_slot(health, 0.0, 0.0, 0.0, 0.0, 123.0, 58.0, 174.0, 12.0, 0.0, 0.0, 4)
    health_text = text(widget_bp, "LobbyHUD_AvatarHealthText", "LobbyHUD_AvatarCanvas", "1,285 / 1,285 (100%)", 11, unreal.LinearColor(0.92, 1.0, 0.88, 1.0))
    canvas_slot(health_text, 0.0, 0.0, 0.0, 0.0, 120.0, 54.0, 180.0, 18.0, 0.0, 0.0, 6)

    mana_bg = image(widget_bp, "LobbyHUD_AvatarManaBg", "LobbyHUD_AvatarCanvas", None, unreal.LinearColor(0.02, 0.03, 0.12, 0.92))
    canvas_slot(mana_bg, 0.0, 0.0, 0.0, 0.0, 120.0, 76.0, 180.0, 18.0, 0.0, 0.0, 3)
    mana = image(widget_bp, "LobbyHUD_AvatarManaFill", "LobbyHUD_AvatarCanvas", None, unreal.LinearColor(0.18, 0.32, 0.96, 0.95))
    canvas_slot(mana, 0.0, 0.0, 0.0, 0.0, 123.0, 79.0, 153.0, 12.0, 0.0, 0.0, 4)
    mana_text = text(widget_bp, "LobbyHUD_AvatarManaText", "LobbyHUD_AvatarCanvas", "2,146 / 2,520 (85%)", 11, unreal.LinearColor(0.88, 0.92, 1.0, 1.0))
    canvas_slot(mana_text, 0.0, 0.0, 0.0, 0.0, 120.0, 75.0, 180.0, 18.0, 0.0, 0.0, 6)

    meta = text(widget_bp, "LobbyHUD_AvatarMetaText", "LobbyHUD_AvatarCanvas", "Rat | Fire | Lv.25", 10, unreal.LinearColor(0.82, 0.84, 0.9, 0.0))
    canvas_slot(meta, 0.0, 0.0, 0.0, 0.0, 118.0, 96.0, 168.0, 16.0, 0.0, 0.0, 1)


def build_skill_bar(widget_bp):
    root = add(widget_bp, unreal.Border, "LobbyHUD_SkillBarRoot", "LobbyHUDCanvas")
    mark_var(root)
    root.set_padding(unreal.Margin(0.0, 0.0, 0.0, 0.0))
    root.set_brush_color(unreal.LinearColor(0.0, 0.0, 0.0, 0.0))
    canvas_slot(root, 0.5, 1.0, 0.5, 1.0, 0.0, -18.0, 760.0, 118.0, 0.5, 1.0, 45)

    canvas = add(widget_bp, unreal.CanvasPanel, "LobbyHUD_SkillBarCanvas", "LobbyHUD_SkillBarRoot")
    back = image(widget_bp, "LobbyHUD_SkillBarBackdrop", "LobbyHUD_SkillBarCanvas", SKILL_BAR)
    fill_canvas(back, 0)

    start_x = 146.0
    slot_size = 58.0
    gap = 9.0
    hotkeys = ["A", "1", "2", "3", "4", "R", "F"]
    colors = [
        unreal.LinearColor(0.42, 0.28, 0.13, 0.92),
        unreal.LinearColor(0.95, 0.18, 0.04, 0.94),
        unreal.LinearColor(0.12, 0.38, 0.96, 0.94),
        unreal.LinearColor(0.10, 0.78, 0.42, 0.94),
        unreal.LinearColor(0.90, 0.54, 0.08, 0.94),
        unreal.LinearColor(0.58, 0.14, 0.95, 0.96),
        unreal.LinearColor(0.12, 0.78, 0.92, 0.94),
    ]

    for index in range(7):
        button = add(widget_bp, unreal.Button, f"LobbyHUD_SkillButton_{index}", "LobbyHUD_SkillBarCanvas")
        mark_var(button)
        canvas_slot(button, 0.0, 0.0, 0.0, 0.0, start_x + index * (slot_size + gap), 38.0, slot_size, slot_size, 0.0, 0.0, 4)

        overlay = add(widget_bp, unreal.Overlay, f"LobbyHUD_SkillOverlay_{index}", button.get_name())
        back_slot = image(widget_bp, f"LobbyHUD_SkillBack_{index}", overlay.get_name(), SKILL_SLOT, colors[index])
        glow = image(widget_bp, f"LobbyHUD_SkillGlow_{index}", overlay.get_name(), None, unreal.LinearColor(1.0, 0.72, 0.2, 0.14))
        cooldown = image(widget_bp, f"LobbyHUD_SkillCooldownOverlay_{index}", overlay.get_name(), None, unreal.LinearColor(0.0, 0.0, 0.0, 0.68))
        cooldown.set_visibility(unreal.SlateVisibility.HIDDEN)
        cd_text = text(widget_bp, f"LobbyHUD_SkillCD_{index}", overlay.get_name(), "", 15, unreal.LinearColor(1.0, 0.76, 0.28, 1.0))
        key = text(widget_bp, f"LobbyHUD_SkillHotkey_{index}", overlay.get_name(), hotkeys[index], 10, unreal.LinearColor(1.0, 0.84, 0.18, 1.0))
        name = text(widget_bp, f"LobbyHUD_SkillName_{index}", overlay.get_name(), f"Skill {index}", 10, unreal.LinearColor(0.95, 0.95, 0.98, 1.0))
        name.set_visibility(unreal.SlateVisibility.COLLAPSED)

        for candidate in (back_slot, glow, cooldown, cd_text, key):
            mark_var(candidate)

    clock = text(widget_bp, "LobbyHUD_SkillBarClockText", "LobbyHUD_SkillBarCanvas", "4:07 PM", 14, unreal.LinearColor(1.0, 0.76, 0.18, 1.0))
    canvas_slot(clock, 0.5, 1.0, 0.5, 1.0, 0.0, -20.0, 110.0, 20.0, 0.5, 1.0, 6)


def build_minimap(widget_bp):
    root = add(widget_bp, unreal.Border, "LobbyHUD_MinimapRoot", "LobbyHUDCanvas")
    mark_var(root)
    root.set_padding(unreal.Margin(0.0, 0.0, 0.0, 0.0))
    root.set_brush_color(unreal.LinearColor(0.0, 0.0, 0.0, 0.0))
    canvas_slot(root, 1.0, 0.0, 1.0, 0.0, -18.0, 18.0, 260.0, 260.0, 1.0, 0.0, 42)

    canvas = add(widget_bp, unreal.CanvasPanel, "LobbyHUD_MinimapCanvas", "LobbyHUD_MinimapRoot")
    back = image(widget_bp, "LobbyHUD_MinimapBackImage", "LobbyHUD_MinimapCanvas", MINIMAP)
    fill_canvas(back, 0)

    dot_canvas = add(widget_bp, unreal.CanvasPanel, "LobbyHUD_MinimapDotCanvas", "LobbyHUD_MinimapCanvas")
    mark_var(dot_canvas)
    canvas_slot(dot_canvas, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2)

    title = text(widget_bp, "LobbyHUD_MinimapZoneText", "LobbyHUD_MinimapCanvas", "(43, 51) 晨曦林地", 13, unreal.LinearColor(1.0, 0.82, 0.18, 1.0))
    canvas_slot(title, 0.5, 0.0, 0.5, 0.0, 0.0, 6.0, 180.0, 24.0, 0.5, 0.0, 4)


def main():
    asset, widget_bp = rebuild_widget_blueprint()
    add(widget_bp, unreal.SafeZone, "LobbyHUDSafeZone")
    add(widget_bp, unreal.CanvasPanel, "LobbyHUDCanvas", "LobbyHUDSafeZone")

    build_avatar(widget_bp)
    build_minimap(widget_bp)
    build_skill_bar(widget_bp)

    unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)
    log(f"Saved asset: {ASSET_PATH}")


main()
