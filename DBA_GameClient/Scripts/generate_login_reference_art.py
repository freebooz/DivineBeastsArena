# 中文阅读说明：
# - 所属应用：DBA_GameClient Unreal Engine 客户端。
# - 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
# - 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
# - 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。

from __future__ import annotations

import math
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter


ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "Exports" / "Art" / "UI" / "LoginTextures"


def lerp(a: int, b: int, t: float) -> int:
    return round(a + (b - a) * t)


def blend(c1: tuple[int, int, int], c2: tuple[int, int, int], t: float) -> tuple[int, int, int]:
    return tuple(lerp(a, b, t) for a, b in zip(c1, c2))


def draw_login_background() -> None:
    width, height = 1920, 1080
    img = Image.new("RGB", (width, height), "#02110d")
    px = img.load()

    top = (3, 30, 28)
    center = (25, 110, 92)
    bottom = (2, 25, 17)
    for y in range(height):
        vertical = y / (height - 1)
        base = blend(top, center, min(vertical * 1.6, 1.0)) if vertical < 0.62 else blend(center, bottom, (vertical - 0.62) / 0.38)
        for x in range(width):
            dx = (x - width * 0.45) / width
            glow = max(0.0, 1.0 - math.hypot(dx * 2.2, (vertical - 0.34) * 1.8))
            px[x, y] = blend(base, (120, 190, 130), glow * 0.42)

    overlay = Image.new("RGBA", img.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay, "RGBA")

    for i in range(16):
        x = 80 + i * 118
        trunk_w = 28 + (i % 5) * 7
        lean = (-80 + (i * 37) % 160)
        draw.line([(x, 0), (x + lean, 670)], fill=(5, 34, 24, 190), width=trunk_w)
        for branch in range(4):
            by = 120 + branch * 95 + (i % 3) * 22
            draw.line([(x + lean * by / 670, by), (x + lean * by / 670 + (-180 + branch * 112), by + 80)], fill=(4, 40, 26, 155), width=max(8, trunk_w // 3))

    draw.ellipse((360, 158, 760, 560), fill=(17, 70, 47, 168), outline=(155, 136, 68, 150), width=6)
    draw.ellipse((448, 220, 676, 464), fill=(7, 48, 34, 210), outline=(198, 178, 96, 130), width=5)
    draw.polygon([(540, 245), (610, 376), (480, 376)], fill=(108, 132, 74, 178), outline=(211, 190, 93, 130))
    draw.ellipse((520, 210, 605, 290), outline=(204, 190, 112, 160), width=5)

    for i in range(46):
        x = (i * 173) % width
        y = 120 + (i * 97) % 720
        r = 1 + (i % 4)
        color = (65, 225, 220, 110 + (i % 3) * 40)
        draw.ellipse((x - r, y - r, x + r, y + r), fill=color)

    for i in range(28):
        x = 18 + i * 72
        y = 850 + (i * 47) % 170
        draw.ellipse((x, y, x + 80, y + 36), fill=(20, 84, 39, 165))
        flower = (238, 214, 145, 210) if i % 3 else (222, 110, 68, 190)
        draw.ellipse((x + 30, y - 16, x + 45, y - 1), fill=flower)

    draw.rectangle((0, 720, width, height), fill=(0, 16, 9, 82))
    draw.polygon([(760, 1080), (920, 715), (1010, 715), (1165, 1080)], fill=(126, 104, 56, 92))
    draw.line([(920, 715), (1010, 715)], fill=(219, 190, 92, 138), width=3)

    overlay = overlay.filter(ImageFilter.GaussianBlur(0.35))
    img = Image.alpha_composite(img.convert("RGBA"), overlay)

    vignette = Image.new("L", img.size, 0)
    vpx = vignette.load()
    for y in range(height):
        for x in range(width):
            nx = (x / width - 0.52) * 2.0
            ny = (y / height - 0.46) * 2.0
            amount = min(190, round(max(0.0, math.hypot(nx, ny) - 0.35) * 150))
            vpx[x, y] = amount
    dark = Image.new("RGBA", img.size, (0, 0, 0, 255))
    img = Image.composite(dark, img, vignette).convert("RGB")

    OUT.mkdir(parents=True, exist_ok=True)
    img.save(OUT / "T_DBA_LoginForestSanctuary.png")


if __name__ == "__main__":
    draw_login_background()
    print(f"Generated {OUT / 'T_DBA_LoginForestSanctuary.png'}")
