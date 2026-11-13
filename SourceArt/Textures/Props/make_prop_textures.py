"""
make_prop_textures.py: TAKE 4471 poster + Chairman portrait textures.
Renders HTML/SVG to PNG with headless Chrome (playwright). Every poster has a
_Changed twin with one subtle difference, for the Corridor B loop.
Usage: python make_prop_textures.py
"""
import os
from playwright.sync_api import sync_playwright

HERE = os.path.dirname(os.path.abspath(__file__))
LOGO = ('<svg viewBox="0 0 100 60" width="{w}"><path d="M15 45 A35 35 0 0 1 85 45" fill="none" stroke="{c}" stroke-width="7"/>'
        '<line x1="5" y1="52" x2="95" y2="52" stroke="{c}" stroke-width="5"/><line x1="25" y1="59" x2="75" y2="59" stroke="{c}" stroke-width="4"/></svg>')

POSTERS = [  # (name, background, ink, headline, changed headline, subline, art)
    ("Energy", "#7fd3c4", "#12313a", "YOUR ENERGY IS OUR GREATEST ASSET", "YOUR ENERGY IS OUR ASSET", "Give a little more today.", "sun"),
    ("Present", "#a7c7f2", "#1b2a44", "BE PRESENT.<br>BE PRESENT.<br>BE PRESENT.", "BE PRESENT.<br>BE PRESENT.<br>BE PRESENT.<br>BE PRESENT.", "Mindfulness is mandatory.", "eye"),
    ("Mark", "#f2d7a7", "#3a2a12", "STAY ON<br>YOUR MARK", "STAY ON<br>YOUR MARK,<br>4471", "Great teams hit their marks.", "mark"),
    ("Smile", "#f2b8b8", "#401c1c", "SMILE.<br>EVERY DAY IS A<br>PERFORMANCE REVIEW", "SMILE.<br>EVERY TAKE IS A<br>PERFORMANCE REVIEW", "Your face is part of the brand.", "smile"),
    ("CorridorB", "#d9dee3", "#20272e", "THERE IS NO<br>CORRIDOR B", "THERE WAS NO<br>CORRIDOR B", "Please continue walking.", "arrow"),
    ("Wellness", "#c9e7c2", "#1d3a17", "WELLNESS<br>IS MANDATORY", "WELLNESS<br>WAS MANDATORY", "The Wellness Center is always open.", "cross"),
]

ART = {
    "sun": '<circle cx="200" cy="200" r="120" fill="{ink}" opacity=".12"/><circle cx="200" cy="200" r="80" fill="{ink}" opacity=".18"/><circle cx="200" cy="200" r="40" fill="{ink}"/>',
    "eye": '<path d="M40 200 Q200 60 360 200 Q200 340 40 200Z" fill="none" stroke="{ink}" stroke-width="14"/><circle cx="200" cy="200" r="50" fill="{ink}"/>',
    "mark": '<line x1="90" y1="110" x2="310" y2="290" stroke="{ink}" stroke-width="36" stroke-linecap="round"/><line x1="310" y1="110" x2="90" y2="290" stroke="{ink}" stroke-width="36" stroke-linecap="round"/>',
    "smile": '<path d="M90 170 Q200 300 310 170" fill="none" stroke="{ink}" stroke-width="22" stroke-linecap="round"/><circle cx="140" cy="110" r="16" fill="{ink}"/><circle cx="260" cy="110" r="16" fill="{ink}"/>',
    "arrow": '<path d="M200 60 V330 M120 250 L200 330 L280 250" fill="none" stroke="{ink}" stroke-width="26" stroke-linecap="round" stroke-linejoin="round"/>',
    "cross": '<path d="M160 70 H240 V160 H330 V240 H240 V330 H160 V240 H70 V160 H160Z" fill="{ink}"/>',
}


def poster_html(bg, ink, headline, sub, art):
    return f'''<html><head><style>@import url('https://fonts.googleapis.com/css2?family=Inter:wght@500;800&display=swap');
body{{margin:0}} .p{{width:1024px;height:1536px;background:{bg};position:relative;font-family:Inter,Arial,sans-serif;color:{ink};overflow:hidden}}
.p:after{{content:"";position:absolute;inset:0;box-shadow:inset 0 0 160px rgba(0,0,0,.18)}}
.art{{position:absolute;left:312px;top:170px}} h1{{position:absolute;left:90px;right:90px;top:690px;margin:0;font-size:92px;line-height:1.02;font-weight:800;letter-spacing:2px}}
.sub{{position:absolute;left:90px;bottom:200px;font-size:38px;font-weight:500;opacity:.8}}
.logo{{position:absolute;left:90px;bottom:80px;display:flex;align-items:center;gap:22px;font-weight:800;letter-spacing:10px;font-size:30px}}
</style></head><body><div class="p"><svg class="art" width="400" height="400" viewBox="0 0 400 400">{ART[art].format(ink=ink)}</svg>
<h1>{headline}</h1><div class="sub">{sub}</div><div class="logo">{LOGO.format(w=110, c=ink)}HALCYON</div></div></body></html>'''


PORTRAIT = '''<html><head><style>@import url('https://fonts.googleapis.com/css2?family=Cormorant+Garamond:wght@600&display=swap');body{margin:0;background:#000}</style></head><body>
<svg width="1024" height="1366" viewBox="0 0 1024 1366" xmlns="http://www.w3.org/2000/svg">
<defs><linearGradient id="gold" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#e9c97a"/><stop offset=".5" stop-color="#9c7a35"/><stop offset="1" stop-color="#e2bd66"/></linearGradient>
<radialGradient id="bg" cx=".5" cy=".3" r=".8"><stop offset="0" stop-color="#2e4a42"/><stop offset="1" stop-color="#0c1714"/></radialGradient>
<linearGradient id="suit" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#1c2533"/><stop offset="1" stop-color="#0b0f16"/></linearGradient></defs>
<rect width="1024" height="1366" fill="url(#gold)"/><rect x="40" y="40" width="944" height="1286" fill="#5a4318"/><rect x="70" y="70" width="884" height="1226" fill="url(#bg)"/>
<!-- the figure: cropped at the chin by the top of the frame -->
<path d="M512 70 C450 70 420 100 412 150 L380 250 C250 290 150 360 120 520 L100 1296 H924 L904 520 C874 360 774 290 644 250 L612 150 C604 100 574 70 512 70Z" fill="url(#suit)"/>
<path d="M430 120 L512 330 L594 120 C570 100 540 90 512 90 C484 90 454 100 430 120Z" fill="#e8ecef"/>
<path d="M490 170 L534 170 L548 520 L512 580 L476 520Z" fill="#7fd3c4"/>
<path d="M470 150 L512 190 L554 150" fill="none" stroke="#c9ced3" stroke-width="6"/>
<path d="M380 250 L512 600 L420 330Z M644 250 L512 600 L604 330Z" fill="#121a26"/>
<circle cx="640" cy="420" r="20" fill="url(#gold)"/><path d="M628 428 L640 406 L652 428Z" fill="none" stroke="#5a4318" stroke-width="3"/>
<!-- folded hands -->
<path d="M380 1060 C420 1010 600 1010 644 1060 C620 1110 410 1110 380 1060Z" fill="#c7a58b"/><path d="M420 1060 C470 1040 560 1040 610 1060" stroke="#a5866f" stroke-width="6" fill="none"/>
<rect x="70" y="70" width="884" height="1226" fill="none" stroke="#000" stroke-opacity=".35" stroke-width="30"/>
<!-- plaque -->
<rect x="362" y="1196" width="300" height="80" rx="6" fill="url(#gold)"/>
<text x="512" y="1232" text-anchor="middle" font-family="Cormorant Garamond, Georgia, serif" font-size="30" fill="#3a2a0e" letter-spacing="4">THE CHAIRMAN</text>
<text x="512" y="1262" text-anchor="middle" font-family="Cormorant Garamond, Georgia, serif" font-size="20" fill="#3a2a0e" letter-spacing="3">Founder · Director · Friend</text>
</svg></body></html>'''

if __name__ == "__main__":
    with sync_playwright() as p:
        b = p.chromium.launch(executable_path=os.environ.get("CHROME", "/usr/bin/google-chrome"))
        pg = b.new_page(viewport={"width": 1024, "height": 1536})
        for name, bg, ink, head, changed, sub, art in POSTERS:
            for suffix, h in (("", head), ("_Changed", changed)):
                pg.set_content(poster_html(bg, ink, h, sub, art)); pg.wait_for_timeout(600)
                pg.locator(".p").screenshot(path=os.path.join(HERE, f"T_Poster_{name}{suffix}.png"))
        pg.set_viewport_size({"width": 1024, "height": 1366})
        pg.set_content(PORTRAIT); pg.wait_for_timeout(800)
        pg.locator("svg").screenshot(path=os.path.join(HERE, "T_ChairmanPortrait.png"))
        b.close()
    print("prop textures written")
