"""
make_glyphs.py: the glyph alphabet for TAKE 4471's Key Locks and clues.

Eight glyphs, one per bit position (glyph 0 = lowest bit). Each comes in three
states: LIT (bit = 1), UNLIT (bit = 0), and the shared HIDDEN glyph ("?" seen
with low Soul Sight). Look: half corporate seal, half occult sigil, half film
leader: thin strokes inside a notched frame.

Writes SVGs + 256 px PNGs (transparent background) next to this file.
Needs `rsvg-convert` (librsvg) on PATH.
Usage: python make_glyphs.py
"""
import os
import subprocess

HERE = os.path.dirname(os.path.abspath(__file__))
S = 128  # design grid

LIT = "#EAFBFF"        # core stroke
GLOW = "#5FE3FF"       # soul-light cyan
UNLIT = "#6B7280"      # dead grey
HIDDEN = "#B8A16A"     # tarnished brass for '?'

# Notched frame shared by every glyph (corners cut like a film-gate / keycard)
FRAME = "M24 16 H104 L112 24 V104 L104 112 H24 L16 104 V24 Z"

# Inner sigils, drawn on a 128 grid (stroke paths)
GLYPHS = {
    0: "M64 34 V94 M64 26 m-4 0 a4 4 0 1 0 8 0 a4 4 0 1 0 -8 0",                      # the Witness: bar + eye-dot
    1: "M40 64 a24 24 0 1 0 48 0 a24 24 0 1 0 -48 0 M54 64 a10 10 0 1 0 20 0 a10 10 0 1 0 -20 0",  # the Reel
    2: "M64 34 L94 90 H34 Z M56 72 a8 6 0 1 0 16 0 a8 6 0 1 0 -16 0",                   # the Eye (pyramid)
    3: "M38 38 L90 90 M90 38 L38 90 M50 64 a14 14 0 1 0 28 0 a14 14 0 1 0 -28 0",       # the Lock (X + ring)
    4: "M34 50 H94 V92 H34 Z M34 50 L46 34 M54 50 L66 34 M74 50 L86 34",                # the Slate (clapperboard)
    5: "M64 64 m0 -6 a6 6 0 1 1 -6 6 a14 14 0 1 1 14 14 a24 24 0 1 1 -24 -24",          # the Spiral (time loop)
    6: "M36 44 H92 M36 64 H76 M36 84 H60 M92 84 V64",                                   # the Signal (fading bars)
    7: "M64 52 m-12 0 a12 12 0 1 0 24 0 a12 12 0 1 0 -24 0 M58 62 L52 94 H76 L70 62",   # the Keyhole
}
NAMES = {0: "Witness", 1: "Reel", 2: "Eye", 3: "Lock", 4: "Slate", 5: "Spiral", 6: "Signal", 7: "Keyhole"}
QUESTION = "M50 48 a14 14 0 1 1 20 12 c-6 3 -6 7 -6 12 M64 88 v2"


def svg(inner_path, color, glow, frame_opacity):
    glow_layer = ""
    defs = ""
    if glow:
        defs = (f'<defs><filter id="g" x="-30%" y="-30%" width="160%" height="160%">'
                f'<feGaussianBlur stdDeviation="5"/></filter></defs>')
        glow_layer = (f'<g filter="url(#g)" opacity="0.9" fill="none" stroke="{GLOW}" stroke-width="9" '
                      f'stroke-linecap="round" stroke-linejoin="round"><path d="{FRAME}"/><path d="{inner_path}"/></g>')
    return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {S} {S}" width="256" height="256">{defs}'
            f'{glow_layer}'
            f'<path d="{FRAME}" fill="none" stroke="{color}" stroke-width="3" opacity="{frame_opacity}" stroke-linejoin="round"/>'
            f'<path d="{inner_path}" fill="none" stroke="{color}" stroke-width="5" stroke-linecap="round" stroke-linejoin="round"/>'
            f'</svg>')


def emit(name, content):
    svg_path = os.path.join(HERE, name + ".svg")
    with open(svg_path, "w") as f:
        f.write(content)
    subprocess.run(["rsvg-convert", "-w", "256", "-h", "256", svg_path,
                    "-o", os.path.join(HERE, name + ".png")], check=True)


if __name__ == "__main__":
    for i, path in GLYPHS.items():
        emit(f"T_Glyph{i}_{NAMES[i]}_Lit", svg(path, LIT, glow=True, frame_opacity=0.9))
        emit(f"T_Glyph{i}_{NAMES[i]}_Unlit", svg(path, UNLIT, glow=False, frame_opacity=0.35))
    emit("T_Glyph_Hidden", svg(QUESTION, HIDDEN, glow=False, frame_opacity=0.5))
    print("glyphs written to", HERE)
