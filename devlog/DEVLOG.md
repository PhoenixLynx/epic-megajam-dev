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
