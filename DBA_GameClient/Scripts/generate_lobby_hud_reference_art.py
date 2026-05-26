from __future__ import annotations

import math
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter


ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "SourceAssets" / "UI" / "LobbyHUD"


GOLD = (214, 164, 74, 255)
GOLD_LIGHT = (255, 220, 124, 255)
GOLD_DARK = (92, 58, 24, 255)
INK = (7, 8, 9, 232)
PANEL = (18, 20, 22, 214)


def ensure_out() -> None:
    OUT.mkdir(parents=True, exist_ok=True)


def glow_blur(size: tuple[int, int], draw_fn, radius: float = 7.0) -> Image.Image:
    glow = Image.new("RGBA", size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(glow, "RGBA")
    draw_fn(draw, 0)
    glow = glow.filter(ImageFilter.GaussianBlur(radius))
    crisp = Image.new("RGBA", size, (0, 0, 0, 0))
    draw_fn(ImageDraw.Draw(crisp, "RGBA"), 1)
    return Image.alpha_composite(glow, crisp)


def draw_beveled_rect(draw: ImageDraw.ImageDraw, box: tuple[int, int, int, int], radius: int, fill, outline=GOLD, width: int = 3) -> None:
    x0, y0, x1, y1 = box
    draw.rounded_rectangle(box, radius=radius, fill=fill, outline=GOLD_DARK, width=width + 2)
    draw.rounded_rectangle((x0 + 2, y0 + 2, x1 - 2, y1 - 2), radius=max(0, radius - 2), outline=outline, width=width)
    draw.line((x0 + radius, y0 + 4, x1 - radius, y0 + 4), fill=GOLD_LIGHT, width=1)
    draw.line((x0 + radius, y1 - 5, x1 - radius, y1 - 5), fill=(70, 42, 18, 220), width=2)


def draw_corner_vines(draw: ImageDraw.ImageDraw, box: tuple[int, int, int, int], inset: int = 12) -> None:
    x0, y0, x1, y1 = box
    corners = [
        ((x0 + inset, y0 + inset), 1, 1),
        ((x1 - inset, y0 + inset), -1, 1),
        ((x0 + inset, y1 - inset), 1, -1),
        ((x1 - inset, y1 - inset), -1, -1),
    ]
    for (cx, cy), sx, sy in corners:
        draw.line((cx, cy, cx + sx * 54, cy), fill=GOLD, width=3)
        draw.line((cx, cy, cx, cy + sy * 34), fill=GOLD, width=3)
        ax0 = cx - sx * 4
        ay0 = cy - sy * 4
        ax1 = cx + sx * 54
        ay1 = cy + sy * 54
        arc_box = (min(ax0, ax1), min(ay0, ay1), max(ax0, ax1), max(ay0, ay1))
        draw.arc(arc_box, 0 if sx < 0 else 180, 90 if sy > 0 else 270, fill=GOLD_LIGHT, width=2)


def unit_frame() -> None:
    size = (512, 192)

    def painter(draw: ImageDraw.ImageDraw, phase: int) -> None:
        alpha = 180 if phase == 0 else 255
        draw_beveled_rect(draw, (42, 28, 500, 150), 20, (*PANEL[:3], min(PANEL[3], alpha)), width=3)
        draw.ellipse((8, 8, 152, 152), fill=(10, 10, 12, 236), outline=GOLD_DARK, width=8)
        draw.ellipse((17, 17, 143, 143), outline=(255, 222, 128, 255), width=4)
        draw.ellipse((27, 27, 133, 133), outline=(48, 32, 18, 255), width=4)
        draw.ellipse((13, 117, 61, 165), fill=(16, 16, 18, 246), outline=GOLD, width=4)
        draw.line((158, 56, 466, 56), fill=GOLD_LIGHT, width=2)
        draw.rectangle((162, 74, 462, 100), fill=(34, 12, 12, 232), outline=GOLD_DARK, width=2)
        draw.rectangle((166, 78, 458, 96), fill=(32, 158, 52, 232))
        draw.rectangle((162, 106, 462, 132), fill=(8, 12, 35, 232), outline=GOLD_DARK, width=2)
        draw.rectangle((166, 110, 420, 128), fill=(38, 82, 210, 232))
        draw_corner_vines(draw, (42, 28, 500, 150), 10)

    glow_blur(size, painter, 5).save(OUT / "T_DBA_LobbyHUD_UnitFrame_512x192.png")


def portrait() -> None:
    img = Image.new("RGBA", (256, 256), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img, "RGBA")
    draw.ellipse((18, 18, 238, 238), fill=(22, 18, 30, 255), outline=GOLD_DARK, width=8)
    draw.ellipse((32, 32, 224, 224), fill=(43, 36, 58, 255))
    draw.ellipse((80, 48, 176, 148), fill=(205, 188, 174, 255))
    draw.polygon([(72, 70), (116, 34), (184, 72), (170, 116), (96, 116)], fill=(128, 112, 172, 255))
    draw.polygon([(68, 218), (96, 150), (160, 150), (190, 218)], fill=(60, 54, 93, 255))
    draw.line((96, 166, 160, 166), fill=(214, 164, 74, 220), width=4)
    draw.ellipse((92, 92, 104, 102), fill=(38, 26, 40, 255))
    draw.ellipse((152, 92, 164, 102), fill=(38, 26, 40, 255))
    draw.arc((106, 92, 150, 130), 24, 156, fill=(122, 70, 82, 255), width=3)
    draw.ellipse((18, 18, 238, 238), outline=GOLD_LIGHT, width=4)
    img.save(OUT / "T_DBA_LobbyHUD_PlayerPortrait_Default_256.png")


def portrait_frame() -> None:
    img = Image.new("RGBA", (256, 256), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img, "RGBA")
    for i, color in enumerate([(48, 28, 12, 255), GOLD, GOLD_LIGHT, GOLD_DARK]):
        inset = 6 + i * 7
        draw.ellipse((inset, inset, 256 - inset, 256 - inset), outline=color, width=7 if i < 2 else 3)
    for angle in range(0, 360, 45):
        rad = math.radians(angle)
        cx, cy = 128 + math.cos(rad) * 109, 128 + math.sin(rad) * 109
        pts = []
        for k in range(4):
            a = rad + math.pi / 4 * k
            r = 15 if k % 2 == 0 else 6
            pts.append((cx + math.cos(a) * r, cy + math.sin(a) * r))
        draw.polygon(pts, fill=GOLD, outline=GOLD_LIGHT)
    img.save(OUT / "T_DBA_LobbyHUD_PortraitFrame_256.png")


def skill_bar() -> None:
    size = (1024, 160)

    def painter(draw: ImageDraw.ImageDraw, phase: int) -> None:
        fill_alpha = 176 if phase == 0 else 232
        draw_beveled_rect(draw, (112, 38, 912, 134), 18, (8, 8, 10, fill_alpha), width=4)
        draw.rectangle((170, 126, 854, 140), fill=(60, 16, 100, 225), outline=(153, 74, 198, 255), width=2)
        draw.line((166, 145, 858, 145), fill=GOLD_DARK, width=3)
        draw.polygon([(92, 82), (134, 38), (168, 84), (132, 134)], fill=(14, 15, 18, 235), outline=GOLD, width=4)
        draw.polygon([(932, 82), (890, 38), (856, 84), (892, 134)], fill=(14, 15, 18, 235), outline=GOLD, width=4)
        draw.arc((70, 52, 176, 154), 210, 346, fill=GOLD_LIGHT, width=4)
        draw.arc((848, 52, 954, 154), 194, 330, fill=GOLD_LIGHT, width=4)
        for x in range(218, 806, 84):
            draw.rounded_rectangle((x, 54, x + 72, 126), radius=8, outline=(78, 47, 22, 180), width=2)

    glow_blur(size, painter, 6).save(OUT / "T_DBA_LobbyHUD_SkillBar_1024x160.png")


def skill_slot() -> None:
    img = Image.new("RGBA", (128, 128), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img, "RGBA")
    draw.rounded_rectangle((8, 8, 120, 120), radius=14, fill=(7, 8, 10, 245), outline=GOLD_DARK, width=8)
    draw.rounded_rectangle((18, 18, 110, 110), radius=9, outline=GOLD, width=4)
    draw.line((22, 20, 106, 20), fill=GOLD_LIGHT, width=2)
    draw.line((22, 108, 106, 108), fill=(48, 28, 13, 255), width=3)
    img.save(OUT / "T_DBA_LobbyHUD_SkillSlot_128.png")


def minimap() -> None:
    img = Image.new("RGBA", (512, 512), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img, "RGBA")
    draw.ellipse((22, 22, 490, 490), fill=(6, 8, 8, 236), outline=GOLD_DARK, width=12)
    draw.ellipse((42, 42, 470, 470), fill=(32, 48, 26, 245), outline=GOLD, width=6)
    terrain = Image.new("RGBA", (512, 512), (0, 0, 0, 0))
    td = ImageDraw.Draw(terrain, "RGBA")
    for i in range(36):
        x = 58 + (i * 79) % 388
        y = 58 + (i * 53) % 388
        r = 32 + (i % 4) * 18
        color = (72 + i % 5 * 10, 105 + i % 3 * 12, 42, 132)
        td.ellipse((x - r, y - r, x + r, y + r), fill=color)
    td.line((76, 356, 176, 284, 236, 280, 318, 206, 436, 184), fill=(148, 112, 58, 164), width=24)
    td.line((88, 360, 186, 292, 244, 290, 326, 216, 428, 194), fill=(198, 158, 82, 128), width=7)
    mask = Image.new("L", (512, 512), 0)
    ImageDraw.Draw(mask).ellipse((58, 58, 454, 454), fill=255)
    img = Image.alpha_composite(img, Image.composite(terrain, Image.new("RGBA", (512, 512), (0, 0, 0, 0)), mask))
    draw = ImageDraw.Draw(img, "RGBA")
    for angle, label in [(270, "N"), (0, "E"), (90, "S"), (180, "W")]:
        rad = math.radians(angle)
        x = 256 + math.cos(rad) * 205
        y = 256 + math.sin(rad) * 205
        draw.ellipse((x - 10, y - 10, x + 10, y + 10), fill=(20, 22, 26, 245), outline=GOLD, width=2)
    draw.ellipse((24, 24, 488, 488), outline=GOLD_LIGHT, width=3)
    draw.ellipse((62, 62, 450, 450), outline=(74, 48, 24, 230), width=4)
    draw.polygon([(256, 32), (270, 70), (256, 62), (242, 70)], fill=GOLD_LIGHT, outline=GOLD_DARK)
    img.save(OUT / "T_DBA_LobbyHUD_MinimapFrame_512.png")


def main() -> None:
    ensure_out()
    unit_frame()
    portrait()
    portrait_frame()
    skill_bar()
    skill_slot()
    minimap()
    for path in sorted(OUT.glob("T_DBA_LobbyHUD_*.png")):
        print(f"Generated {path}")


if __name__ == "__main__":
    main()
