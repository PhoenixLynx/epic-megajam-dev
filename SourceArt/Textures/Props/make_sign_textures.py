"""
make_sign_textures.py: TAKE 4471. Signs and the restroom mirror (all original, drawn in HTML/SVG).

  T_WetFloor        yellow A-frame face: slipping figure, CAUTION / WET FLOOR / (always)
  T_ExitSign        green EXIT with a running figure
  T_RestroomSign    restroom plaque: two figures, RESTROOM, and a tiny "4471 x" tally
  T_Mirror          what the mirror shows when you look closer (you, mint suit, a beat late)
Usage: python make_sign_textures.py   (playwright + Chrome; Poppins in ~/.fonts)
"""
import os

from playwright.sync_api import sync_playwright

HERE = os.path.dirname(os.path.abspath(__file__))

SLIP = """<svg viewBox="0 0 200 200" width="{w}" height="{h}" style="{style}">
 <g fill="#151515">
  <circle cx="122" cy="38" r="17"/>
  <path d="M112 58 L86 112 L60 120 L58 132 L94 126 L108 98 L118 132 L150 150 L156 140 L132 122 L128 88 L150 104 L172 92 L168 82 L148 88 L128 62 Z"/>
  <path d="M30 168 Q60 158 90 168 T150 168 L150 176 Q120 166 90 176 T30 176 Z"/>
  <ellipse cx="60" cy="182" rx="40" ry="6"/>
 </g></svg>"""

RUN = """<svg viewBox="0 0 200 200" width="{w}" height="{h}" style="{style}">
 <g fill="#ffffff">
  <circle cx="118" cy="32" r="16"/>
  <path d="M104 52 L72 70 L56 98 L66 104 L80 80 L96 74 L84 116 L54 150 L64 160 L100 124 L118 146 L120 186 L134 186 L132 140 L112 112 L122 84 L140 104 L166 108 L166 96 L146 92 L124 56 Z"/>
 </g></svg>"""

PAGES = {
    "T_WetFloor": (512, 1024, """
<div style="position:absolute;inset:0;background:#f2c419"></div>
<div style="position:absolute;left:28px;right:28px;top:40px;height:120px;background:#151515;color:#f2c419;
 font:800 92px Poppins;text-align:center;line-height:120px;letter-spacing:4px">CAUTION</div>
<div style="position:absolute;left:131px;top:265px;width:250px;height:250px;transform:rotate(45deg);border:18px solid #151515;background:#f2c419"></div>
""" + SLIP.format(w=230, h=230, style="position:absolute;left:141px;top:265px") + """
<div style="position:absolute;left:0;right:0;top:640px;font:800 96px/1 Poppins;color:#151515;text-align:center">WET<br>FLOOR</div>
<div style="position:absolute;left:0;right:0;top:850px;font:500 46px Poppins;color:#151515;text-align:center">(always)</div>
<div style="position:absolute;left:0;right:0;bottom:30px;font:600 24px Poppins;color:#5a4a08;text-align:center;letter-spacing:6px">HALCYON FACILITIES</div>"""),
    "T_ExitSign": (1024, 360, """
<div style="position:absolute;inset:0;background:#11a85a"></div>
""" + RUN.format(w=300, h=300, style="position:absolute;left:40px;top:30px") + """
<div style="position:absolute;left:360px;top:30px;font:800 230px/300px Poppins;color:#fff;letter-spacing:12px">EXIT</div>"""),
    "T_RestroomSign": (512, 512, """
<div style="position:absolute;inset:0;background:#eef3f4;border:16px solid #7fd4c4"></div>
<svg viewBox="0 0 200 140" width="400" height="280" style="position:absolute;left:56px;top:50px"><g fill="#2b3a40">
 <circle cx="55" cy="18" r="13"/><rect x="40" y="36" width="30" height="50" rx="8"/><rect x="42" y="84" width="11" height="52" rx="5"/><rect x="57" y="84" width="11" height="52" rx="5"/>
 <rect x="97" y="10" width="6" height="128" fill="#7fd4c4"/>
 <circle cx="145" cy="18" r="13"/><path d="M130 36 L160 36 L178 108 L112 108 Z"/><rect x="132" y="104" width="10" height="34" rx="5"/><rect x="148" y="104" width="10" height="34" rx="5"/>
</g></svg>
<div style="position:absolute;left:0;right:0;top:350px;font:700 64px Poppins;color:#2b3a40;text-align:center;letter-spacing:6px">RESTROOM</div>
<div style="position:absolute;left:0;right:0;top:440px;font:500 26px Poppins;color:#6b7c82;text-align:center">please wash your hands &middot; twice</div>"""),
    "T_Mirror": (760, 1000, """
<div style="position:absolute;inset:0;background:linear-gradient(160deg,#c9d6da,#9fb2b8 60%,#8ea2a8)"></div>
<div style="position:absolute;left:0;right:0;bottom:0;height:260px;background:linear-gradient(#b8c4c6,#a3b1b4)"></div>
<div style="position:absolute;left:520px;top:120px;width:160px;height:520px;background:rgba(255,255,255,.25)"></div>
<!-- you: mint suit, badge, a plain mannequin head -->
<div style="position:absolute;left:250px;top:520px;width:260px;height:480px;background:#7fd4c4;border-radius:110px 110px 0 0"></div>
<div style="position:absolute;left:350px;top:520px;width:60px;height:200px;background:#e9eef0;clip-path:polygon(0 0,100% 0,50% 100%)"></div>
<div style="position:absolute;left:420px;top:640px;width:56px;height:70px;background:#fff;border:3px solid #2b3a40;font:700 14px Poppins;color:#2b3a40;text-align:center;line-height:70px">4471</div>
<div style="position:absolute;left:345px;top:440px;width:70px;height:100px;background:#e6e1d8"></div>
<div style="position:absolute;left:290px;top:210px;width:180px;height:250px;border-radius:50%;background:#e6e1d8"></div>
<!-- the reflection is a beat late: a faint second outline, offset -->
<div style="position:absolute;left:304px;top:206px;width:180px;height:250px;border-radius:50%;border:3px solid rgba(43,58,64,.25)"></div>
<div style="position:absolute;inset:0;background:radial-gradient(circle at 50% 45%,transparent 55%,rgba(30,40,45,.35))"></div>
<div style="position:absolute;left:36px;bottom:30px;font:500 22px Poppins;color:rgba(43,58,64,.55)">(fingerprints on the glass: on the inside)</div>"""),
}

if __name__ == "__main__":
    with sync_playwright() as p:
        b = p.chromium.launch(executable_path=os.environ.get("CHROME", "/usr/bin/google-chrome"))
        for name, (w, h, body) in PAGES.items():
            pg = b.new_page(viewport={"width": w, "height": h})
            pg.set_content(f"<html><body style='margin:0;width:{w}px;height:{h}px;position:relative;overflow:hidden'>{body}</body></html>")
            pg.wait_for_timeout(300)
            pg.screenshot(path=os.path.join(HERE, name + ".png"))
            print("wrote", name)
        b.close()
