"""
make_house_ending_art.py: TAKE 4471. The ending signage, dailies, skyline, cake top and title background
redrawn in the game's own house style (the flat HALCYON look of the posters, screens and boards), using Sol's
pieces (ART-010..016, title key art v04) as the content reference.

  T_TitleKeyArt         1920x1080  the title screen background (left side stays calm: the menu sits there)
  T_WrapBanner          2048x512   the wrap party banner
  T_CakeTop             1024x1024  the cake you can cut, seen from above (RGBA, round)
  T_DirectorCard_Blank  1024x256   the director's chair name card, before you sit ...
  T_DirectorCard_4471   1024x256   ... and after
  T_OvertimeMenu        600x840    the kitchen board in Act 3 (same frame as T_LunchSpecials)
  T_Dailies_Desk / _Conference / _CorridorB / _Ward   1024x582  the director's monitor wall
  T_PenthouseSkyline    4096x1024  the city at dusk, past the director's floor

Usage: python make_house_ending_art.py [outdir] [names...]   (playwright + Chrome; Poppins, Inter, Caveat installed)
"""
import math
import os
import sys

from playwright.sync_api import sync_playwright

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = sys.argv[1] if len(sys.argv) > 1 else HERE
ONLY = sys.argv[2:]   # optional: just these names
MINT, MINT_D, INK, SLATE, PAPER, PINK, CREAM = "#7fd4c4", "#5fb9a8", "#12313a", "#2b3a40", "#f4f7f8", "#f2b8b5", "#ece6dc"


def mark(color, size, stroke=None):
    """The HALCYON half-sun (as on the posters and the reception desk)."""
    rays = "".join(
        f'<line x1="{50 + 30 * math.cos(a):.1f}" y1="{56 - 30 * math.sin(a):.1f}" x2="{50 + 40 * math.cos(a):.1f}" y2="{56 - 40 * math.sin(a):.1f}" '
        f'stroke="{color}" stroke-width="4" stroke-linecap="round"/>' for a in [math.pi * k / 8 for k in range(1, 8)])
    return (f'<svg viewBox="0 0 100 80" width="{size}" height="{size * 0.8}">'
            f'<path d="M26 56 A24 24 0 0 1 74 56 Z" fill="{color}"/>{rays}'
            f'<line x1="10" y1="64" x2="90" y2="64" stroke="{color}" stroke-width="5" stroke-linecap="round"/>'
            f'<line x1="26" y1="74" x2="74" y2="74" stroke="{color}" stroke-width="4" stroke-linecap="round"/></svg>')


def person(x, y, s=1.0, body=MINT, head=CREAM, back=True):
    """The office's mannequin, flat: a rounded capsule body and a round head (as on the CCTV and promo screens)."""
    w, h, r = 46 * s, 110 * s, 23 * s
    return (f'<rect x="{x - w / 2:.1f}" y="{y - h:.1f}" width="{w:.1f}" height="{h:.1f}" rx="{r:.1f}" fill="{body}"/>'
            f'<circle cx="{x:.1f}" cy="{y - h - 20 * s:.1f}" r="{19 * s:.1f}" fill="{head}"/>')


def svg_page(w, h, inner, bg="transparent"):
    return (f'<div style="width:{w}px;height:{h}px;background:{bg};overflow:hidden">'
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" viewBox="0 0 {w} {h}" '
            f'style="font-family:Poppins,Inter,sans-serif">{inner}</svg></div>')


# ------------------------------------------------------------------ title background
# Between the flat house style and Sol's photographic key art: the same flat shapes, but lit. Soft gradients on every
# surface, daylight falling through the windows onto the carpet, blurred contact shadows, a little haze toward the
# back, bloom on the ceiling lights and a fine grain, so it reads as a place rather than a diagram.
def title():
    W, H = 1920, 1080
    vx, vy = 1180, 470            # vanishing point
    bx0, by0, bx1, by1 = 820, 330, 1540, 610   # the back wall
    g = ["""<defs>
      <linearGradient id="ceil" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#fbfcfc"/><stop offset="1" stop-color="#e4ebec"/></linearGradient>
      <linearGradient id="lwall" x1="0" y1="0" x2="1" y2="0"><stop offset="0" stop-color="#dfe8ea"/><stop offset="1" stop-color="#eef3f4"/></linearGradient>
      <linearGradient id="rwall" x1="1" y1="0" x2="0" y2="0"><stop offset="0" stop-color="#cfd9dc"/><stop offset="1" stop-color="#e6ecee"/></linearGradient>
      <linearGradient id="back" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#7a98a5"/><stop offset="1" stop-color="#5f7d8b"/></linearGradient>
      <linearGradient id="floor" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#7890a0"/><stop offset="0.5" stop-color="#5d7684"/><stop offset="1" stop-color="#3f5560"/></linearGradient>
      <linearGradient id="shaft" x1="0" y1="0" x2="1" y2="0"><stop offset="0" stop-color="#ffffff" stop-opacity="0.38"/><stop offset="1" stop-color="#ffffff" stop-opacity="0"/></linearGradient>
      <linearGradient id="glass" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#e9f5f5"/><stop offset="0.45" stop-color="#cfe6e8"/><stop offset="1" stop-color="#b9d6d9"/></linearGradient>
      <linearGradient id="body" x1="0" y1="0" x2="1" y2="0"><stop offset="0" stop-color="#9be0d2"/><stop offset="0.55" stop-color="#7fd4c4"/><stop offset="1" stop-color="#5fb9a8"/></linearGradient>
      <radialGradient id="head" cx="0.35" cy="0.35" r="0.8"><stop offset="0" stop-color="#fbf8f2"/><stop offset="1" stop-color="#d8d0c2"/></radialGradient>
      <linearGradient id="panel" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#8fdccd"/><stop offset="1" stop-color="#63bfae"/></linearGradient>
      <linearGradient id="panelback" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#e3f3ef"/><stop offset="1" stop-color="#c7e6df"/></linearGradient>
      <linearGradient id="desk" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#f6f2ec"/><stop offset="1" stop-color="#e3ddd3"/></linearGradient>
      <linearGradient id="mug" x1="0" y1="0" x2="1" y2="0"><stop offset="0" stop-color="#5fb9a8"/><stop offset="0.35" stop-color="#93ddcf"/><stop offset="1" stop-color="#57ad9c"/></linearGradient>
      <linearGradient id="canvas" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#2a3236"/><stop offset="1" stop-color="#14191b"/></linearGradient>
      <radialGradient id="haze" cx="0.5" cy="0.5" r="0.5"><stop offset="0" stop-color="#eef4f5" stop-opacity="0.2"/><stop offset="1" stop-color="#eef4f5" stop-opacity="0"/></radialGradient>
      <radialGradient id="vig" cx="0.6" cy="0.45" r="0.85"><stop offset="0.55" stop-color="#000" stop-opacity="0"/><stop offset="1" stop-color="#0b1a1f" stop-opacity="0.42"/></radialGradient>
      <filter id="soft" x="-50%" y="-50%" width="200%" height="200%"><feGaussianBlur stdDeviation="14"/></filter>
      <filter id="softer" x="-50%" y="-50%" width="200%" height="200%"><feGaussianBlur stdDeviation="30"/></filter>
      <filter id="bloom" x="-50%" y="-200%" width="200%" height="500%"><feGaussianBlur stdDeviation="9"/></filter>
      <filter id="grain"><feTurbulence type="fractalNoise" baseFrequency="0.9" numOctaves="2" seed="4471"/><feColorMatrix type="saturate" values="0"/>
        <feComponentTransfer><feFuncA type="table" tableValues="0 0.07"/></feComponentTransfer></filter>
      <filter id="carpet"><feTurbulence type="fractalNoise" baseFrequency="0.6 1.8" numOctaves="3" seed="7"/><feColorMatrix type="saturate" values="0"/>
        <feComponentTransfer><feFuncA type="table" tableValues="0 0.12"/></feComponentTransfer></filter>
      <clipPath id="floorclip"><polygon points="FLOORPTS"/></clipPath>
    </defs>""".replace("FLOORPTS", f"{bx0},{by1} {bx1},{by1} {W},{H} 0,{H}")]
    g.append(f'<polygon points="0,0 {W},0 {bx1},{by0} {bx0},{by0}" fill="url(#ceil)"/>')
    g.append(f'<polygon points="0,0 {bx0},{by0} {bx0},{by1} 0,{H}" fill="url(#lwall)"/>')
    g.append(f'<polygon points="{W},0 {bx1},{by0} {bx1},{by1} {W},{H}" fill="url(#rwall)"/>')
    g.append(f'<polygon points="{bx0},{by1} {bx1},{by1} {W},{H} 0,{H}" fill="url(#floor)"/>')
    g.append(f'<rect x="0" y="0" width="{W}" height="{H}" filter="url(#carpet)" clip-path="url(#floorclip)"/>')
    g.append(f'<rect x="{bx0}" y="{by0}" width="{bx1 - bx0}" height="{by1 - by0}" fill="url(#back)"/>')
    # corners: a little ambient occlusion where walls meet
    g.append(f'<g opacity="0.35" filter="url(#soft)"><line x1="{bx0}" y1="{by0}" x2="{bx0}" y2="{by1}" stroke="#4e6a76" stroke-width="18"/>'
             f'<line x1="{bx1}" y1="{by0}" x2="{bx1}" y2="{by1}" stroke="#4e6a76" stroke-width="18"/>'
             f'<line x1="{bx0}" y1="{by1}" x2="{bx1}" y2="{by1}" stroke="#2e434d" stroke-width="16"/>'
             f'<line x1="{bx0}" y1="{by1}" x2="0" y2="{H}" stroke="#2e434d" stroke-width="22"/><line x1="{bx1}" y1="{by1}" x2="{W}" y2="{H}" stroke="#2e434d" stroke-width="22"/></g>')
    # ceiling light panels with bloom
    for row in range(5):
        t = 0.12 + row * 0.17
        y = (1 - t) * 40 + t * by0
        for col in (-1, 0, 1):
            cx = (1 - t) * (960 + col * 620) + t * (vx + col * 230)
            wdt = (1 - t) * 260 + t * 90
            hgt = (1 - t) * 22 + t * 8
            g.append(f'<rect x="{cx - wdt / 2:.0f}" y="{y - hgt:.0f}" width="{wdt:.0f}" height="{hgt * 3:.0f}" fill="#ffffff" opacity="0.55" filter="url(#bloom)"/>')
            g.append(f'<rect x="{cx - wdt / 2:.0f}" y="{y:.0f}" width="{wdt:.0f}" height="{hgt:.0f}" rx="4" fill="#ffffff"/>')
    # windows on the left wall, and the daylight they throw across the floor
    for i in range(6):
        t0, t1 = i / 6, (i + 0.82) / 6
        x0, x1 = t0 * bx0, t1 * bx0
        yt0, yt1 = t0 * by0 + 60 * (1 - t0), t1 * by0 + 60 * (1 - t1)
        yb0, yb1 = H - t0 * (H - by1) - 200 * (1 - t0), H - t1 * (H - by1) - 200 * (1 - t1)
        g.append(f'<polygon points="{x0:.0f},{yt0:.0f} {x1:.0f},{yt1:.0f} {x1:.0f},{yb1:.0f} {x0:.0f},{yb0:.0f}" fill="url(#glass)"/>')
        g.append(f'<polygon points="{x0:.0f},{yt0:.0f} {x1:.0f},{yt1:.0f} {x1:.0f},{yt1 + (yb1 - yt1) * 0.3:.0f} {x0:.0f},{yt0 + (yb0 - yt0) * 0.3:.0f}" fill="#f4fbfb" opacity="0.7"/>')
        # the light patch on the carpet: the window's shape, sheared toward the middle of the room
        fx0, fx1 = x0 + 260 * (1 - t0) + 120, x1 + 260 * (1 - t1) + 120
        g.append(f'<polygon points="{x0:.0f},{yb0:.0f} {x1:.0f},{yb1:.0f} {fx1 + 140:.0f},{yb1 + 40:.0f} {fx0 + 140:.0f},{yb0 + 80:.0f}" fill="url(#shaft)" opacity="0.8" clip-path="url(#floorclip)"/>')
    # the back wall: a door (red light over it), the HALCYON mark, a camera
    g.append('<rect x="1396" y="436" width="100" height="174" fill="#3f5864"/><rect x="1404" y="444" width="84" height="166" fill="#4b6672"/>'
             '<circle cx="1446" cy="420" r="16" fill="#e5484d" opacity="0.45" filter="url(#bloom)"/><circle cx="1446" cy="420" r="6" fill="#ff5a5f"/>')
    g.append(mark("#ffffff", 130).replace("<svg ", '<svg x="1076" y="388" opacity="0.92" ', 1))
    g.append('<text x="1141" y="530" text-anchor="middle" font-size="30" font-weight="600" letter-spacing="9" fill="#ffffff" opacity="0.92">HALCYON</text>')
    g.append('<g><rect x="1600" y="250" width="62" height="34" rx="6" fill="#f6f8f8" stroke="#cdd8db" stroke-width="2"/>'
             '<rect x="1650" y="256" width="40" height="22" rx="4" fill="#e9eef0"/><circle cx="1614" cy="267" r="6" fill="#2b3a40"/>'
             '<circle cx="1640" cy="262" r="9" fill="#ff4d4f" opacity="0.5" filter="url(#bloom)"/><circle cx="1640" cy="262" r="3.5" fill="#e5484d"/><rect x="1660" y="226" width="8" height="28" fill="#cdd8db"/></g>')
    # cubicles along the left (far to near): contact shadows, lit panels, people with a little volume
    for x, y, s in [(870, 655, 0.58), (745, 715, 0.82), (560, 820, 1.2)]:
        g.append(f'<ellipse cx="{x - 15 * s:.0f}" cy="{y + 6 * s:.0f}" rx="{200 * s:.0f}" ry="{20 * s:.0f}" fill="#22343c" opacity="0.45" filter="url(#soft)"/>')
        g.append(f'<rect x="{x - 180 * s:.0f}" y="{y - 230 * s:.0f}" width="{330 * s:.0f}" height="{230 * s:.0f}" fill="url(#panelback)"/>')
        g.append(f'<rect x="{x - 180 * s:.0f}" y="{y - 230 * s:.0f}" width="{330 * s:.0f}" height="{8 * s:.0f}" fill="#b6dcd3"/>')
        g.append(f'<rect x="{x - 130 * s:.0f}" y="{y - 150 * s:.0f}" width="{78 * s:.0f}" height="{52 * s:.0f}" rx="3" fill="{SLATE}"/>')
        g.append(f'<rect x="{x - 125 * s:.0f}" y="{y - 145 * s:.0f}" width="{68 * s:.0f}" height="{42 * s:.0f}" fill="#9fdccf"/>')
        g.append(f'<rect x="{x - 135 * s:.0f}" y="{y - 160 * s:.0f}" width="{88 * s:.0f}" height="{70 * s:.0f}" fill="#bff0e6" opacity="0.35" filter="url(#bloom)"/>')
        px, py, ps = x + 30 * s, y - 40 * s, s * 0.95
        g.append(f'<rect x="{px - 23 * ps:.0f}" y="{py - 110 * ps:.0f}" width="{46 * ps:.0f}" height="{110 * ps:.0f}" rx="{23 * ps:.0f}" fill="url(#body)"/>')
        g.append(f'<circle cx="{px:.0f}" cy="{py - 130 * ps:.0f}" r="{19 * ps:.0f}" fill="url(#head)"/>')
        g.append(f'<rect x="{x - 185 * s:.0f}" y="{y - 92 * s:.0f}" width="{340 * s:.0f}" height="{92 * s:.0f}" fill="url(#panel)"/>')
        g.append(f'<rect x="{x - 185 * s:.0f}" y="{y - 92 * s:.0f}" width="{340 * s:.0f}" height="{9 * s:.0f}" fill="#a9e6da"/>')
    # haze toward the back (depth)
    g.append(f'<ellipse cx="{vx}" cy="{vy}" rx="520" ry="230" fill="url(#haze)"/>')
    # the director's chair, empty, on its tape mark, in a pool of light
    cx, cy = 1360, 790
    g.append(f'<ellipse cx="{cx}" cy="{cy - 10}" rx="230" ry="70" fill="#ffffff" opacity="0.16" filter="url(#softer)"/>')
    g.append(f'<ellipse cx="{cx + 10}" cy="{cy + 8}" rx="125" ry="20" fill="#1c2b31" opacity="0.6" filter="url(#soft)"/>')
    g.append(f'<g stroke="{MINT}" stroke-width="7" stroke-linecap="round" opacity="0.9"><line x1="{cx - 60}" y1="{cy + 30}" x2="{cx + 60}" y2="{cy + 60}"/>'
             f'<line x1="{cx + 60}" y1="{cy + 30}" x2="{cx - 60}" y2="{cy + 60}"/></g>')
    g.append(f'<g stroke="#1b2326" stroke-width="12" stroke-linecap="round">'
             f'<line x1="{cx - 70}" y1="{cy}" x2="{cx + 60}" y2="{cy - 130}"/><line x1="{cx + 70}" y1="{cy}" x2="{cx - 60}" y2="{cy - 130}"/>'
             f'<line x1="{cx - 75}" y1="{cy - 130}" x2="{cx - 75}" y2="{cy - 290}"/><line x1="{cx + 75}" y1="{cy - 130}" x2="{cx + 75}" y2="{cy - 290}"/>'
             f'<line x1="{cx - 95}" y1="{cy - 190}" x2="{cx - 40}" y2="{cy - 190}"/><line x1="{cx + 40}" y1="{cy - 190}" x2="{cx + 95}" y2="{cy - 190}"/></g>')
    g.append(f'<g stroke="#3a464b" stroke-width="3" stroke-linecap="round" opacity="0.8"><line x1="{cx - 79}" y1="{cy - 135}" x2="{cx - 79}" y2="{cy - 285}"/>'
             f'<line x1="{cx + 71}" y1="{cy - 135}" x2="{cx + 71}" y2="{cy - 285}"/></g>')   # a highlight on the frame
    g.append(f'<rect x="{cx - 80}" y="{cy - 140}" width="160" height="18" fill="url(#canvas)"/>')
    g.append(f'<rect x="{cx - 80}" y="{cy - 290}" width="160" height="62" fill="url(#canvas)"/>')
    g.append(f'<text x="{cx}" y="{cy - 248}" text-anchor="middle" font-size="24" font-weight="700" letter-spacing="5" fill="#f4f7f8">DIRECTOR</text>')
    # a plant by the back wall
    g.append('<ellipse cx="1590" cy="626" rx="40" ry="8" fill="#22343c" opacity="0.4" filter="url(#soft)"/><rect x="1560" y="560" width="54" height="64" rx="6" fill="#f4f7f8"/>' +
             "".join(f'<ellipse cx="{1587 + dx}" cy="{548 + dy}" rx="22" ry="40" transform="rotate({rot} {1587 + dx} {548 + dy})" fill="{c}"/>'
                     for dx, dy, rot, c in [(-18, -10, -25, "#5aa47f"), (16, -14, 22, "#4c9572"), (0, -36, 0, "#6bb88f"), (-4, 4, -60, "#4c9572"), (8, 0, 60, "#5aa47f")]))
    # foreground: the corner of desk 4471, your mug, your lanyard and badge (slightly soft: it's close to the lens)
    g.append(f'<polygon points="1060,1080 1240,860 {W},830 {W},1080" fill="url(#desk)"/>')
    g.append(f'<polygon points="1060,1080 1240,860 1250,878 1076,1080" fill="#d2cabd"/>')
    g.append('<ellipse cx="1550" cy="912" rx="90" ry="14" fill="#5b5146" opacity="0.35" filter="url(#soft)"/>')
    g.append(f'<g transform="translate(1480,772)"><rect x="0" y="0" width="120" height="140" rx="16" fill="url(#mug)"/>'
             f'<path d="M120 34 q46 0 46 38 q0 38 -46 38" fill="none" stroke="{MINT_D}" stroke-width="18"/>'
             f'<ellipse cx="60" cy="2" rx="60" ry="10" fill="#4f9f90"/><ellipse cx="60" cy="4" rx="50" ry="7" fill="#3a2a22" opacity="0.85"/>'
             f'<rect x="14" y="10" width="10" height="120" rx="5" fill="#ffffff" opacity="0.28"/>'
             f'<g transform="translate(22,44)">{mark("#ffffff", 76)}</g></g>')
    g.append(f'<path d="M1300 984 C 1380 904, 1560 964, 1640 944 S 1800 904, 1820 964" fill="none" stroke="#3f7f73" stroke-width="16" stroke-linecap="round" opacity="0.35" filter="url(#bloom)"/>')
    g.append(f'<path d="M1300 980 C 1380 900, 1560 960, 1640 940 S 1800 900, 1820 960" fill="none" stroke="{MINT_D}" stroke-width="16" stroke-linecap="round"/>')
    g.append('<g transform="translate(1700,945) rotate(-12)"><rect x="4" y="8" width="150" height="100" rx="10" fill="#5b5146" opacity="0.3" filter="url(#soft)"/>'
             '<rect x="0" y="0" width="150" height="100" rx="10" fill="#ffffff" stroke="#d3dcde" stroke-width="3"/>'
             f'<rect x="58" y="-14" width="34" height="22" rx="4" fill="#b8c4c7"/><text x="75" y="70" text-anchor="middle" font-size="40" font-weight="700" fill="{INK}">4471</text></g>')
    g.append('<rect width="1920" height="1080" fill="url(#vig)"/>')
    g.append('<rect width="1920" height="1080" filter="url(#grain)"/>')
    return (W, H, svg_page(W, H, "".join(g)))


# ------------------------------------------------------------------ the wrap banner
def banner():
    flags = "".join(
        f'<polygon points="{60 + i * 96},0 {140 + i * 96},0 {100 + i * 96},62" fill="{[MINT, "#ffffff", "#e3b65a"][i % 3]}"/>' for i in range(20))
    return (2048, 512, f'''<div style="position:relative;width:2048px;height:512px;background:{MINT};font-family:Poppins,Inter,sans-serif;color:#fff;overflow:hidden">
      <svg width="2048" height="70" style="position:absolute;left:0;top:18px">{flags.replace(MINT, MINT_D)}</svg>
      <div style="position:absolute;left:0;right:0;top:118px;display:flex;align-items:center;justify-content:center;gap:56px">
        {mark("#ffffff", 230)}
        <div><div style="font:800 168px/1 Poppins;letter-spacing:6px">THAT'S A WRAP!</div>
        <div style="font:600 44px Poppins;letter-spacing:16px;margin-top:22px;opacity:.92">CONGRATULATIONS, EMPLOYEE 4471</div></div></div></div>''')


# ------------------------------------------------------------------ the cake top
def caketop():
    R = 500
    dots = "".join(f'<circle cx="{512 + 462 * math.cos(a):.1f}" cy="{512 + 462 * math.sin(a):.1f}" r="27" fill="#fbf8f2" stroke="#e7e1d6" stroke-width="3"/>'
                   for a in [2 * math.pi * k / 46 for k in range(46)])
    import random
    rnd = random.Random(4471)
    sprinkles = ""
    for _ in range(70):
        a, r = rnd.uniform(0, 2 * math.pi), rnd.uniform(60, 400)
        x, y = 512 + r * math.cos(a), 512 + r * math.sin(a)
        if 250 < x < 780 and 300 < y < 760:   # keep the lettering clear
            continue
        sprinkles += (f'<rect x="{x:.0f}" y="{y:.0f}" width="22" height="7" rx="3.5" fill="{rnd.choice([PINK, "#ffffff", "#e3b65a", MINT_D])}" '
                      f'transform="rotate({rnd.uniform(0, 180):.0f} {x:.0f} {y:.0f})"/>')
    inner = (f'<circle cx="512" cy="512" r="{R}" fill="{MINT}"/><circle cx="512" cy="512" r="{R - 70}" fill="#8edccd"/>{sprinkles}{dots}'
             f'<g transform="translate(437,250)">{mark(INK, 150)}</g>'
             f'<text x="512" y="510" text-anchor="middle" font-size="92" font-weight="800" fill="{INK}" letter-spacing="2">THAT\'S A</text>'
             f'<text x="512" y="610" text-anchor="middle" font-size="92" font-weight="800" fill="{INK}" letter-spacing="2">WRAP,</text>'
             f'<text x="512" y="745" text-anchor="middle" font-size="128" font-weight="800" fill="#ffffff" stroke="{INK}" stroke-width="6" paint-order="stroke" letter-spacing="6">4471</text>')
    return (1024, 1024, svg_page(1024, 1024, inner))


# ------------------------------------------------------------------ director's chair cards
def card(name):
    blank = '<span style="display:inline-block;width:250px;border-bottom:8px solid #fff;height:84px"></span>'
    return (1024, 256, f'''<div style="position:relative;width:1024px;height:256px;background:#161c1f;font-family:Poppins,Inter,sans-serif;overflow:hidden">
      <div style="position:absolute;left:0;right:0;bottom:0;height:16px;background:{MINT}"></div>
      <div style="position:absolute;left:0;right:0;top:0;bottom:16px;display:flex;align-items:center;justify-content:center;gap:28px">
        {mark(MINT, 100)}
        <div style="font:800 92px/1 Poppins;color:#fff;letter-spacing:4px">DIRECTOR:</div>
        <div style="font:800 92px/1 Poppins;color:{MINT};letter-spacing:4px">{name or blank}</div></div></div>''')


# ------------------------------------------------------------------ the overtime board (same frame as the lunch board)
def menu():
    return (600, 840, f'''<div style="position:relative;width:600px;height:840px;font-family:Poppins,Inter,sans-serif">
      <div style="position:absolute;inset:0;background:#24302f;border:18px solid #8a6a46"></div>
      <div style="position:absolute;left:0;right:0;top:40px;text-align:center">{mark(MINT, 130)}</div>
      <div style="position:absolute;left:0;right:0;top:150px;font:800 52px Poppins;color:#f4f7f8;text-align:center;letter-spacing:4px">OVERTIME MENU</div>
      <div style="position:absolute;left:0;right:0;top:218px;font:600 21px Poppins;color:{MINT};text-align:center;letter-spacing:3px">AFTER HOURS. SAME COMMITMENT.</div>
      <div style="position:absolute;left:60px;right:44px;top:282px;font:44px/1.22 Caveat;color:#f4f7f8">
        &bull; Coffee <span style="font-size:32px">(still warm. always warm)</span><br>
        &bull; Cake <span style="font-size:32px">(for later. DON'T touch)</span><br>
        &bull; Soup of the day: today's<br>&nbsp;&nbsp;<span style="font-size:32px">(it was yesterday's at lunch)</span><br>
        &bull; Craft services closes when<br>&nbsp;&nbsp;the director says it closes</div>
      <div style="position:absolute;left:0;right:0;bottom:44px;font:600 22px Poppins;color:#9fb2b8;text-align:center">Please remain available.</div></div>''')


# ------------------------------------------------------------------ the dailies (the CCTV look, with a slate)
def daily(scene, label, tc):
    W, H = 1024, 582
    g = [f'<rect width="{W}" height="{H}" fill="#05080a"/>', f'<rect x="8" y="8" width="{W - 16}" height="{H - 16}" fill="url(#bg)"/>']
    if scene == "desk":   # 4471 from behind, at the desk, the monitor glowing
        g += ['<defs><linearGradient id="bg" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#40504f"/><stop offset="1" stop-color="#26302f"/></linearGradient></defs>',
              '<rect x="120" y="300" width="780" height="40" fill="#cfc8bc"/><rect x="140" y="340" width="20" height="200" fill="#9aa"/><rect x="860" y="340" width="20" height="200" fill="#9aa"/>',
              '<rect x="420" y="140" width="260" height="150" rx="6" fill="#0e1416"/><rect x="432" y="152" width="236" height="126" fill="#7fd4c4" opacity="0.85"/><rect x="535" y="290" width="30" height="14" fill="#333"/>',
              '<rect x="240" y="120" width="12" height="190" fill="#bfe5dc"/><rect x="250" y="120" width="0" height="0"/>',
              person(510, 520, 2.1), '<rect x="700" y="268" width="40" height="34" rx="6" fill="#7fd4c4"/>']
    elif scene == "conference":   # the 10:00 call: everyone around the table, the screen of faces
        g += ['<defs><linearGradient id="bg" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#3d4a4c"/><stop offset="1" stop-color="#263132"/></linearGradient></defs>',
              '<rect x="352" y="60" width="320" height="150" rx="6" fill="#0e1416"/>'] + \
             [f'<rect x="{362 + (i % 3) * 102}" y="{70 + (i // 3) * 68}" width="96" height="62" fill="{"#1d2a2e" if i != 4 else "#000"}"/>'
              + (f'<circle cx="{410 + (i % 3) * 102}" cy="{101 + (i // 3) * 68}" r="14" fill="#9fb3b9"/>' if i != 4 else "") for i in range(6)] + \
             ['<polygon points="330,330 694,330 760,500 264,500" fill="#d8cdb8"/>'] + \
             [person(x, y, 0.9) for x, y in [(300, 360), (290, 450), (724, 360), (734, 450)]] + [person(512, 560, 1.2, body="#7fd4c4")]
    elif scene == "corridor":   # Corridor B, one point, the far door, something standing in it
        g += ['<defs><linearGradient id="bg" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#2a2f30"/><stop offset="1" stop-color="#151a1b"/></linearGradient></defs>',
              '<polygon points="8,8 1016,8 600,230 424,230" fill="#3a4446"/><polygon points="8,574 1016,574 600,350 424,350" fill="#232b2c"/>',
              '<polygon points="8,8 424,230 424,350 8,574" fill="#4a5658"/><polygon points="1016,8 600,230 600,350 1016,574" fill="#445052"/>',
              '<rect x="424" y="230" width="176" height="120" fill="#1b2122"/><rect x="486" y="262" width="52" height="88" fill="#0b0f10"/>'] + \
             [f'<polygon points="{x0},{y0} {x1},{y1} {x1},{y2} {x0},{y3}" fill="#7fd4c4" opacity="0.55"/>'
              for x0, y0, x1, y1, y2, y3 in [(80, 120, 150, 150, 440, 480), (250, 196, 300, 210, 380, 400), (944, 120, 874, 150, 440, 480), (774, 196, 724, 210, 380, 400)]] + \
             ['<rect x="492" y="270" width="40" height="80" rx="20" fill="#000"/>'] + \
             [f'<rect x="{512 - w / 2:.0f}" y="{y}" width="{w}" height="6" fill="#e9f2f2" opacity="0.8"/>' for y, w in [(110, 180), (170, 110), (205, 70)]]
    else:   # the ward: beds in a row, a curtain, the window you can't see through
        g += ['<defs><linearGradient id="bg" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#3f4c4e"/><stop offset="1" stop-color="#28302f"/></linearGradient></defs>',
              '<rect x="620" y="70" width="330" height="170" fill="#0b1012" stroke="#9fb3b9" stroke-width="6"/>',
              '<rect x="400" y="100" width="12" height="320" fill="#9fb3b9"/><rect x="330" y="110" width="120" height="300" fill="#7fd4c4" opacity="0.6"/>'] + \
             [f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="8" fill="#e8eef0"/><rect x="{x}" y="{y}" width="{w * 0.28:.0f}" height="{h}" rx="8" fill="#bfe5dc"/>'
              f'<rect x="{x + 6}" y="{y + h}" width="8" height="40" fill="#9fb3b9"/><rect x="{x + w - 14}" y="{y + h}" width="8" height="40" fill="#9fb3b9"/>'
              for x, y, w, h in [(90, 300, 230, 70), (480, 360, 300, 90), (140, 440, 330, 100)]]
    g += [f'<text x="24" y="40" font-family="DejaVu Sans Mono,monospace" font-size="19" font-weight="600" fill="#cfe">DAILIES &#183; TAKE 4471 &#183; {label}</text>',
          f'<circle cx="930" cy="33" r="7" fill="#f55"/><text x="944" y="40" font-family="DejaVu Sans Mono,monospace" font-size="19" font-weight="600" fill="#f55">REC</text>',
          f'<text x="1000" y="560" text-anchor="end" font-family="DejaVu Sans Mono,monospace" font-size="18" font-weight="600" fill="#cfe">{tc}</text>',
          '<rect x="8" y="8" width="1008" height="566" fill="url(#scan)"/>',
          '<defs><pattern id="scan" width="4" height="4" patternUnits="userSpaceOnUse"><rect y="3" width="4" height="1" fill="#000" opacity="0.25"/></pattern></defs>']
    return (W, H, svg_page(W, H, "".join(g)))


# ------------------------------------------------------------------ the skyline
def skyline():
    import random
    rnd = random.Random(2214)
    W, H = 4096, 1024
    g = ['<defs><linearGradient id="sky" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#2c4656"/><stop offset="0.45" stop-color="#6f8fa0"/>'
         f'<stop offset="0.72" stop-color="{PINK}"/><stop offset="0.86" stop-color="#f6d6c4"/></linearGradient></defs>',
         f'<rect width="{W}" height="{H}" fill="url(#sky)"/>', f'<circle cx="2300" cy="760" r="120" fill="#fbe3d3" opacity="0.8"/>']
    for layer, (col, base, hmin, hmax, win) in enumerate([("#6f8796", 820, 80, 260, 0.0), ("#46606d", 870, 140, 420, 0.25), (SLATE, 940, 120, 360, 0.4)]):
        x = -40
        while x < W:
            w = rnd.randint(110, 280)
            h = rnd.randint(hmin, hmax)
            top = base - h
            g.append(f'<rect x="{x}" y="{top}" width="{w}" height="{H - top}" fill="{col}"/>')
            if win:
                for wy in range(top + 24, H - 40, 36):
                    for wx in range(x + 16, x + w - 20, 30):
                        if rnd.random() < win * 0.5:
                            g.append(f'<rect x="{wx}" y="{wy}" width="12" height="16" fill="{MINT if rnd.random() < 0.7 else "#fbe3d3"}" opacity="0.9"/>')
            x += w + rnd.randint(-20, 30)
    # HALCYON HQ, tallest, with the half-sun on top (a little too bright)
    g.append(f'<rect x="1700" y="300" width="300" height="724" fill="#22333a"/><rect x="1730" y="330" width="240" height="8" fill="{MINT}"/>')
    g.append(f'<g transform="translate(1775,190)">{mark(MINT, 150)}</g>')
    for wy in range(380, 1000, 40):
        for wx in range(1724, 1980, 34):
            if (wx + wy) % 3:
                g.append(f'<rect x="{wx}" y="{wy}" width="14" height="18" fill="{MINT}" opacity="0.85"/>')
    return (W, H, svg_page(W, H, "".join(g)))


# ------------------------------------------------------------------ the party flier (kitchen board, after lunch)
def flier():
    return (600, 800, f'''<div style="position:relative;width:600px;height:800px;background:{PAPER};font-family:Poppins,Inter,sans-serif;overflow:hidden">
      <div style="position:absolute;left:0;right:0;top:0;height:250px;background:{MINT}"></div>
      <svg width="600" height="60" style="position:absolute;left:0;top:14px">{"".join(f'<polygon points="{20 + i * 58},0 {68 + i * 58},0 {44 + i * 58},40" fill="{["#ffffff", "#e3b65a", MINT_D][i % 3]}"/>' for i in range(10))}</svg>
      <div style="position:absolute;left:0;right:0;top:78px;text-align:center">{mark("#ffffff", 120)}</div>
      <div style="position:absolute;left:0;right:0;top:270px;font:800 76px/1 Poppins;color:{INK};text-align:center">WRAP<br>PARTY!</div>
      <div style="position:absolute;left:0;right:0;top:450px;font:600 34px Poppins;color:{INK};text-align:center">Tonight &#183; The Lobby</div>
      <div style="position:absolute;left:0;right:0;top:505px;font:600 30px Poppins;color:{MINT_D};text-align:center">Doors at 2:14</div>
      <div style="position:absolute;left:60px;right:60px;top:575px;font:500 24px/1.45 Inter;color:#5a6c72;text-align:center">Cake. Music. Speeches.<br>Attendance is celebration.<br>Celebration is mandatory.</div>
      <div style="position:absolute;left:0;right:0;bottom:34px;font:38px Caveat;color:#b02a2a;transform:rotate(-3deg)">(you're the guest of honour)</div></div>''')


# ------------------------------------------------------------------ office art: a corporate print for the blank wall
def officeart():
    W, H = 1600, 400
    g = [f'<defs><linearGradient id="sk" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#e9f5f2"/><stop offset="0.7" stop-color="{PINK}"/><stop offset="1" stop-color="#f6d6c4"/></linearGradient></defs>',
         f'<rect width="{W}" height="{H}" fill="url(#sk)"/>',
         f'<circle cx="{W / 2}" cy="300" r="150" fill="#ffffff" opacity="0.85"/>',
         f'<path d="M{W / 2 - 150} 300 A150 150 0 0 1 {W / 2 + 150} 300 Z" fill="{MINT}"/>']
    for i, a in enumerate([math.pi * k / 10 for k in range(1, 10)]):
        g.append(f'<line x1="{W / 2 + 175 * math.cos(a):.0f}" y1="{300 - 175 * math.sin(a):.0f}" x2="{W / 2 + 230 * math.cos(a):.0f}" y2="{300 - 230 * math.sin(a):.0f}" stroke="{MINT}" stroke-width="10" stroke-linecap="round"/>')
    for k, (y, c) in enumerate([(300, MINT_D), (330, MINT), (355, "#a9e6da"), (378, "#cdeee6")]):
        g.append(f'<rect x="0" y="{y}" width="{W}" height="{24 - k * 4}" fill="{c}"/>')
    g.append(f'<text x="{W - 40}" y="{H - 24}" text-anchor="end" font-size="22" font-weight="600" letter-spacing="6" fill="{INK}" opacity="0.6">SUNRISE No. 4471</text>')
    return (W, H, svg_page(W, H, "".join(g)))


# ------------------------------------------------------------------ the conference screen before you join
def callwaiting():
    return (1024, 576, f'''<div style="position:relative;width:1024px;height:576px;background:#1b2a30;font-family:Poppins,Inter,sans-serif;overflow:hidden">
      <div style="position:absolute;left:0;right:0;top:0;height:46px;background:#24363d;color:#cfe;font:600 18px/46px Poppins;padding:0 22px">HALCYON MEET</div>
      <div style="position:absolute;left:0;right:0;top:110px;text-align:center">{mark(MINT, 120)}</div>
      <div style="position:absolute;left:0;right:0;top:230px;font:700 46px Poppins;color:#f4f7f8;text-align:center">Sync (12:30)</div>
      <div style="position:absolute;left:0;right:0;top:300px;font:500 26px Inter;color:#9fb3b9;text-align:center">Not connected. Waiting for you to join.</div>
      <div style="position:absolute;left:0;right:0;top:350px;font:500 22px Inter;color:#6f8790;text-align:center">11 participants are already here. The host's camera is off.</div>
      <div style="position:absolute;left:312px;width:400px;top:420px;height:58px;border-radius:29px;background:{MINT};font:600 24px/58px Poppins;color:{INK};text-align:center">Use the conference phone</div></div>''')


PAGES = {
    "T_Screen_CallWaiting": callwaiting,
    "T_PartyFlier": flier,
    "T_OfficeArt": officeart,
    "T_TitleKeyArt": title,
    "T_WrapBanner": banner,
    "T_CakeTop": caketop,
    "T_DirectorCard_Blank": lambda: card(""),
    "T_DirectorCard_4471": lambda: card("4471"),
    "T_OvertimeMenu": menu,
    "T_Dailies_Desk": lambda: daily("desk", "SC 1 &#183; DESK", "09:00:14:07"),
    "T_Dailies_Conference": lambda: daily("conference", "SC 2 &#183; THE CALL", "10:14:00:12"),
    "T_Dailies_CorridorB": lambda: daily("corridor", "SC 3 &#183; CORRIDOR B", "14:14:00:00"),
    "T_Dailies_Ward": lambda: daily("ward", "SC 6 &#183; THE WARD", "02:14:00:00"),
    "T_PenthouseSkyline": skyline,
}

if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    with sync_playwright() as p:
        b = p.chromium.launch(executable_path="/usr/bin/google-chrome")
        for name, fn in PAGES.items():
            if ONLY and name not in ONLY:
                continue
            w, h, html = fn()
            pg = b.new_page(viewport={"width": w, "height": h})
            pg.set_content(f'<html><head><style>*{{margin:0;padding:0}} body{{background:transparent}}</style></head><body>{html}</body></html>')
            pg.wait_for_timeout(150)
            pg.screenshot(path=os.path.join(OUT, name + ".png"), omit_background=(name == "T_CakeTop"))
            pg.close()
            print("wrote", name)
        b.close()
