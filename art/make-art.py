#!/usr/bin/env python3
"""Draws FUTBOL13's Home-screen art (our own, nothing from the game).

  art/make-art.py [OUT_DIR]   (default: art/)

Writes the names the app build looks up for art name "futbol13":
  futbol13.png        512x512 icon (icon0.png; also the launcher's game picture)
  futbol13-bg.png     3840x2160 RGB background (pic0/pic1; build_game_app.sh needs this exact size)
  futbol13-logo.png   3840x2160 RGBA title logo layer (pic2; transparent but for the logo)

A white football over a dark pitch, drawn from shapes alone: no text, no crests,
no imagery from any game. No randomness, so two runs write identical bytes.
"""
import math, pathlib, sys
from PIL import Image, ImageChops, ImageDraw, ImageFilter

LINE = (255, 255, 255)
BLACK = (14, 14, 16)


def gradient(w, h):
    """Pitch green at the top fading to near black, built from one 1xH ramp (fast, deterministic)."""
    ramp = Image.linear_gradient("L").resize((1, h), Image.BILINEAR)  # 0 at top .. 255 at bottom
    top, bottom = (6, 36, 15), (1, 12, 5)
    chans = [ramp.point(lambda v, a=a, b=b: round(a + (b - a) * v / 255)) for a, b in zip(top, bottom)]
    return Image.merge("RGB", chans).resize((w, h), Image.NEAREST)


def pitch(img, bands=12):
    """Mowing stripes, then the pitch's white lines with a soft glow under them."""
    w, h = img.size
    out = img.convert("RGBA")
    stripes = Image.new("RGBA", img.size, (0, 0, 0, 0))
    sd = ImageDraw.Draw(stripes)
    for i in range(1, bands, 2):
        sd.rectangle([round(i * w / bands), 0, round((i + 1) * w / bands) - 1, h], fill=(255, 255, 255, 7))
    out.alpha_composite(stripes)

    lw = max(2, int(6 * w / 3840))
    mx, my = w * 0.06, h * 0.06
    cx, cy, r = w / 2, h / 2, h * 0.09
    spot = max(lw, r * 0.06)
    lines = Image.new("RGBA", img.size, (0, 0, 0, 0))
    ld = ImageDraw.Draw(lines)
    fill = LINE + (140,)
    ld.rectangle([mx, my, w - mx, h - my], outline=fill, width=lw)
    ld.line([(cx, my), (cx, h - my)], fill=fill, width=lw)
    ld.ellipse([cx - r, cy - r, cx + r, cy + r], outline=fill, width=lw)
    ld.ellipse([cx - spot, cy - spot, cx + spot, cy + spot], fill=fill)
    out.alpha_composite(lines.filter(ImageFilter.GaussianBlur(lw * 2)))
    out.alpha_composite(lines)
    return out


def pentagon(cx, cy, r, angle):
    """A regular pentagon's corners, the first one at `angle` (radians) from the centre."""
    return [(cx + r * math.cos(angle + k * 2 * math.pi / 5), cy + r * math.sin(angle + k * 2 * math.pi / 5))
            for k in range(5)]


def ball(img, scale=1.0, centre=None):
    """A white football: a centre pentagon, five more around it clipped by the disc, a white glow."""
    img = img.convert("RGBA")
    w, h = img.size
    cx, cy = centre if centre else (w / 2, h / 2)
    s = min(w, h) * 0.3 * scale
    box = [cx - s, cy - s, cx + s, cy + s]

    glow = Image.new("RGBA", img.size, (0, 0, 0, 0))
    g = s * 1.06
    ImageDraw.Draw(glow).ellipse([cx - g, cy - g, cx + g, cy + g], fill=LINE + (128,))
    img.alpha_composite(glow.filter(ImageFilter.GaussianBlur(s * 0.12)))

    disc = Image.new("L", img.size, 0)
    ImageDraw.Draw(disc).ellipse(box, fill=255)
    patches = Image.new("L", img.size, 0)
    pd = ImageDraw.Draw(patches)
    up = -math.pi / 2
    seams = Image.new("L", img.size, 0)
    sd = ImageDraw.Draw(seams)
    sw = max(1, int(s * 0.018))
    middle = pentagon(cx, cy, 0.36 * s, up)
    pd.polygon(middle, fill=255)
    outer = []
    for k in range(5):
        a = up + k * 2 * math.pi / 5
        px, py = cx + 0.78 * s * math.cos(a), cy + 0.78 * s * math.sin(a)
        outer.append(pentagon(px, py, 0.2 * s, a))
        pd.polygon(outer[-1], fill=255)
    for k in range(5):
        here, there = outer[k], outer[(k + 1) % 5]
        inner = ((here[2][0] + here[3][0]) / 2, (here[2][1] + here[3][1]) / 2)
        sd.line([middle[k], inner], fill=255, width=sw)       # centre patch to the one beyond it
        sd.line([here[1], there[4]], fill=255, width=sw)      # between neighbouring outer patches
    patches = ImageChops.multiply(ImageChops.lighter(patches, ImageChops.multiply(seams, Image.new("L", img.size, 150))), disc)

    face = Image.new("RGBA", img.size, LINE + (255,))
    face.paste(BLACK + (255,), mask=patches)
    face.putalpha(disc)
    img.alpha_composite(face)
    rim = max(2, int(s * 0.025))
    ImageDraw.Draw(img).ellipse(box, outline=(40, 44, 48, 255), width=rim)
    return img


def main(out):
    out = pathlib.Path(out)
    out.mkdir(parents=True, exist_ok=True)
    icon = ball(pitch(gradient(512, 512)), 1.0)
    icon.convert("RGB").save(out / "futbol13.png", optimize=False)
    w, h = 3840, 2160
    bg = ball(pitch(gradient(w, h)), 0.8, (0.3 * w, h / 2))
    bg.convert("RGB").save(out / "futbol13-bg.png", optimize=False)
    logo = ball(Image.new("RGBA", (w, h), (0, 0, 0, 0)), 0.45)
    logo.save(out / "futbol13-logo.png", optimize=False)


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else pathlib.Path(__file__).parent)
