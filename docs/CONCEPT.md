# Concept (draft v0.1): working title "SOULHASH"

**Genre:** first-person cryptographic horror, explore / fight / puzzle
**Status:** draft. Waiting on jam theme; open questions at bottom.

## Pitch (player's words)

> Find the missing pieces of your own soul code, called keys. You solve for them, like mining your own soul bitcoins. As you solve, your Soul Sight improves until you can see both the interface and the source code. Malware and false bits stand in the way, and enemies get harder as you level and the closer you get. Centralized entities have built an illusion overlay to trap you and divert your soul energy.

## The three layers

| Layer | What it is | What the player does there |
|-------|-----------|----------------------------|
| **Real World** (the Interface) | A reality that looks normal but doesn't feel right. It's a skin drawn over the truth. | Explore, find clues, get ambushed. |
| **Astral Real-Time Zone** (the Overlay) | The hidden layer the gatekeeper entities built to hide and gate your soul energy. It runs live, in "real time". | Fight the gatekeepers, take back soul energy. |
| **Source** | The true code underneath. The final goal. | Reach it with enough soul bits = win. |

**Build trick:** the Astral Zone is drawn *on top of the same level* as the Real World. Turning on Soul Sight switches post-process and materials and reveals hidden actors. That gives two worlds for the cost of one level, and the core fantasy is literally "seeing through the interface."

## Core loop

1. Explore the Real World.
2. Use **Soul Sight** (time-limited, it drains) to see the Astral overlay: hidden enemies, energy, false bits.
3. **Fight** gatekeepers (they deal real damage) to free **soul energy**.
4. Spend the energy at a **Key Lock** to solve a generated puzzle → **mint a soul bit** (soul bits only exist once you've recovered the energy).
5. Each soul bit **upgrades Soul Sight** (longer, sees more) and can be spent to **Dissolve** (destroy) or **Patch** (neutralize) an enemy instead of fighting it.
6. With N soul bits (target: 3), the path to **Source** opens.

## Puzzle: XOR Key Lock (recommended)

- The lock shows a **target signature**: a row of 8 glyphs, each glyph = one bit, on or off.
- You get 4–8 **key fragments**, each its own 8-glyph row.
- Toggle fragments on and off. The lock combines the active ones with **XOR** (exclusive-or: a bit is on if an odd number of active fragments have it on) and shows the running result.
- Match the target = key solved.
- **Always solvable by construction:** generate the fragments randomly, secretly pick a random subset, XOR them together, and that result *is* the target.
- **False bits:** some fragments are malware decoys that corrupt the result. Only higher Soul Sight reveals them.
- **Sight ties in:** at low Soul Sight some glyphs show as `?`. Each level reveals more.
- **Scaling:** more bits per row, more fragments, more decoys, and a timer while enemies close in.

Fallback if XOR feels too abstract: Lights-Out style glyph grid (press a node, it and its neighbors flip). Generate it by pressing random nodes from the solved state, so it's always solvable.

## Every run is unique

- Run seed comes from the system clock's high-resolution ticks plus a random stream. It's never shown or saved, so a run can't be recreated.
- The seed drives: puzzle generation, enemy placement, which rooms hide energy, small Real World "wrongness" details.
- **Dev-only seed override** (stripped or hidden in the shipped build) so bugs can be reproduced during the jam.

## Enemies: the Gatekeepers

- Centralized entities that hold your soul energy.
- **Difficulty = f(soul bits owned, distance to Source):** health, speed and aggression go up as you level and get closer.
- Each one carries a visible **code signature** under Soul Sight. Spend a soul bit to Dissolve or Patch it.
- MVP: 1 enemy base + 2 variants (melee chaser, ranged "notary" that stamps/curses).

## Aesthetic: cryptographic horror

- Bright, polite corporate surface vs. hostile hidden machinery underneath.
- Inspirations (for mood only, nothing copied): Magilumiere Co. Ltd. (corporate magic bureaucracy), Death Note (rules, names, contracts), Backrooms / Control / SCP (wrong spaces, institutional horror), The Matrix (code under reality).
- Soul Sight look: shining, luminous linework bleeding through surfaces into glyph-code.

## MVP scope (3 days, solo)

- 1 Real World level (a handful of rooms) + Astral overlay via Soul Sight
- XOR Key Lock, procedurally generated
- 1 enemy type + 2 variants, simple combat
- 3 soul bits → Source ending
- Title screen, death/restart, ending screen

## Open questions

- [ ] Jam theme. Concept must bend to fit it.
- [ ] What the Real World is (options in DEVLOG Session 2).
- [ ] Puzzle choice: XOR Key Lock vs. glyph grid.
- [ ] Combat style: melee, projectile, or both.
- [ ] Final title.
