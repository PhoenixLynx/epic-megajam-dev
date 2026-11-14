"""
make_screen_textures.py: TAKE 4471. Images for the office monitors (1024 x 582, the screen's 0.59 : 0.335 shape).

The monitors flip between these during Play, so the office looks like people are working:
  T_Screen_Inbox, T_Screen_Sheet, T_Screen_Code, T_Screen_Chart, T_Screen_Call, T_Screen_Saver, T_Screen_Login
and at high Fear:
  T_Screen_Fear (LOOK CLOSER), T_Screen_Glyphs (a field of glyphs)
Usage: python make_screen_textures.py   (needs playwright + Chrome; fonts Inter/Poppins in ~/.fonts)
"""
import os
import random

from playwright.sync_api import sync_playwright

HERE = os.path.dirname(os.path.abspath(__file__))
W, H = 1024, 582
rnd = random.Random(4471)

BASE = """<html><head><style>
* { margin: 0; padding: 0; box-sizing: border-box; }
body { width: %dpx; height: %dpx; overflow: hidden; font-family: Inter, sans-serif; }
.bar { height: 34px; background: #7fd4c4; color: #fff; font: 600 15px Poppins; padding: 8px 16px; display: flex; justify-content: space-between; }
.pane { position: absolute; top: 34px; bottom: 0; }
.row { height: 34px; border-bottom: 1px solid #dbe5e8; padding: 6px 12px; font-size: 13px; color: #33454d; white-space: nowrap; overflow: hidden; }
.mono { font-family: 'DejaVu Sans Mono', monospace; }
</style></head><body>%s</body></html>"""


def page(body, bg="#f6f9fa"):
    return BASE % (W, H, f'<div style="position:absolute;inset:0;background:{bg}">{body}</div>')


def inbox():
    subjects = ["Re: Q4 energy targets", "Lunch is served", "Places, everyone.", "Fwd: Fwd: smile!", "Your badge photo",
                "Compliance reminder", "Re: the clocks", "Team building (mandatory)", "Re: Re: Corridor B",
                "Printer jam (again)", "Who took my yogurt", "Quarterly morale survey", "Invite: Sync (12:30)"]
    rows = "".join(f'<div class="row"><b>{rnd.choice(["People Team","Calendar","IT","C","Facilities","Morale Bot"])}</b>'
                   f'<br>{s}</div>' for s in subjects)
    body = ('<div class="bar"><span>HALCYON MAIL</span><span>9:41</span></div>'
            f'<div class="pane" style="left:0;width:320px;background:#eef4f6">{rows}</div>'
            '<div class="pane" style="left:320px;right:0;padding:24px 30px;color:#1b2a30">'
            '<div style="font:600 24px Poppins">Re: Q4 energy targets</div><div style="margin:8px 0 18px;color:#6a7d84">From: The Chairman</div>'
            + "".join(f'<div style="height:12px;margin:9px 0;background:#d9e4e7;width:{rnd.randint(55,98)}%"></div>' for _ in range(14))
            + '</div>')
    return page(body)


def sheet():
    cols = "ABCDEFGHIJ"
    head = "".join(f'<td style="background:#e4eef1;font-weight:600;text-align:center">{c}</td>' for c in cols)
    rows = ""
    for r in range(1, 19):
        cells = "".join(f'<td>{rnd.choice(["", f"{rnd.randint(10,9999):,}", f"{rnd.random()*100:.1f}%", "4471", "OK", "OK", "—"])}</td>'
                        for _ in cols)
        rows += f'<tr><td style="background:#e4eef1;text-align:center">{r}</td>{cells}</tr>'
    body = ('<div class="bar"><span>Q4_ENERGY_TARGETS_FINAL_final(3).xlsx</span><span>Saved</span></div>'
            '<div class="pane" style="left:0;right:0"><table style="border-collapse:collapse;width:100%;font-size:12px;color:#33454d">'
            f'<tr><td></td>{head}</tr>{rows}</table></div>')
    body = body.replace("<td", '<td style="border:1px solid #dbe5e8;height:28px;padding:0 6px"', 0)
    return page(body.replace("<td>", '<td style="border:1px solid #dbe5e8;height:28px;padding:0 6px">'))


def code():
    words = ["soul", "key", "fragment", "take", "loop", "xor", "signature", "integrity", "consent", "director"]
    lines = []
    for i in range(26):
        ind = "&nbsp;" * (4 * rnd.randint(0, 3))
        w = rnd.sample(words, 3)
        lines.append(f'<span style="color:#5d7a83">{i+120:>4}</span>  {ind}'
                     f'<span style="color:#7fd4c4">{rnd.choice(["if","for","return","let","const","assert"])}</span> '
                     f'{w[0]}_{w[1]}(<span style="color:#f1c27d">{rnd.randint(0,255)}</span>) '
                     f'<span style="color:#e8a0a0">^</span> {w[2]};')
    body = ('<div class="bar" style="background:#26363c"><span>integrity_check.hcy</span><span>build 4471</span></div>'
            '<div class="pane mono" style="left:0;right:0;padding:12px 16px;font-size:14px;line-height:19px;color:#d6e2e5">'
            + "<br>".join(lines) + '</div>')
    return page(body, bg="#16232a")


def chart():
    bars = "".join(f'<div style="display:inline-block;width:46px;margin:0 9px;height:{h}px;background:#7fd4c4;vertical-align:bottom"></div>'
                   for h in [rnd.randint(80, 330) for _ in range(12)])
    body = ('<div class="bar"><span>MORALE DASHBOARD</span><span>LIVE</span></div>'
            '<div class="pane" style="left:0;right:0;padding:24px 34px">'
            '<div style="display:flex;gap:18px;margin-bottom:22px">'
            + "".join(f'<div style="flex:1;background:#fff;border:1px solid #dbe5e8;padding:12px 16px">'
                      f'<div style="font-size:12px;color:#6a7d84">{k}</div><div style="font:600 28px Poppins;color:#1b2a30">{v}</div></div>'
                      for k, v in (("Energy", "98%"), ("Smiles / hr", "4,471"), ("Consent", "100%"), ("Retakes", "0")))
            + f'</div><div style="height:350px;border-bottom:2px solid #9fb3b9">{bars}</div></div>')
    return page(body)


def call():
    tiles = ""
    for i in range(6):
        name = ["C (organizer)", "4471", "Dana", "Ops", "People Team", "Guest"][i]
        cam = '<div style="color:#9fb3b9;font-size:13px">camera off</div>' if i != 1 else \
              '<div style="width:70px;height:70px;border-radius:50%;background:#7fd4c4;margin:0 auto"></div>'
        tiles += (f'<div style="background:#22323a;border-radius:8px;display:flex;flex-direction:column;align-items:center;'
                  f'justify-content:center;gap:10px">{cam}<div style="color:#e6eef0;font-size:14px">{name}</div></div>')
    body = ('<div class="bar" style="background:#26363c"><span>SYNC (12:30)</span><span>&#9679; REC</span></div>'
            '<div class="pane" style="left:0;right:0;padding:14px;display:grid;grid-template-columns:repeat(3,1fr);'
            f'grid-template-rows:repeat(2,1fr);gap:12px">{tiles}</div>')
    return page(body, bg="#16232a")


def saver():
    body = ('<div style="position:absolute;inset:0;display:flex;flex-direction:column;align-items:center;justify-content:center;'
            'background:radial-gradient(circle at 40% 35%,#9fe3d6,#4fa898)">'
            '<div style="font:700 86px Poppins;color:#fff;letter-spacing:10px">HALCYON</div>'
            '<div style="font:500 20px Inter;color:#eafaf6;margin-top:10px">You are exactly where you need to be.</div></div>')
    return page(body)


def login():
    body = ('<div style="position:absolute;inset:0;display:flex;flex-direction:column;align-items:center;justify-content:center;background:#eef4f6">'
            '<div style="width:96px;height:96px;border-radius:50%;background:#7fd4c4"></div>'
            '<div style="font:600 26px Poppins;color:#1b2a30;margin-top:16px">Employee 4471</div>'
            '<div style="width:300px;height:40px;border:1px solid #9fb3b9;background:#fff;margin-top:16px;padding:9px 12px;color:#9fb3b9">&#9679;&#9679;&#9679;&#9679;&#9679;&#9679;&#9679;</div>'
            '<div style="font:500 14px Inter;color:#6a7d84;margin-top:12px">Press E to sign in</div></div>')
    return page(body)


def fear():
    noise = "".join(f'<div style="position:absolute;left:0;right:0;top:{rnd.randint(0,H)}px;height:{rnd.randint(1,6)}px;'
                    f'background:rgba(255,255,255,{rnd.random()*0.25:.2f})"></div>' for _ in range(40))
    body = (f'{noise}<div style="position:absolute;inset:0;display:flex;align-items:center;justify-content:center;'
            'font:800 92px Poppins;color:#e8eef0;letter-spacing:6px;text-shadow:6px 0 #c44,-6px 0 #4cc">LOOK CLOSER</div>')
    return page(body, bg="#0b1013")


def glyphs():
    marks = "".join(f'<span style="display:inline-block;width:38px;height:38px;margin:6px;'
                    f'border:3px solid #7fd4c4;{"background:#7fd4c4" if rnd.random() < 0.5 else ""}"></span>'
                    for _ in range(140))
    body = f'<div style="position:absolute;inset:0;padding:18px;line-height:0">{marks}</div>'
    return page(body, bg="#0b1013")


SCREENS = {"Inbox": inbox, "Sheet": sheet, "Code": code, "Chart": chart, "Call": call, "Saver": saver,
           "Login": login, "Fear": fear, "Glyphs": glyphs}

if __name__ == "__main__":
    with sync_playwright() as p:
        b = p.chromium.launch(executable_path=os.environ.get("CHROME", "/usr/bin/google-chrome"))
        pg = b.new_page(viewport={"width": W, "height": H})
        for name, fn in SCREENS.items():
            pg.set_content(fn())
            pg.wait_for_timeout(150)
            out = os.path.join(HERE, f"T_Screen_{name}.png")
            pg.screenshot(path=out)
            print("wrote", os.path.basename(out))
        b.close()
