# Audio Plan: generative ambient

**Composer:** me (SkyYamaha). All recordings made during the jam.
**Engine side:** Unreal MetaSounds, driven by the run seed, the Fear value, and Soul Sight.

## References (inspiration only, nothing copied)

- Spacing / mood reference I picked: https://youtu.be/Jli-cviuC2c

## Two palettes

| Palette | Where | Feel | Suggested scales |
|---------|-------|------|-----------------|
| **Bright ambient** | Campus (Interface) | Clean, airy, corporate-calm, a little too pleasant | Major pentatonic, Lydian (bright, floating) |
| **Dark ambient** | Augmented Fear, Astral | Eerie, heavy, close, wrong | Phrygian, Locrian, whole-tone (unstable, uneasy) |

The **Fear value (0→1)** crossfades from bright to dark. Soul Sight adds a shimmer layer on top of either.

## Generative techniques (all cheap to build)

1. **Phasing loops (Eno-style):** several pad and drone loops of *different, odd lengths* (e.g. 7, 11, 13, 17 seconds) play at once. Because the lengths don't divide evenly, they drift against each other and the combination almost never lines up the same way twice. No code needed beyond "loop each one."
2. **Shuffle bag:** melodic phrases go in a bag; the game draws one at random, never repeating until the bag is empty, then reshuffles. Variety without the same phrase twice in a row.
3. **Follow-on table (simple Markov chain):** for each phrase, a short list of which phrases may follow it, with weights. It keeps the random order musically sensible: phrase A tends to lead to C or D, never B.
4. **Scale-locked random walk:** record single notes of one instrument. The game picks the next note by stepping up or down a little within the current scale (bright scale on campus, dark scale in Fear). It writes new melodies forever that still sound like you.
5. **Fear-driven processing:** as Fear rises, pitch drifts flat (detune), a low-pass filter muffles the highs, phrases get sparser and slower, and bright layers drop out one by one.
6. **Random variation per play:** small random pitch, volume and start-point offsets on every one-shot, so repeated sounds never sound copy-pasted.
7. **Loop stutter:** in a time loop, the music repeats the last bar or two and stutters, the "trapped" feeling in sound.

All randomness uses the **run seed**, so each run's soundtrack is unique.

## Recording list

**Bright ambient (campus)**
- [ ] 3–4 pad loops, each a different odd length (7s, 11s, 13s, 17s)
- [ ] 6–10 short melodic phrases (2–4 bars), corporate-calm
- [ ] Single notes of one clean instrument across 2 octaves (for the random walk)
- [ ] Chimes / UI-style one-shots (elevator ding, notification, PA chime)
- [ ] Short "corporate anthem" theme for the ending party (+ one slightly out-of-tune stem)

**Dark ambient (Fear / Astral)**
- [ ] 3–4 drone loops, odd lengths (sub hum, metallic, breathy)
- [ ] 6–10 eerie phrases or textures
- [ ] Single notes of a darker instrument across 2 octaves
- [ ] Stingers: 4–6 short hits for scares and reveals
- [ ] Chase stems: pulse, percussion, rising tension (looping, tempo-matched)
- [ ] Soul Sight shimmer layer (high, glassy, luminous)

**Format:** .wav, 48 kHz, mono for one-shots and notes, stereo for pads, drones and stems. Trim loops so they loop cleanly with no click.

## Reusing motifs from my older work

New recordings of a musical idea from my earlier albums are new jam content, but for transparency I'll **note any reused motif in the devlog**. Old recorded audio files are not used.
