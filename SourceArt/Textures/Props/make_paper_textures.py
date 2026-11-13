"""
make_paper_textures.py: call sheet + Act 3 script-page (patient chart) textures.
Glyph clues change every run, so pages leave an empty GLYPH STRIP box; the game
draws the run's glyphs into it (a widget or decal on top).
Real pages have a coffee ring (the custodian's tip); fake pages are suspiciously clean.
Usage: python make_paper_textures.py   (needs playwright + Chrome)
"""
import os
from playwright.sync_api import sync_playwright

HERE = os.path.dirname(os.path.abspath(__file__))

CSS = """@import url('https://fonts.googleapis.com/css2?family=Courier+Prime:wght@400;700&family=Permanent+Marker&display=swap');
body{margin:0} .pg{width:1024px;height:1448px;position:relative;overflow:hidden;font-family:'Courier Prime','Courier New',monospace;color:#1b1b1b;
background:radial-gradient(ellipse at 30% 20%,#fbfaf5,#efece2 70%,#e4dfd1);}
.pg:before{content:"";position:absolute;inset:0;background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='300' height='300'%3E%3Cfilter id='n'%3E%3CfeTurbulence type='fractalNoise' baseFrequency='.7' numOctaves='3'/%3E%3C/filter%3E%3Crect width='300' height='300' filter='url(%23n)' opacity='.08'/%3E%3C/svg%3E")}
.ring{position:absolute;width:250px;height:250px;border-radius:50%;border:16px solid rgba(110,70,30,.28);box-shadow:inset 0 0 18px rgba(110,70,30,.25),0 0 6px rgba(110,70,30,.2);transform:rotate(12deg)}
.strip{border:3px dashed #9a8a6a;height:90px;margin:26px 0;display:flex;align-items:center;justify-content:center;color:#9a8a6a;font-size:18px;letter-spacing:6px}
.marker{font-family:'Permanent Marker',cursive;color:#b3261e}
"""

CALLSHEET = """<div class="pg" style="padding:70px 80px;box-sizing:border-box">
<div style="display:flex;justify-content:space-between;align-items:flex-end;border-bottom:4px solid #1b1b1b;padding-bottom:14px">
 <div><div style="font-size:22px;letter-spacing:6px">HALCYON CONTINUITY</div><div style="font-size:64px;font-weight:700;letter-spacing:4px">CALL SHEET</div></div>
 <div style="text-align:right;font-size:22px;line-height:1.5">DAY 1 OF 1<br>DIRECTOR: C<br>TAKE: <span class="marker" style="font-size:38px"><s>1</s> <s>2</s> <s>3</s> <s>4</s> <s>5</s> <s>6</s> 7</span></div></div>
<div style="font-size:24px;line-height:1.55;margin-top:30px">
<b>CAST</b><br>&nbsp; EMPLOYEE 4471 ............. THEMSELF<br>&nbsp; COWORKERS ................. AS CAST<br>&nbsp; SECURITY .................. AS CAST<br>&nbsp; CUSTODIAN ................. AS CAST<br>&nbsp; THE DARK FIGURE ........... STUNT<br><br>
<b>SCHEDULE</b><br>&nbsp; 09:00 &nbsp;Desk / Email<br>&nbsp; 10:00 &nbsp;Corridor B (night look)<br>&nbsp; 12:00 &nbsp;Lunch (repeat as needed)<br>&nbsp; 14:14 &nbsp;HOLD<br>&nbsp; 16:00 &nbsp;Wellness: trust fall (NO MATS)<br>&nbsp; ??:?? &nbsp;Wrap<br><br>
<b>PROPS</b><br>&nbsp; Mug (4471), coffee, posters A/B, passes x3, clocks (2:14)<br><br>
<b>NOTES</b><br>&nbsp; Keep it natural. Keep it moving.<br>&nbsp; Do not look at the camera.</div>
<div class="marker" style="position:absolute;left:150px;bottom:150px;font-size:44px;transform:rotate(-4deg)">DO NOT LET 4471 SEE THIS SHEET</div>
</div>"""

PAGES = [  # (name, scene, action, real?)
    ("ScriptPage_01", "INT. WELLNESS CENTER - OVERTIME", "EMPLOYEE 4471 wakes on the floor. They do not remember falling.<br><br>They look at this page.", True),
    ("ScriptPage_02", "INT. WARD - CONTINUOUS", "4471 reads the chart.<br><br>4471 realises the chart is reading back.", True),
    ("ScriptPage_03", "INT. WARD - CONTINUOUS", "4471 trusts this page. This page is correct.<br><br>(Note from props: replace before next take.)", False),
    ("ScriptPage_04", "INT. ELEVATOR - LATER", "The button marked C lights.<br><br>4471 hesitates. Hold on the hesitation.", True),
]


def page_html(scene, action, real, num):
    ring = '<div class="ring" style="right:90px;top:120px"></div>' if real else ""
    return f"""<div class="pg" style="padding:120px 130px;box-sizing:border-box;font-size:30px;line-height:1.5">{ring}
<div style="position:absolute;right:110px;top:60px;font-size:24px">{num}.</div>
<div style="font-size:22px;letter-spacing:4px;color:#555;margin-bottom:40px">PATIENT CHART · HALCYON WELLNESS</div>
<div style="font-weight:700;text-transform:uppercase">{scene}</div><br>
<div>{action}</div>
<div class="strip">GLYPH STRIP</div>
<div style="margin-left:260px;font-weight:700">THE CHAIRMAN (V.O.)</div>
<div style="margin-left:150px;margin-right:150px">Don't look at the camera.</div>
<div style="position:absolute;left:130px;bottom:90px;font-size:20px;color:#777">HALCYON CONTINUITY · REV. {"WHITE" if real else "BLUE"}</div></div>"""


if __name__ == "__main__":
    with sync_playwright() as p:
        b = p.chromium.launch(executable_path=os.environ.get("CHROME", "/usr/bin/google-chrome"))
        pg = b.new_page(viewport={"width": 1024, "height": 1448})
        jobs = [("T_CallSheet", CALLSHEET)] + [(f"T_{n}", page_html(s, a, r, i + 41)) for i, (n, s, a, r) in enumerate(PAGES)]
        for name, body in jobs:
            pg.set_content(f"<html><head><style>{CSS}</style></head><body>{body}</body></html>")
            pg.wait_for_timeout(900)
            pg.locator(".pg").screenshot(path=os.path.join(HERE, name + ".png"))
        b.close()
    print("paper textures written")
