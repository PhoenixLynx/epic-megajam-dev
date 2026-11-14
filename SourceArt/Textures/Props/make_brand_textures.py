"""
make_brand_textures.py: HALCYON logo + kitchen notes.
  T_HalcyonLogo        mark + wordmark, mint on white (square: posters, badge, screens)
  T_HalcyonLogoPanel   white on mint, wide (reception desk front)
  T_FridgeNote_Yogurt  handwritten sticky note (snarky)
  T_FridgeNote_Memo    printed People Team memo
Fonts: Inter/Poppins (OFL), Permanent Marker (Apache 2.0), Caveat (OFL); listed as sourced in docs/RESOURCES.md.
Usage: python make_brand_textures.py
"""
import os
from playwright.sync_api import sync_playwright

HERE = os.path.dirname(os.path.abspath(__file__))
MINT, INK = "#7FD3C4", "#12313A"


def mark(color, size):
    # Half-sun rising over two horizon lines, with 7 short rays: calm, corporate, a little cult-like
    rays = "".join(
        f'<line x1="{50 + 30 * c:.1f}" y1="{56 - 30 * s:.1f}" x2="{50 + 40 * c:.1f}" y2="{56 - 40 * s:.1f}" stroke="{color}" stroke-width="4" stroke-linecap="round"/>'
        for c, s in [(__import__("math").cos(a), __import__("math").sin(a)) for a in [__import__("math").pi * k / 8 for k in range(1, 8)]])
    return (f'<svg viewBox="0 0 100 80" width="{size}" height="{size * 0.8}">'
            f'<path d="M26 56 A24 24 0 0 1 74 56 Z" fill="{color}"/>{rays}'
            f'<line x1="10" y1="64" x2="90" y2="64" stroke="{color}" stroke-width="5" stroke-linecap="round"/>'
            f'<line x1="26" y1="74" x2="74" y2="74" stroke="{color}" stroke-width="4" stroke-linecap="round"/></svg>')


PAGES = {
    "T_HalcyonLogo": (1024, 1024, f'''<div style="width:1024px;height:1024px;background:#fff;display:flex;flex-direction:column;align-items:center;justify-content:center;gap:30px;font-family:Poppins,Inter,sans-serif;color:{INK}">
        {mark(MINT, 520)}<div style="font-size:150px;font-weight:600;letter-spacing:34px;margin-left:34px">HALCYON</div>
        <div style="font-size:40px;letter-spacing:14px;color:#5b7a80">CONTINUITY</div></div>'''),
    "T_HalcyonLogoPanel": (2048, 512, f'''<div style="width:2048px;height:512px;background:{MINT};display:flex;align-items:center;justify-content:center;gap:70px;font-family:Poppins,Inter,sans-serif;color:#fff">
        {mark("#fff", 300)}<div><div style="font-size:190px;font-weight:600;letter-spacing:42px;line-height:1">HALCYON</div>
        <div style="font-size:52px;letter-spacing:22px;margin-top:18px;opacity:.9">CONTINUITY · WELLNESS · YOU</div></div></div>'''),
    "T_FridgeNote_Yogurt": (768, 768, '''<div style="width:768px;height:768px;background:linear-gradient(170deg,#fff6a8,#f6e27a);box-sizing:border-box;padding:70px 60px;font-family:'Permanent Marker',cursive;color:#1e2a6b;transform:rotate(0deg)">
        <div style="font-size:54px;line-height:1.25">TO WHOEVER KEEPS<br>TAKING MY YOGURT:</div>
        <div style="font-family:Caveat,cursive;font-size:76px;line-height:1.05;margin-top:34px">I've seen every take.<br>I <u>KNOW</u> it's you.</div>
        <div style="font-family:Caveat,cursive;font-size:58px;margin-top:40px;text-align:right">- Accounting</div></div>'''),
    "T_FridgeNote_Memo": (1024, 1365, '''<div style="width:1024px;height:1365px;background:#fbfaf6;box-sizing:border-box;padding:110px 100px;font-family:Inter,sans-serif;color:#22303a">
        <div style="font-size:36px;letter-spacing:10px;color:#7FD3C4;font-weight:700">HALCYON PEOPLE TEAM</div>
        <div style="font-size:84px;font-weight:800;margin-top:30px;line-height:1.05">KITCHEN<br>REMINDER</div>
        <div style="font-size:44px;line-height:1.5;margin-top:50px">Please label your food.<br><br>Unlabelled items will be <b>re-cast</b>.<br><br>Fridge clean-out is every Friday.<br>It is always Friday.</div>
        <div style="font-size:30px;margin-top:80px;color:#6b7a85">Thank you for your energy. 🌅</div></div>'''),
}

if __name__ == "__main__":
    with sync_playwright() as p:
        b = p.chromium.launch(executable_path=os.environ.get("CHROME", "/usr/bin/google-chrome"))
        pg = b.new_page()
        for name, (w, h, body) in PAGES.items():
            pg.set_viewport_size({"width": w, "height": h})
            pg.set_content(f"<html><body style='margin:0'><div id='t'>{body}</div></body></html>")
            pg.wait_for_timeout(700)
            pg.locator("#t > div").first.screenshot(path=os.path.join(HERE, name + ".png"))
        b.close()
    print("brand textures written")
