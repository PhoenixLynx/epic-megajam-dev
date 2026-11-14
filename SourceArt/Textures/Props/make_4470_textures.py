"""
make_4470_textures.py: TAKE 4471. Traces of the employee before you: 4470.

  T_Label_4470      label on a moving box in the janitor closet (Permanent Marker)
  T_Polaroid_4470   a polaroid of desk #4471 with someone at it, face scribbled out
  T_Sticky_4470     a sticky note in handwriting (Caveat): "it's not your first day"
  T_Nameplate_4471  your nameplate: a 4471 sticker that doesn't quite cover 4470
Usage: python make_4470_textures.py   (playwright + Chrome; fonts Permanent Marker, Caveat, Poppins in ~/.fonts)
"""
import os

from playwright.sync_api import sync_playwright

HERE = os.path.dirname(os.path.abspath(__file__))

PAGES = {
    "T_Label_4470": (1024, 560, """
<div style="position:absolute;inset:0;background:#b8895a"></div>
<div style="position:absolute;left:60px;top:50px;right:60px;bottom:50px;background:#f3efe4;transform:rotate(-1.5deg);
 box-shadow:0 4px 0 rgba(0,0,0,.15);padding:30px 40px;font-family:'Permanent Marker';color:#1d1d1d">
 <div style="font-size:120px;line-height:1">4470</div>
 <div style="font-size:44px;margin-top:10px">PERSONAL EFFECTS</div>
 <div style="font-size:30px;margin-top:22px;color:#b02a2a">DO NOT OPEN &mdash; RETAKE</div>
</div>
<div style="position:absolute;left:0;right:0;top:250px;height:60px;background:rgba(210,190,150,.55)"></div>"""),
    "T_Polaroid_4470": (700, 840, """
<div style="position:absolute;inset:0;background:#f7f5ef"></div>
<div style="position:absolute;left:44px;top:44px;width:612px;height:612px;background:linear-gradient(#cfe1e3,#a9c3c6);overflow:hidden">
 <div style="position:absolute;left:60px;right:60px;bottom:120px;height:26px;background:#d8d2c4"></div>
 <div style="position:absolute;left:120px;bottom:146px;width:210px;height:130px;background:#22292c;border:8px solid #444"></div>
 <div style="position:absolute;left:380px;bottom:146px;width:110px;height:250px;background:#7fd4c4;border-radius:50px 50px 8px 8px"></div>
 <div style="position:absolute;left:395px;bottom:390px;width:80px;height:80px;border-radius:50%;background:#e6e1d8"></div>
 <div style="position:absolute;left:378px;bottom:378px;width:120px;height:110px;
  background:repeating-linear-gradient(35deg,#111 0 7px,transparent 7px 13px),repeating-linear-gradient(-40deg,#111 0 6px,transparent 6px 12px)"></div>
 <div style="position:absolute;left:0;right:0;top:0;bottom:0;background:radial-gradient(circle at 50% 40%,transparent 40%,rgba(0,0,0,.35))"></div>
</div>
<div style="position:absolute;left:60px;top:680px;font:64px Caveat;color:#2a2a6a;transform:rotate(-2deg)">desk 4471 &mdash; day 1</div>"""),
    "T_Sticky_4470": (600, 600, """
<div style="position:absolute;inset:0;background:#f6e27a;box-shadow:inset 0 -30px 40px rgba(0,0,0,.08)"></div>
<div style="position:absolute;left:44px;top:50px;right:40px;font:64px/1.05 Caveat;color:#1f2a6e;transform:rotate(-3deg)">
 if you're reading this<br>it's not your first day.<br><br>LOOK CLOSER.<br><span style="font-size:50px">&mdash; 4470</span></div>"""),
    "T_Nameplate_4471": (1024, 300, """
<div style="position:absolute;inset:0;background:#e9eef0;border:14px solid #b9c6ca"></div>
<div style="position:absolute;left:110px;top:60px;font:700 140px Poppins;color:#2b3a40;letter-spacing:8px">4470</div>
<div style="position:absolute;left:40px;top:52px;width:410px;height:190px;background:#ffffff;transform:rotate(-2deg);overflow:hidden;
 box-shadow:0 2px 6px rgba(0,0,0,.2);font:700 140px Poppins;color:#2b3a40;letter-spacing:4px;padding-left:8px;font-size:128px">4471</div>
<div style="position:absolute;left:560px;top:110px;font:600 46px Poppins;color:#7fd4c4">HALCYON</div>"""),
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
