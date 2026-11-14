"""
make_placeholder_sfx.py: synthesized stand-in sound effects for TAKE 4471.

These are PLACEHOLDERS so the prototype has sound tonight. SkyYamaha replaces
them with real recordings during the jam. Everything is generated from math
(no samples), 48 kHz, 16-bit mono WAV.

Usage: python make_placeholder_sfx.py   (needs numpy)
"""
import os
import wave

import numpy as np

SR = 48000
HERE = os.path.dirname(os.path.abspath(__file__))
rng = np.random.default_rng(4471)


def t(sec):
    return np.arange(int(SR * sec)) / SR


def env(sec, attack=0.002, decay=0.3):
    x = t(sec)
    a = np.clip(x / attack, 0, 1)
    return a * np.exp(-x / decay)


def tone(freq, sec, decay, partials=((1, 1.0),)):
    x = t(sec)
    out = sum(amp * np.sin(2 * np.pi * freq * mult * x) for mult, amp in partials)
    return out * env(sec, decay=decay)


def bandpass_noise(sec, lo, hi):
    n = rng.standard_normal(int(SR * sec))
    spec = np.fft.rfft(n)
    f = np.fft.rfftfreq(len(n), 1 / SR)
    spec[(f < lo) | (f > hi)] = 0
    return np.fft.irfft(spec, len(n))


def pad(sig, sec):
    out = np.zeros(int(SR * sec))
    out[: len(sig)] = sig[: len(out)]
    return out


def write(name, sig, peak=0.8):
    sig = sig / (np.max(np.abs(sig)) + 1e-9) * peak
    data = (sig * 32767).astype("<i2")
    with wave.open(os.path.join(HERE, name + ".wav"), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SR)
        w.writeframes(data.tobytes())
    print(f"{name}.wav  {len(sig) / SR:.2f}s")


# Clapperboard snap: sharp wood crack + low thump + short room tail
snap = bandpass_noise(0.4, 900, 6000) * env(0.4, 0.0005, 0.018)
thump = tone(95, 0.4, 0.05)
tail = bandpass_noise(0.4, 300, 3000) * env(0.4, 0.01, 0.12) * 0.15
write("sfx_clapper_snap", snap + 0.6 * thump + tail)

# PA chime: friendly three-note corporate "ding-dong-ding" (E5 C5 G4)
bell = ((1, 1.0), (2, 0.35), (3, 0.12), (4.2, 0.05))
chime = np.zeros(int(SR * 2.4))
for i, f in enumerate((659.25, 523.25, 392.0)):
    s = tone(f, 1.6, 0.55, bell)
    start = int(i * 0.38 * SR)
    chime[start:start + len(s)] += s
write("sfx_pa_chime", chime)

# Elevator ding: one clean bell with slightly inharmonic partials
write("sfx_elevator_ding", pad(tone(1318.5, 2.0, 0.6, ((1, 1), (2.76, 0.3), (5.4, 0.08))), 2.0))

# Fluorescent buzz LOOP: 120 Hz mains hum + harmonics + irregular flicker. Exactly 4 s, loops cleanly.
x = t(4.0)
hum = sum(a * np.sin(2 * np.pi * 120 * k * x) for k, a in ((1, 1.0), (2, 0.5), (3, 0.35), (5, 0.2), (7, 0.12)))
flicker = 1 - 0.35 * (np.sin(2 * np.pi * 0.5 * x) > 0.92) - 0.2 * (np.sin(2 * np.pi * 1.75 * x) > 0.97)
write("sfx_fluoro_buzz_loop", hum * flicker, peak=0.5)

# Glyph toggle: soft click + tiny blip
write("sfx_glyph_toggle", pad(bandpass_noise(0.05, 2000, 8000) * env(0.05, 0.0003, 0.004), 0.15)
      + 0.4 * pad(tone(1760, 0.12, 0.03), 0.15), peak=0.6)

# Lock SOLVED: rising shimmer arpeggio (A minor add9, glassy)
solved = np.zeros(int(SR * 2.2))
for i, f in enumerate((440, 523.25, 659.25, 987.77, 1318.5)):
    s = tone(f, 1.4, 0.5, ((1, 1), (2, 0.25), (3, 0.1)))
    st = int(i * 0.09 * SR)
    solved[st:st + len(s)] += s
write("sfx_lock_solved", solved)

# Lock WRONG: dull low double buzz (no information, just "no")
wrong = np.zeros(int(SR * 0.7))
for st in (0.0, 0.28):
    b = np.sign(np.sin(2 * np.pi * 110 * t(0.22))) * env(0.22, 0.003, 0.15)
    i = int(st * SR)
    wrong[i:i + len(b)] += b
write("sfx_lock_wrong", wrong, peak=0.5)

# Lock CORRUPTED: bit-crushed descending glitch (malware)
x = t(1.2)
sweep = np.sin(2 * np.pi * np.cumsum(np.linspace(1800, 60, len(x))) / SR)
crushed = np.round(sweep * 4) / 4                        # 3-bit crush
hold = np.repeat(crushed[::12], 12)[: len(x)]            # sample-rate crush
stutter = (np.floor(x * 18) % 3 != 0)                    # chopped
write("sfx_lock_corrupted", hold * stutter * env(1.2, 0.002, 0.6), peak=0.6)

# Badge reader: OK (two rising beeps) / DENIED (one low beep)
ok = np.concatenate([tone(1567.98, 0.09, 1.0) * env(0.09, 0.002, 0.05), np.zeros(int(0.04 * SR)),
                     tone(2093.0, 0.12, 1.0) * env(0.12, 0.002, 0.07)])
write("sfx_badge_ok", ok, peak=0.5)
write("sfx_badge_denied", tone(392.0, 0.35, 1.0, ((1, 1), (3, 0.3))) * env(0.35, 0.002, 0.2), peak=0.5)

# Heartbeat loop (Fear stage 3+): lub-dub at 72 bpm, exactly 2 beats
beat = np.zeros(int(SR * 60 / 72 * 2))
for b in range(2):
    for off, amp in ((0.0, 1.0), (0.16, 0.6)):
        s = tone(50, 0.25, 0.06) * amp
        i = int((b * 60 / 72 + off) * SR)
        beat[i:i + len(s)] += s[: len(beat) - i]
write("sfx_heartbeat_loop", beat, peak=0.7)

# --- Friday night batch: sip, locked door, door open/close, footstep-free UI tick ---
# Slurp: short airy noise burst with a rising band (lips + air), then a small gulp
x = t(0.55)
band = np.zeros_like(x)
for i, (lo, hi) in enumerate(((300, 1200), (600, 2400), (900, 3600))):
    seg = bandpass_noise(0.55, lo, hi)
    band += seg * np.exp(-((x - 0.08 - i * 0.07) / 0.06) ** 2)
gulp = pad(tone(140, 0.18, 0.05, ((1, 1), (2, 0.4))), 0.55)
gulp = np.roll(gulp, int(SR * 0.38))
write("sfx_sip_slurp", band * 0.9 + gulp * 0.5, peak=0.6)

# Locked door: handle rattle (3 quick metallic knocks) + latch thunk
rattle = np.zeros(int(SR * 0.6))
for k, at in enumerate((0.0, 0.09, 0.17, 0.26)):
    hit = bandpass_noise(0.06, 1500, 6000) * env(0.06, 0.0005, 0.012)
    hit += tone(820 + k * 35, 0.06, 0.02, ((1, 1), (2.7, 0.4)))
    s0 = int(SR * at)
    rattle[s0:s0 + len(hit)] += hit[: len(rattle) - s0]
thunk = pad(tone(95, 0.2, 0.06) + bandpass_noise(0.2, 80, 400) * env(0.2, 0.001, 0.04), 0.6)
write("sfx_door_locked", rattle + 0.8 * np.roll(thunk, int(SR * 0.33)), peak=0.6)

# Door open: latch click + soft swing whoosh; close: whoosh + thud
click = pad(bandpass_noise(0.03, 2000, 7000) * env(0.03, 0.0003, 0.006), 0.9)
whoosh = bandpass_noise(0.9, 150, 900) * np.sin(np.pi * np.clip(t(0.9) / 0.9, 0, 1)) ** 2
write("sfx_door_open", click + 0.35 * whoosh, peak=0.45)
thud = pad(tone(70, 0.3, 0.08) + bandpass_noise(0.3, 60, 300) * env(0.3, 0.001, 0.05), 0.9)
write("sfx_door_close", 0.35 * whoosh[::-1] + 0.9 * np.roll(thud, int(SR * 0.6)), peak=0.5)

# Coffee pour: liquid trickle (bubbly band noise with amplitude flutter)
x = t(1.6)
flutter = 0.6 + 0.4 * np.abs(np.sin(2 * np.pi * 11 * x + 2 * np.sin(2 * np.pi * 3 * x)))
pour = bandpass_noise(1.6, 500, 3000) * flutter * np.clip(x / 0.1, 0, 1) * np.clip((1.6 - x) / 0.3, 0, 1)
write("sfx_coffee_pour", pour, peak=0.45)

# --- Game feel: footsteps (4 variations), pick up, throw, impacts ---
for i in range(4):   # soft, clicky office shoes (less thump)
    click = bandpass_noise(0.09, 1800 + 200 * i, 6500) * env(0.09, 0.0004, 0.006 + 0.001 * i)
    pad_ = bandpass_noise(0.09, 300, 1400) * env(0.09, 0.001, 0.014) * 0.35
    heel = np.roll(pad(bandpass_noise(0.03, 2500, 7000) * env(0.03, 0.0003, 0.004), 0.09), int(SR * 0.018)) * 0.5
    write(f"sfx_step_{i + 1}", pad(click + pad_ + heel, 0.12), peak=0.35)
write("sfx_pickup", pad(bandpass_noise(0.08, 900, 5000) * env(0.08, 0.001, 0.02) + 0.4 * tone(520, 0.08, 0.02), 0.2), peak=0.45)
x = t(0.35)
whoosh = bandpass_noise(0.35, 300, 2500) * np.sin(np.pi * np.clip(x / 0.35, 0, 1)) ** 1.5
write("sfx_throw", whoosh, peak=0.45)
write("sfx_impact_light", pad(bandpass_noise(0.12, 400, 4000) * env(0.12, 0.0005, 0.025) + 0.5 * tone(330, 0.12, 0.03, ((1, 1), (2.3, 0.5))), 0.3), peak=0.55)
write("sfx_impact_heavy", pad(tone(70, 0.35, 0.09) + bandpass_noise(0.35, 60, 1200) * env(0.35, 0.001, 0.07), 0.5), peak=0.7)

# Desk phone: classic double trill (two 0.4 s bursts of 20 Hz-gated 1.2+1.5 kHz), 3 s cycle; pick-up click; dial tone
x = t(0.4)
trill = (np.sin(2 * np.pi * 1200 * x) + 0.8 * np.sin(2 * np.pi * 1500 * x)) * (np.sin(2 * np.pi * 20 * x) > 0)
trill *= np.clip(x / 0.01, 0, 1) * np.clip((0.4 - x) / 0.01, 0, 1)
ring = np.zeros(int(SR * 3.0))
ring[: len(trill)] += trill
ring[int(SR * 0.6): int(SR * 0.6) + len(trill)] += trill
write("sfx_phone_ring", ring, peak=0.45)
write("sfx_phone_pickup", pad(bandpass_noise(0.05, 800, 5000) * env(0.05, 0.0005, 0.01), 0.3) + 0.6 * pad(tone(180, 0.08, 0.02), 0.3), peak=0.5)
x = t(2.0)
write("sfx_phone_dialtone", (np.sin(2 * np.pi * 350 * x) + np.sin(2 * np.pi * 440 * x)) * np.clip((2.0 - x) / 0.05, 0, 1), peak=0.25)

# Phone line static: crackly band-limited hiss with random pops, 6 s, fades in/out
x = t(6.0)
hiss = bandpass_noise(6.0, 300, 3400) * (0.5 + 0.5 * np.abs(np.sin(2 * np.pi * 0.7 * x)))
pops = np.zeros_like(x)
for _ in range(60):
    i = rng.integers(0, len(x) - 400)
    pops[i:i + 400] += rng.standard_normal(400) * np.exp(-np.arange(400) / 40) * rng.uniform(0.5, 2.0)
fade = np.clip(x / 0.15, 0, 1) * np.clip((6.0 - x) / 0.4, 0, 1)
write("sfx_phone_static", (hiss * 0.5 + pops * 0.6) * fade, peak=0.35)

# Eating: three crunchy bites (bright noise bursts with a low thud), a little wrapper crinkle
crunch = np.zeros(int(SR * 0.9))
for k, at in enumerate((0.0, 0.28, 0.55)):
    bite = bandpass_noise(0.12, 1500, 7000) * env(0.12, 0.002, 0.03) * (1.0 - 0.2 * k)
    bite += pad(tone(110, 0.08, 0.02), 0.12) * 0.4
    s0 = int(SR * at)
    crunch[s0:s0 + len(bite)] += bite[: len(crunch) - s0]
crinkle = bandpass_noise(0.9, 3000, 9000) * (rng.random(int(SR * 0.9)) > 0.985) * 0.6
write("sfx_eat_crunch", crunch + crinkle, peak=0.55)

# --- Restroom (Saturday) ---------------------------------------------------
# Tap: steady water (band noise with a slow flutter), soft start and stop
x = t(3.0)
water = bandpass_noise(3.0, 400, 6000) * (0.8 + 0.2 * np.sin(2 * np.pi * 7 * x)) + bandpass_noise(3.0, 150, 700) * 0.4
write("sfx_tap_water", water * np.clip(x / 0.12, 0, 1) * np.clip((3.0 - x) / 0.4, 0, 1), peak=0.4)
# Hand dryer: loud fan roar with a motor whine that spins up
x = t(4.0)
whine = np.sin(2 * np.pi * np.cumsum(220 + 180 * np.clip(x / 0.6, 0, 1)) / SR) * 0.15
roar = bandpass_noise(4.0, 300, 5000) + bandpass_noise(4.0, 80, 300) * 0.6
write("sfx_hand_dryer", (roar + whine) * np.clip(x / 0.5, 0, 1) * np.clip((4.0 - x) / 0.6, 0, 1), peak=0.6)
# Flush: a clunk, a rising rush that swirls down, then refill trickle
x = t(4.5)
rush = bandpass_noise(4.5, 200, 3000) * np.exp(-np.maximum(x - 0.3, 0) / 1.2) * np.clip((x - 0.1) / 0.2, 0, 1)
swirl = 0.6 + 0.4 * np.sin(2 * np.pi * (3 + 2 * x) * x)
trickle = bandpass_noise(4.5, 1500, 6000) * 0.15 * np.clip((x - 2.0) / 0.5, 0, 1) * np.clip((4.5 - x) / 0.5, 0, 1)
clunk = pad(tone(140, 0.12, 0.03, ((1, 1), (2.7, 0.4))), 4.5) * 0.8
write("sfx_flush", rush * swirl + trickle + clunk, peak=0.55)
# Knock: three knuckle raps on a thin stall door
knock = np.zeros(int(SR * 0.9))
for k, at in enumerate((0.0, 0.22, 0.44)):
    rap = tone(180, 0.15, 0.025, ((1, 1), (2.3, 0.5), (4.1, 0.25))) + bandpass_noise(0.15, 800, 3000) * env(0.15, 0.001, 0.01) * 0.5
    s0 = int(SR * at)
    knock[s0:s0 + len(rap)] += rap * (1.0 - 0.1 * k)
write("sfx_knock", knock, peak=0.6)
