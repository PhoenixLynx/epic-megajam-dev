# Concept (v0.2): working title "SOULHASH"

**Genre:** first-person cryptographic horror, explore / stalk-and-chase / puzzle
**Status:** core direction locked. Waiting on jam theme.

## Pitch

You are a standardized citizen-employee at a bright, cheerful corporation. Under the surface, centralized entities have built an illusion overlay to trap people and divert their soul energy. You start to see through it. Every key you solve pulls you further out of the interface and into the truth, while the malware fights back with brainwashing and engineered fear.

## The layers

| Layer | Who made it | Feel | Role |
|-------|------------|------|------|
| **Interface: the Campus** | The corporation | Bright, clean, polite, too perfect | Where the game starts. Your normal workday. |
| **Astral Real-Time Zone** | The gatekeepers (hidden) | The truth: shining linework, glyph-code bleeding through surfaces | Seen through **Soul Sight**. Where your soul energy is held. |
| **Augmented Fear** | The malware | Visceral, rotting, stalked: the campus turned against you | The malware's attack state. Brainwashing + extreme fear. |
| **Source** | No one / you | Pure code | Final goal. |

**Soul Sight = truth. Augmented Fear = lies.** They are opposites, and that's the core tension.

## Real World: the Campus

- You're an employee: badge, ID number, standardized desk, wellness reminders.
- Bright corporate surface: posters thanking you for your energy, consent prompts on doors, smiling staff who are a little too still.
- Mood references (nothing copied): Magilumiere Co. Ltd.'s corporate cheer, Death Note's contracts and rules.
- **The Wellness Center** is the campus clinic. In Augmented Fear it becomes **the Hospital**.

## Soul Sight: you slip into it

Not a free toggle at the start. It grows with each soul bit:

| Soul bits | Soul Sight behavior |
|-----------|--------------------|
| 0 | Involuntary flickers: a second of glyph-light bleeding through a wall, then gone. |
| 1 | Hold to see, briefly; it drains fast. |
| 2 | Longer; reveals false bits and enemy code signatures. |
| 3 | Sees both interface and source. Path to Source opens. |

## Augmented Fear: the slip

The player shouldn't know exactly when it starts. No siren, no hard cut. It creeps in over ~30–90 seconds until you realize it's wrong and something is already following you.

**Creep stages (all driven by one 0→1 "Fear" value):**
1. **Off by a little:** a light buzzes, an ambient sound lags, one poster's text is different.
2. **Draining:** color desaturates, fog thickens, footsteps echo wrong, doors you passed are now closed.
3. **Watched:** staff mannequins have turned to face you, the corridor is longer than it was.
4. **Stalked:** the gatekeeper is visible at the edge of the screen, then it chases.

**Unreal tools:** a post-process blend weight lerped by Fear, a material parameter collection (rot/grime/desaturation on every surface), an audio low-pass + heartbeat layer, and actor swaps out of the player's view (check if it's off-screen before swapping).

**Soul Sight pierces Fear:** using it during Fear shows what's real (true exits, fake enemies, the stalker's code signature). But it drains fast, so it's a tense resource.

## Core loop

1. Work your day on the Campus. Small objectives, notice what's wrong.
2. Soul Sight flickers reveal hidden **soul energy** and **Key Locks**.
3. Going after energy triggers **Augmented Fear**: the campus turns, a gatekeeper stalks and chases. It deals real damage.
4. Survive, recover the energy, reach a **Key Lock**, solve it → **mint a soul bit**.
5. Soul bit = stronger Soul Sight, and can be spent to **Dissolve** or **Patch** a gatekeeper.
6. Each bit makes the next Fear episode harder and deeper (Hospital wing).
7. 3 bits → Source.

## Puzzle: XOR Key Lock (locked in)

- Target signature: a row of glyph-bits. Key fragments: rows of glyph-bits. Toggle fragments; active ones combine by XOR. Match the target.
- Generated backwards from a secret answer subset → always solvable, never the same.
- **False bits:** malware decoy fragments, revealed only at Soul Sight 2+.
- Low sight shows some glyphs as `?`.
- Scaling: more bits, more fragments, more decoys, solving while stalked.

## Every run is unique

- Seed from high-res clock ticks; never shown or saved.
- Drives: puzzles, energy and lock locations, gatekeeper spawns, which campus details are "wrong."
- Dev-only seed override for debugging.

## Enemies: Gatekeepers

- Malware entities that brainwash and inflict fear. They stalk first and chase second.
- **Difficulty = f(soul bits owned, distance to Source).**
- Code signature visible under Soul Sight; spend a soul bit to Dissolve or Patch.
- MVP: 1 stalker (chase + grab damage). Stretch: a ranged "Notary" variant.

## MVP scope (3 days, solo)

**Must:**
- Campus level (lobby, office floor, corridor, Wellness Center), built once
- Augmented Fear as a state on the same level (post-process + materials + audio + swaps)
- Soul Sight bleed tied to soul bits
- XOR Key Lock, generated
- 1 stalker gatekeeper with chase + damage
- 3 soul bits → Source ending; title, death/restart, end screen

**Stretch (in order):**
1. Hospital wing as its own deeper Fear area
2. Dissolve / Patch spend
3. Second enemy variant
4. Extra campus "wrongness" events

## Open questions

- [ ] Jam theme. The concept bends to fit it.
- [ ] Combat: can the player fight back, or only evade until Dissolve/Patch?
- [ ] Final title.
