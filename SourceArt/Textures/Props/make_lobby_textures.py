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

PAGES.update({
    "T_KitchenBoard": (1200, 800, f"""
<div style="position:absolute;inset:0;background:#b98b5a;border:24px solid #6e4f31"></div>
<div style="position:absolute;left:60px;top:60px;width:300px;height:220px;background:#fff6a8;transform:rotate(-3deg);font:40px/1.1 Caveat;padding:18px;color:#1f2a6e">LOST: red stapler.<br>(not that one. the other red one)</div>
<div style="position:absolute;left:420px;top:50px;width:340px;height:260px;background:{PAPER};transform:rotate(2deg);font:600 26px/1.3 Poppins;padding:20px;color:{INK}">FIRE DRILL<br><span style="font-weight:400;font-size:22px">Date: you'll know.<br>Please remain calm.<br>Please remain.</span></div>
<div style="position:absolute;left:820px;top:70px;width:300px;height:250px;background:#ffd1dc;transform:rotate(-2deg);font:46px/1.05 Caveat;padding:18px;color:#7a1c3c">Cake in the fridge<br>for 4471's birthday!!<br><span style="font-size:32px">(again)</span></div>
<div style="position:absolute;left:80px;top:360px;width:420px;height:330px;background:{MINT};transform:rotate(1.5deg);padding:24px;color:#fff;font:800 40px/1.1 Poppins">MORALE IS<br>MANDATORY<div style="font:500 22px Poppins;margin-top:16px">Attendance at Fun is tracked.<br>Fun has been scheduled for 2:14.</div></div>
<div style="position:absolute;left:560px;top:380px;width:280px;height:300px;background:{PAPER};transform:rotate(-4deg);font:40px/1.1 Caveat;padding:18px;color:#1f2a6e">Whoever keeps<br>labelling my mug<br>"4470" -<br>it's not funny.</div>
<div style="position:absolute;left:890px;top:400px;width:240px;height:260px;background:#cfe9e3;transform:rotate(3deg);font:600 24px/1.3 Poppins;padding:18px;color:{INK}">CLEAN THE<br>MICROWAVE<br><span style="font-weight:400">It remembers.</span></div>
""" + "".join(f'<div style="position:absolute;left:{x}px;top:{y}px;width:22px;height:22px;border-radius:50%;background:#c62828;box-shadow:0 2px 2px rgba(0,0,0,.4)"></div>' for x, y in ((200, 64), (580, 56), (960, 76), (280, 366), (690, 388), (1000, 404)))),
    "T_ScriptNotes": (700, 960, """
<div style="position:absolute;inset:0;background:#fbfbf7"></div>
<div style="position:absolute;left:70px;right:60px;top:60px;font:24px/1.45 'Courier New',monospace;color:#1b1b1b;white-space:pre-wrap">INT. HALCYON LOBBY - MORNING

The RECEPTIONIST looks up. Warm. Too warm.

          RECEPTIONIST
    Welcome back.
          (beat)
    Oh -- first day. Of course.
    First day.

EMPLOYEE 4471 hesitates.

          RECEPTIONIST
    Your badge is on the desk.
    It's always on the desk.</div>
<div style="position:absolute;left:300px;top:700px;width:340px;font:52px/1.05 Caveat;color:#b02a2a;transform:rotate(-6deg)">smile with<br>the eyes.<br>AGAIN. - C</div>
<div style="position:absolute;left:40px;top:40px;font:600 18px Poppins;color:#999">p. 1 of 1 (take 4471)</div>"""),
    "T_Brochure": (600, 860, f"""
<div style="position:absolute;inset:0;background:{PAPER}"></div>
<div style="position:absolute;left:0;right:0;top:0;height:330px;background:linear-gradient(135deg,{MINT},#4fa596)"></div>
<div style="position:absolute;left:40px;top:40px">{LOGO.format(w=140, h=84)}</div>
<div style="position:absolute;left:40px;top:150px;font:800 54px/1.05 Poppins;color:#fff">Welcome to<br>HALCYON</div>
<div style="position:absolute;left:40px;top:360px;right:40px;font:600 30px/1.3 Poppins;color:{INK}">A guide for new employees<br><span style="font-weight:400;color:#5a6c72">(and returning ones)</span></div>
<div style="position:absolute;left:40px;top:500px;right:40px;font:400 24px/1.5 Poppins;color:{INK}">&bull; You are exactly where you need to be.<br>&bull; Lunch is served at 12:00.<br>&bull; The clocks are correct.<br>&bull; There is no Corridor B.</div>
<div style="position:absolute;left:0;right:0;bottom:30px;font:500 18px Poppins;color:#9fb2b8;text-align:center">HALCYON &middot; Your energy. Our future.</div>"""),
})

PAGES.update({f"T_Printout_{i + 1}": (600, 860, f"""<div style="position:absolute;inset:0;background:#fdfdfb"></div>
<div style="position:absolute;left:50px;top:50px">{LOGO.format(w=90, h=54)}</div>
<div style="position:absolute;left:50px;right:50px;top:140px;font:700 34px/1.25 Poppins;color:{INK}">{t}</div>
<div style="position:absolute;left:50px;right:50px;top:300px;font:400 24px/1.6 Poppins;color:#45565c">{b}</div>""")
    for i, (t, b) in enumerate([
        ("MEMO: Continuity is everyone's responsibility.", "Please do not move props between takes.<br>If something looks different, it isn't.<br><br>- Continuity"),
        ("PRINT JOB 4470 (queued)", "Status: waiting since forever.<br>Document: final_notes_DO_NOT_READ.txt<br><br>[the rest of the page is blank, except for a row of squares]"),
        ("YOUR PERFORMANCE: 99%", "Please give the remaining 1%.<br><br>Where is it?<br>We know you have it.<br><br>- The Morale Team"),
    ])})

PAGES["T_Whiteboard"] = (1200, 600, f"""<div style="position:absolute;inset:0;background:#f7f9f9;border:18px solid #c9d1d4"></div>
<div style="position:absolute;left:60px;top:40px;font:64px/1.05 Caveat;color:#1f4fb0">Q4 ENERGY TARGETS &uarr;&uarr;&uarr;</div>
<div style="position:absolute;left:70px;top:140px;font:48px/1.3 Caveat;color:#2b2b2b">Retention: 100%<br>Morale: mandatory<br>Retakes: as needed</div>
<svg style="position:absolute;left:640px;top:120px" width="460" height="260" viewBox="0 0 460 260"><polyline fill="none" stroke="#c62828" stroke-width="6" points="10,240 90,200 160,215 240,140 300,160 380,60 450,20"/>
<line x1="10" y1="250" x2="450" y2="250" stroke="#333" stroke-width="4"/><line x1="10" y1="250" x2="10" y2="10" stroke="#333" stroke-width="4"/></svg>
<div style="position:absolute;right:60px;bottom:40px;font:40px Caveat;color:#2a8a3a">(do not erase)</div>
<div style="position:absolute;left:70px;bottom:50px;font:34px Caveat;color:#555">who keeps drawing squares on this?</div>""")

def chart(header, lines, note=""):
    return (600, 860, f"""<div style="position:absolute;inset:0;background:#fbfbf7"></div>
<div style="position:absolute;left:0;right:0;top:0;height:80px;background:#d9e6ea;font:700 26px/80px Poppins;color:{INK};padding-left:36px;letter-spacing:3px">HALCYON CARE &middot; {header}</div>
<div style="position:absolute;left:36px;right:30px;top:110px;font:22px/1.5 'Courier New',monospace;color:#1b1b1b;white-space:pre-wrap">{lines}</div>
<div style="position:absolute;left:36px;right:30px;bottom:60px;font:40px/1.1 Caveat;color:#b02a2a;transform:rotate(-3deg)">{note}</div>""")


PAGES.update({
    "T_Chart_4471": chart("PATIENT 4471", "ADMITTED:  today (again)\nCONDITION: looked closer\nTREATMENT: retake\n\nNOTES:\nPatient will read this chart.\nPatient is reading this chart\nnow.", "squares in the margin: copy them"),
    "T_Chart_Stage": chart("BED 7", "INT. WARD - CONTINUOUS\n\n4471 turns to the next bed.\n         (beat)\nThey will hesitate here.\n\n4471 hesitates.", "same squares as 4471's chart"),
    "T_Chart_4470": chart("PATIENT 4470", "ADMITTED:  yesterday\nSTATUS:    WRAPPED\n\nNOTES:\nThe squares on this chart\nwere added later, in a\ndifferent hand.", "don't trust this one"),
    "T_Chart_Blank": chart("PATIENT ____", "ADMITTED:  next take\nCONDITION: pending\n\nNOTES:\n(blank)"),
    "T_TrustMark": (512, 512, f"""<div style="position:absolute;inset:0;background:#eef6f4"></div>
<div style="position:absolute;left:226px;top:30px;width:60px;height:452px;background:{MINT};transform:rotate(45deg)"></div>
<div style="position:absolute;left:226px;top:30px;width:60px;height:452px;background:{MINT};transform:rotate(-45deg)"></div>"""),
    "T_Sign_Service": sign("SERVICE ELEVATOR", "Staff and patients only.<br><br>Access pass 3.", colour="#5a6c72"),
})

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
