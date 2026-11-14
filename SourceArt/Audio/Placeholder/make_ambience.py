"""
make_ambience.py: placeholder AMBIENT loops for TAKE 4471 (replace with SkyYamaha recordings).
Every file loops seamlessly (length is a whole number of cycles of every tone; noise is crossfaded).
"""
import os, wave
import numpy as np

SR = 48000
HERE = os.path.dirname(os.path.abspath(__file__))
rng = np.random.default_rng(2214)


def write(name, sig, peak=0.5):
    sig = sig / (np.max(np.abs(sig)) + 1e-9) * peak
    with wave.open(os.path.join(HERE, name + ".wav"), "wb") as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(SR)
        w.writeframes((sig * 32767).astype("<i2").tobytes())
    print(name)


def loop_noise(sec, lo, hi, xfade=1.0):
    """Band-limited noise that loops: crossfade the tail into the head."""
    n, x = int(SR * sec), int(SR * xfade)
    raw = rng.standard_normal(n + x)
    spec = np.fft.rfft(raw); f = np.fft.rfftfreq(len(raw), 1 / SR)
    spec[(f < lo) | (f > hi)] = 0
    raw = np.fft.irfft(spec, len(raw))
    out = raw[:n].copy()
    fade = np.linspace(0, 1, x)
    out[:x] = raw[:x] * fade + raw[n:n + x] * (1 - fade)
    return out


t = lambda sec: np.arange(int(SR * sec)) / SR

# Office room tone: HVAC air + soft hum + occasional keyboard clicks + a distant phone trill
sec = 16.0
office = loop_noise(sec, 60, 900) * 0.6 + 0.15 * np.sin(2 * np.pi * 60 * t(sec))
for _ in range(70):
    i = rng.integers(0, int(SR * (sec - 0.1)))
    click = rng.standard_normal(240) * np.exp(-np.arange(240) / 30) * rng.uniform(0.08, 0.25)
    office[i:i + 240] += click
ring_at = int(SR * 9.0)
ring = np.sin(2 * np.pi * 1200 * t(0.8)) * (np.sin(2 * np.pi * 20 * t(0.8)) > 0) * 0.05
office[ring_at:ring_at + len(ring)] += ring
write("amb_office_roomtone_loop", office, 0.35)

# Kitchen: fridge compressor hum (55/110/165 Hz) + light air
sec = 8.0
fridge = sum(a * np.sin(2 * np.pi * f * t(sec)) for f, a in ((55, 1), (110, 0.5), (165, 0.25), (330, 0.06)))
fridge = fridge * (1 + 0.05 * np.sin(2 * np.pi * 0.25 * t(sec))) + loop_noise(sec, 100, 2000) * 0.15
write("amb_kitchen_fridge_loop", fridge, 0.3)

# Lobby: big soft air + a faint, too-calm major chord drone (corporate lobby music placeholder)
sec = 16.0
chord = sum(np.sin(2 * np.pi * f * t(sec)) * 0.1 for f in (130.8125, 196.0, 261.625, 329.625))   # C3 G3 C4 E4 (whole cycles in 16 s)
lobby = loop_noise(sec, 80, 1500) * 0.4 + chord * (0.7 + 0.3 * np.sin(2 * np.pi * t(sec) / 16))
write("amb_lobby_air_loop", lobby, 0.3)

# Corridor B: lower, closer, wrong: sub hum + fluorescent buzz + slow detuned beating
sec = 12.0
corr = (np.sin(2 * np.pi * 40 * t(sec)) * 0.6 + np.sin(2 * np.pi * 120 * t(sec)) * 0.3
        + np.sin(2 * np.pi * 120.25 * t(sec)) * 0.3 + loop_noise(sec, 30, 300) * 0.5)
write("amb_corridorb_hum_loop", corr, 0.35)
