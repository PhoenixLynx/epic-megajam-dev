# Devlog: 2026 Epic AI MegaJam (itch.io)

**Developer:** phoenixlynx (solo)
**Engine:** Unreal Engine
**Jam dates:** Thu Nov 12 – Sun Nov 15, 2026
**Deadline:** Sun Nov 15, 2026, 6:59 PM Phoenix time (MST, UTC-7)
**Theme:** [THEME]
**Log started:** Thu Nov 12, 2026

Every asset and line of gameplay code in this project is made during the jam window.

---

## Stage tracker

| # | Stage | Status | Started | Finished |
|---|-------|--------|---------|----------|
| 1 | Concept | 🔄 In progress | Nov 12 19:19 | |
| 2 | Storyboard | ⬜ | | |
| 3 | Prototype | ⬜ | | |
| 4 | Art | ⬜ | | |
| 5 | Audio | ⬜ | | |
| 6 | Polish | ⬜ | | |
| 7 | Packaging | ⬜ | | |
| 8 | Submission | ⬜ | | |

---

## AI tool ledger

A running record of every AI tool used and what it did.

| When (MST) | Tool | What it did |
|------------|------|-------------|
| 2026-11-12 19:17 | Claude (Anthropic) | Set up repo scaffold: git on `main`, Unreal .gitignore, Git LFS tracking, folder layout, this devlog, submission checklist. |
| 2026-11-12 19:19 | Claude (Anthropic) | Turned my raw concept notes into docs/CONCEPT.md: layer structure, core loop, XOR Key Lock puzzle design, seed approach, MVP scope, Real World options. Ideas and direction are mine; Claude structured and scoped them. |
| 2026-11-12 19:26 | Claude (Anthropic) | Updated CONCEPT.md to v0.2 from my direction: corporate campus start, Soul Sight "slip" progression, Augmented Fear layer + creep stages, Wellness Center → Hospital, Unreal techniques for the transition, revised MVP/stretch list. |
| 2026-11-12 19:27 | Claude (Anthropic) | CONCEPT.md v0.3 from my direction: escalation table, "trapped in time and space" mechanics (loops, frozen time, rewind on capture), subtle Key Lock with a 3-key difficulty ramp. |
| 2026-11-12 19:30 | Claude (Anthropic) | CONCEPT.md v0.4 from my direction: NPCs as seeded malware agents with false clues; party ending with the Collector, broken into beats. |

---

## Sessions

### Session 1: Thu Nov 12, 2026, 19:17 MST: Setup

- **Goal:** Get the repo, folders, and record-keeping ready before any game work.
- **Done:**
  - Git repo initialized on branch `main`.
  - Unreal `.gitignore` added (Binaries/, Intermediate/, Saved/, DerivedDataCache/, .vs/, and the usual extras).
  - Git LFS tracking for `.uasset .umap .fbx .blend .wav .png .mp4`.
  - Folders created: `devlog/`, `devlog/screenshots/`, `storyboard/`, `docs/`.
  - `devlog/DEVLOG.md` and `docs/SUBMISSION_CHECKLIST.md` created.
- **Decisions & why:**
  - LFS set up before the first asset, so no binary ever lands in plain git history (that's hard to undo later).
  - The repo root is meant to be the Unreal project root (the folder with the `.uproject`), so the ignore paths match.
- **Problems:** Remote URL and theme not provided yet.
- **AI tools used:** Claude (scaffold + docs).
- **Next:** Get theme and game ideas → Concept stage. Connect remote and push.

### Session 2: Thu Nov 12, 2026, 19:19 MST: Concept

- **Goal:** Get my concept out of my head and into a buildable shape.
- **Done:**
  - Wrote out the raw idea: recover soul energy from gatekeeper entities in an Astral Real-Time Zone, mint soul bits by solving generated keys, upgrade Soul Sight until you can see through the interface to Source. Cryptographic horror.
  - Drafted `docs/CONCEPT.md` (v0.1).
- **Decisions & why:**
  - **Astral Zone = overlay on the same level, toggled by Soul Sight.** Halves level-building work, and "seeing through" is the core fantasy anyway.
  - **XOR Key Lock as the lead puzzle.** It reads as cryptography, is guaranteed solvable when generated backwards from a hidden answer, and ties directly into Soul Sight (hidden bits, decoy false bits).
  - **Unrecreatable runs** via a clock-based seed, plus a dev-only override for debugging.
  - **MVP = 3 soul bits to win**, 1 level, 1 enemy + 2 variants.
- **Real World options (undecided):**
  - **A: Cached Home.** Your own apartment, but rendered like a cache: textures load in late, photos have faces replaced by scrolling hex, rooms repeat with tiny differences, the front door opens to a hallway that ends in a loading spinner. Personal (fits "your soul"), cheap to build.
  - **B: Onboarding Campus.** A bright corporate wellness office. Every door asks for consent, posters thank you for your energy, the staff are mannequins with name badges. Strongest Magilumiere / Death Note "contract" flavor.
  - **C: Terms Street.** A quiet night street where every sign, receipt and billboard is fine print that rewrites itself when you look away.
- **Problems:** Theme still unknown, so the concept may need to bend.
- **AI tools used:** Claude (concept structuring + scoping).
- **Next:** Theme → lock concept → storyboard.

### Session 3: Thu Nov 12, 2026, 19:26 MST: Concept direction locked

- **Goal:** Pick the Real World and define the enemy experience.
- **Done:**
  - **Real World = the Campus:** you start as a standardized citizen-employee at a bright corporation. The cheer is there to contrast with the conspiracy underneath.
  - **Soul Sight is something you slip into**, growing with each solved key: involuntary flickers at first, full sight at 3 soul bits.
  - **XOR Key Lock confirmed** as the puzzle, uniquely generated every time.
  - **New layer: Augmented Fear.** The malware brainwashes and inflicts extreme fear. You slip into it without knowing exactly when, until you're being stalked and chased. It should feel visceral. Not a siren-style hard switch.
  - **Hospital:** the campus Wellness Center becomes the Hospital in Fear.
  - Concept doc updated to v0.2.
- **Decisions & why:**
  - Soul Sight (truth) and Augmented Fear (lies) are framed as opposites; Soul Sight can pierce Fear but drains fast. One tension drives both systems.
  - Fear is one 0→1 value driving post-process, materials, audio and off-screen actor swaps, so the creep is gradual and it's all one system to build.
  - Hospital wing moved to stretch goal #1; Wellness Center covers it in the MVP.
- **Problems:** Theme still unknown.
- **AI tools used:** Claude (concept doc update, scoping, Unreal technique suggestions).
- **Next:** Theme → storyboard (campus layout + first Fear episode beat by beat).

### Session 4: Thu Nov 12, 2026, 19:27 MST: Escalation, entrapment, subtle puzzles

- **Goal:** Make the Fear zones scale and make puzzles harder to brute-force.
- **Done:**
  - The malware's tricks and Fear zones escalate as you get better (more soul bits) and closer to Source.
  - Fear zones should make you feel **trapped in time and space**: looping corridors, doors back to the same room, frozen clocks, repeating announcements, rewind on capture.
  - **Puzzles and solutions should be subtle and not easy to check:** target hidden in the environment, no running result, no partial feedback, wrong answers raise Fear; Soul Sight is a costly way to check.
  - Concept doc updated to v0.3.
- **Decisions & why:**
  - **3-key difficulty ramp.** A fully subtle first key risks jam players quitting before they learn the rule; key 1 teaches, keys 2–3 hide.
  - **Rewind on capture instead of game over.** Fits "trapped in time," and avoids a restart screen.
  - **Loop corridor via invisible teleport.** Cheapest convincing space trap in Unreal; MVP includes one.
- **Problems:** Theme still unknown. Combat style undecided.
- **AI tools used:** Claude (concept update, mechanic suggestions).
- **Next:** Theme + combat decision → storyboard.

### Session 5: Thu Nov 12, 2026, 19:30 MST: NPC agents and the ending

- **Goal:** Define who you can trust, and how it ends.
- **Done:**
  - Confirmed: getting stuck in a loop is a core feeling of the game.
  - **Any NPC, even the nice ones, can be an agent of the malware.**
  - **Ending:** like the movie *The Game*, all the malware and horrible things turn out to be your best friends, and a giant corporate party celebrates your win. But something still seems off. You've recovered your soul bits, and something comes to "collect" them to build you a utopian afterlife or palace. It leaves you hanging: can you trust them, or have you been stolen from again?
  - Concept doc updated to v0.4.
- **Decisions & why:**
  - **Agents chosen by the run seed**, so trust has to be re-earned every run, and they feed false clues into the subtle Key Lock. One system serves both paranoia and puzzle difficulty.
  - **Ending ends on a cut to black mid-Soul Sight flicker** so the question stays open.
  - **Party room reuses existing NPCs and the stalker model**, which keeps the ending cheap to build.
- **Problems:** Theme still unknown; combat style still open.
- **AI tools used:** Claude (concept update, ending beat breakdown).
- **Next:** Theme + combat decision → storyboard.
