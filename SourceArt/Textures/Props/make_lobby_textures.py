"""
make_lobby_textures.py: TAKE 4471. Lobby, elevator, security desk, lunch (Saturday batch). Original HTML art.

  T_LunchSpecials     lunch board on the kitchen island (Act 2)
  T_SignInSheet       the reception clipboard (4471 every day... and 4470)
  T_Screen_CCTV       security desk monitors: four camera feeds, one of them is behind you
  T_Screen_Promo      lobby screen: HALCYON promo
  T_Magazine          HALCYON QUARTERLY on the coffee table
  T_Rug               lobby rug
  T_ElevatorPanel     inside the elevator: 1 2 3 C (and B4)
  T_Sign_ElevatorFire / T_Sign_Emergency / T_Sign_Wait   signs
Usage: python make_lobby_textures.py   (playwright + Chrome; Poppins, Caveat in ~/.fonts)
"""
import os

from playwright.sync_api import sync_playwright

HERE = os.path.dirname(os.path.abspath(__file__))
INK, MINT, PAPER = "#2b3a40", "#7fd4c4", "#f4f7f8"
LOGO = f"""<svg viewBox="0 0 100 60" width="{{w}}" height="{{h}}"><g fill="none" stroke="{MINT}" stroke-width="5">
<path d="M10 50 A40 40 0 0 1 90 50"/></g><g stroke="{MINT}" stroke-width="4">
<line x1="50" y1="2" x2="50" y2="12"/><line x1="20" y1="14" x2="26" y2="21"/><line x1="80" y1="14" x2="74" y2="21"/></g></svg>"""


def sign(title, lines, colour="#c62828", fg="#fff"):
    return (512, 512, f"""<div style="position:absolute;inset:0;background:{PAPER};border:14px solid {colour}"></div>
<div style="position:absolute;left:14px;right:14px;top:14px;height:110px;background:{colour};color:{fg};font:800 52px/110px Poppins;text-align:center;letter-spacing:3px">{title}</div>
<div style="position:absolute;left:40px;right:40px;top:150px;font:600 34px/1.25 Poppins;color:{INK};text-align:center">{lines}</div>""")


PAGES = {
    "T_LunchSpecials": (600, 840, f"""
<div style="position:absolute;inset:0;background:#24302f;border:18px solid #8a6a46"></div>
<div style="position:absolute;left:0;right:0;top:40px;text-align:center">{LOGO.format(w=150, h=90)}</div>
<div style="position:absolute;left:0;right:0;top:140px;font:800 50px Poppins;color:#f4f7f8;text-align:center;letter-spacing:4px">LUNCH IS SERVED</div>
<div style="position:absolute;left:0;right:0;top:205px;font:600 30px Poppins;color:{MINT};text-align:center;letter-spacing:6px">TODAY'S SPECIALS</div>
<div style="position:absolute;left:60px;right:50px;top:270px;font:44px/1.25 Caveat;color:#f4f7f8">
 &bull; Pizza (delivered 12:00)<br>&bull; Turkey sandwich<br>&nbsp;&nbsp;<span style="font-size:32px">(you asked for turkey)</span><br>
 &bull; Salad: every leaf the same size<br>&bull; Donut: do not read the sprinkles<br>&bull; Soup of the day: yesterday's</div>
<div style="position:absolute;left:0;right:0;bottom:44px;font:600 22px Poppins;color:#9fb2b8;text-align:center">Lunch is served. Lunch is served.</div>"""),
    "T_SignInSheet": (700, 960, f"""
<div style="position:absolute;inset:0;background:#fbfbf7"></div>
<div style="position:absolute;left:0;right:0;top:0;height:90px;background:{MINT};font:700 40px/90px Poppins;color:#fff;text-align:center;letter-spacing:4px">VISITOR SIGN-IN</div>
<div style="position:absolute;left:30px;right:30px;top:110px;font:600 22px Poppins;color:{INK};display:flex;justify-content:space-between"><span>DATE</span><span>NAME / BADGE</span><span>IN</span><span>OUT</span></div>
<div style="position:absolute;left:30px;right:30px;top:146px;height:780px;background:repeating-linear-gradient(#fbfbf7 0 58px,#c9d3d6 58px 60px)"></div>
<div style="position:absolute;left:36px;top:150px;font:40px/60px Caveat;color:#1f2a6e;white-space:pre">today      4471          9:00
today      4471          9:00
today      4471          9:00      ----
today      4471          9:00
today      4470          9:00      wrapped
today      4471          9:00
today      4471          9:00
today      4471          9:00
today      4471          9:00
today      <span style="color:#b02a2a">sign here &rarr;</span></div>"""),
    "T_Screen_CCTV": (1024, 582, """
<div style="position:absolute;inset:0;background:#05080a"></div>
""" + "".join(f"""<div style="position:absolute;left:{8 + (i % 2) * 508}px;top:{8 + (i // 2) * 287}px;width:500px;height:279px;
 background:linear-gradient({c});overflow:hidden">
 <div style="position:absolute;left:10px;top:8px;font:600 18px monospace;color:#cfe">CAM {i + 1} &middot; {n}</div>
 <div style="position:absolute;right:10px;top:8px;font:600 18px monospace;color:#f55">&#9679; REC</div>
 <div style="position:absolute;right:10px;bottom:8px;font:600 16px monospace;color:#cfe">14:14:14</div>
 {extra}</div>""" for i, (n, c, extra) in enumerate([
    ("LOBBY", "#3a4a4e,#22302f", '<div style="position:absolute;left:200px;top:110px;width:40px;height:120px;background:#7fd4c4;border-radius:20px 20px 4px 4px"></div><div style="position:absolute;left:204px;top:80px;width:32px;height:36px;border-radius:50%;background:#ddd"></div>'),
    ("CORRIDOR B", "#2a2f30,#151a1b", '<div style="position:absolute;left:220px;top:60px;width:60px;height:180px;background:linear-gradient(#000,#111);border-radius:30px 30px 0 0;opacity:.85"></div>'),
    ("OPEN OFFICE", "#3d4a4c,#263132", '<div style="position:absolute;left:60px;top:150px;width:380px;height:20px;background:#556"></div>'),
    ("DESK 4471", "#40504f,#2a3534", '<div style="position:absolute;left:210px;top:70px;width:80px;height:200px;background:#7fd4c4;border-radius:40px 40px 6px 6px"></div><div style="position:absolute;left:222px;top:30px;width:56px;height:62px;border-radius:50%;background:#ddd"></div><div style="position:absolute;left:150px;top:240px;font:600 16px monospace;color:#ff0">YOU (FROM BEHIND)</div>')])) + """
<div style="position:absolute;inset:0;background:repeating-linear-gradient(transparent 0 3px,rgba(0,0,0,.25) 3px 4px)"></div>"""),
    "T_Screen_Promo": (1024, 582, f"""
<div style="position:absolute;inset:0;background:linear-gradient(135deg,#eaf6f3,#bfe7df)"></div>
<div style="position:absolute;left:70px;top:70px">{LOGO.format(w=200, h=120)}</div>
<div style="position:absolute;left:70px;top:210px;font:800 76px Poppins;color:{INK}">HALCYON</div>
<div style="position:absolute;left:74px;top:310px;font:500 36px Poppins;color:{INK}">Your energy. Our future.</div>
<div style="position:absolute;left:74px;top:380px;font:500 24px Poppins;color:#5a6c72">Now hiring: 4472.</div>
<div style="position:absolute;right:80px;top:120px;width:300px;height:340px;border-radius:150px 150px 20px 20px;background:{MINT}"></div>
<div style="position:absolute;right:155px;top:60px;width:150px;height:170px;border-radius:50%;background:#e6e1d8"></div>"""),
    "T_Magazine": (700, 900, f"""
<div style="position:absolute;inset:0;background:{MINT}"></div>
<div style="position:absolute;left:40px;top:30px;font:800 74px Poppins;color:#fff;letter-spacing:2px">HALCYON</div>
<div style="position:absolute;left:44px;top:120px;font:600 28px Poppins;color:{INK};letter-spacing:8px">QUARTERLY</div>
<div style="position:absolute;left:150px;top:260px;width:400px;height:500px;border-radius:200px 200px 30px 30px;background:#5fb5a6"></div>
<div style="position:absolute;left:250px;top:180px;width:200px;height:230px;border-radius:50%;background:#e6e1d8"></div>
<div style="position:absolute;left:40px;top:760px;font:800 42px Poppins;color:#fff">Employee of the Take:</div>
<div style="position:absolute;left:40px;top:812px;font:800 48px Poppins;color:{INK}">4471</div>
<div style="position:absolute;right:40px;top:822px;font:600 22px Poppins;color:{INK}">ISSUE DATE: TOMORROW</div>"""),
    "T_Rug": (1024, 640, f"""
<div style="position:absolute;inset:0;background:#cfe9e3;border:40px solid #9fd2c7"></div>
<div style="position:absolute;inset:70px;border:6px solid #ffffff"></div>
<div style="position:absolute;left:0;right:0;top:200px;text-align:center">{LOGO.format(w=300, h=180)}</div>"""),
    "T_ElevatorPanel": (400, 800, """
<div style="position:absolute;inset:0;background:linear-gradient(90deg,#9aa4a8,#d5dadc,#9aa4a8)"></div>
<div style="position:absolute;left:40px;right:40px;top:40px;height:90px;background:#05080a;color:#ff4d3d;font:700 60px/90px monospace;text-align:center">1</div>
""" + "".join(f"""<div style="position:absolute;left:{90 if k % 2 == 0 else 220}px;top:{200 + (k // 2) * 140}px;width:90px;height:90px;border-radius:50%;
 background:radial-gradient(#f4f4f4,#b9c0c2);border:5px solid #6d777a;font:700 40px/90px Poppins;color:#2b3a40;text-align:center">{b}</div>""" for k, b in enumerate(["1", "2", "3", "C", "B4", "&#9888;"])) + """
<div style="position:absolute;left:0;right:0;bottom:60px;font:600 20px Poppins;color:#2b3a40;text-align:center">C: AUTHORISED PERSONNEL ONLY</div>"""),
    "T_Sign_ElevatorFire": sign("IN CASE OF FIRE", "DO NOT USE THE ELEVATOR.<br><br>IN CASE OF RETAKE,<br>REMAIN WHERE YOU ARE."),
    "T_Sign_Emergency": sign("EMERGENCY", "There is no emergency.<br><br>Remain calm.<br>Remain.", colour="#11a85a"),
    "T_Sign_Wait": sign("RECEPTION", "Please wait to be called.<br><br>You will be called.", colour="#4fa596"),
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
