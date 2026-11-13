# Devlog: 2026 Epic AI MegaJam (itch.io)

**Developer:** phoenixlynx (solo)
**Engine:** Unreal Engine
**Jam dates:** Thu Nov 12 – Sun Nov 15, 2026
**Deadline:** Sun Nov 15, 2026, 6:59 PM Phoenix time (MST, UTC-7)
**Theme:** DIRECTOR'S CUT
**Log started:** Thu Nov 12, 2026

Every asset and line of gameplay code in this project is made during the jam window.

---

## Stage tracker

| # | Stage | Status | Started | Finished |
|---|-------|--------|---------|----------|
| 1 | Concept | ✅ Done | Nov 12 19:19 | Nov 12 19:54 |
| 2 | Storyboard | 🔄 In progress | Nov 12 19:59 | |
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
| 2026-11-12 19:36 | Claude (Anthropic) | CONCEPT.md v0.5 from my direction: improvised combat (objects, fire extinguisher, path-blocking debris) with Unreal implementation notes; ending offer + Yes/No choice. |
| 2026-11-12 19:37 | Claude (Anthropic) | CONCEPT.md v0.6 from my direction: coffee burn + more office weapons; "astral logic" powers tiered by soul bits, with Unreal implementation notes. |
| 2026-11-12 19:40 | Claude (Anthropic) | Web-searched the jam rules on pre-made assets and the theme. Added the music plan (stems composed during the jam + MetaSounds runtime generation) to CONCEPT.md; added rule items to the checklist. |
| 2026-11-12 19:42 | Claude (Anthropic) | Wrote docs/AUDIO_PLAN.md: bright/dark ambient palettes, 7 generative techniques, recording list. I compose all the music; Claude designed the generative system. |
| 2026-11-12 19:46 | Claude (Anthropic) | Brainstormed "Director's Cut" theme tie-ins → docs/THEME_BRAINSTORM.md (core reframe, mechanics, ending, titles). Web-searched to try to confirm the theme officially (not confirmed yet). |
| 2026-11-12 19:48 | Claude (Anthropic) | CONCEPT.md v0.7: folded in the theme. "Orchestrated movie" spine, the Chairman with a 7-step foreshadowing list, movie motifs, wrap-party ending, theatrical vs. director's cut. |
| 2026-11-12 19:52 | Claude (Anthropic) | CONCEPT.md v0.8: workday activity table (email, compliance training, video calls, meetings, NPCs, team building) and day structure (3 acts = 3 keys). |
| 2026-11-12 19:54 | Claude (Anthropic) | Read Epic's partner-perks email I shared; confirmed the theme; sorted perks into use / maybe / skip for this game in docs/RESOURCES.md (codes and links left out of the repo). |
| 2026-11-12 19:59 | Claude (Anthropic) | Drafted storyboard/STORYBOARD.md v0.1 from the concept: campus map, beat tables for Acts 1–3 and the ending, MVP/stretch tags, screenshot markers, scope fallback. Added SourceArt/ folders + LFS/ignore rules for Blender and Ableton files. |
| 2026-11-12 20:06 | Claude (Anthropic) | Added access passes, security guards and custodians to CONCEPT.md and STORYBOARD.md from my direction. |

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

### Session 6: Thu Nov 12, 2026, 19:36 MST: Combat and the ending choice

- **Goal:** Decide how the player fights and how the ending offer works.
- **Done:**
  - **Combat:** the player can fight back with any object the world has, not because they're a trained fighter, but because they have to use what's there: fire extinguishers, office supplies. You can slow enemies with things you put in their path.
  - **Ending offer:** you're told the astral realm is open now and you can build yourself a world; you just have to hand over your keys.
  - **Ending choice:** Yes/No. If you choose no, maybe they're taken anyway. Refine as it unfolds.
  - Concept doc updated to v0.5.
- **Decisions & why:**
  - **Improvised hits stagger and slow; only soul bits end a gatekeeper.** Keeps the horror (it's never safe to stay and fight) while giving the player agency.
  - **Debris slows via an overlap speed penalty**, not runtime navmesh rebuilding, which is cheaper and more reliable in a jam.
  - **Fire extinguisher** as the signature tool: limited charge, blinds and slows.
- **Problems:** Theme still unknown.
- **AI tools used:** Claude (concept update, combat implementation notes).
- **Next:** Theme → close Concept stage → storyboard.

### Session 7: Thu Nov 12, 2026, 19:37 MST: Astral logic

- **Goal:** Expand improvised combat and connect it to astral progress.
- **Done:**
  - More improvised weapons: burn enemies with coffee; throw reams of printer paper, keyboards, cabinets.
  - **Astral logic:** the more you can be in the astral realm, the more you can bend physics: short flight, telekinesis with objects, a certain amount of impossible physics. Nothing too crazy.
  - Concept doc updated to v0.6.
- **Decisions & why:**
  - **Powers tiered by soul bits** (telekinesis → float → heavy throw), so progress is felt in your hands, not just in what you see.
  - **Telekinesis reuses the grab system** with longer reach, so it's nearly free to build.
  - **All astral powers drain Soul Sight**: one resource for seeing truth, piercing Fear, checking locks, and bending physics. Every use is a choice.
  - Tier 1 telekinesis in MVP; float and heavy throw are stretch.
- **Problems:** Theme still unknown.
- **AI tools used:** Claude (concept update, implementation notes).
- **Next:** Theme → close Concept stage → storyboard.

### Session 8: Thu Nov 12, 2026, 19:40 MST: Music plan + rules check

- **Goal:** Decide how music works and whether my existing tracks can be used.
- **Done:**
  - I'm a musician (SkyYamaha). I offered some of my tracks to build on, but I also want the music to be somewhat generated and non-repeatable.
  - Checked the jam rules: pre-jam assets are allowed only if declared as sourced assets, and those **aren't scored**. Also conflicts with this project's own rule that every asset is made during the jam.
  - Plan: compose new stems during the jam; assemble and vary them live with MetaSounds, driven by the run seed and the Fear value.
  - The same search turned up the likely theme: **"Director's Cut"** (from another participant's devlog; to confirm).
- **Decisions & why:**
  - **Stems and fragments over finished songs:** they're what generative assembly needs, and they're faster to make.
  - **Fear value drives the music mix**, the same single value as visuals, so audio and visuals creep in sync.
- **Problems:** Old tracks would cost scoring; theme still unconfirmed.
- **AI tools used:** Claude (web search on rules, music system design).
- **Next:** Confirm theme → fit concept to it → storyboard.

### Session 9: Thu Nov 12, 2026, 19:42 MST: Audio plan

- **Goal:** Define the sound of the game.
- **Done:**
  - Direction: something really eerie, dark ambient, plus brighter ambient. I'll compose new material, and we'll build generative algorithms to expand on it.
  - Wrote `docs/AUDIO_PLAN.md`.
  - Shared a YouTube reference for the spacing of the music and the mood (to be inspired by, not to copy): https://youtu.be/Jli-cviuC2c
- **Decisions & why:**
  - **Bright ambient = campus, dark ambient = Fear/Astral,** crossfaded by the Fear value, so the music tracks the slip.
  - **Phasing loops of odd lengths** as the base layer: the simplest way to get endlessly non-repeating ambience.
  - **Shuffle bag + follow-on table + scale-locked random walk** for melody: variety that still sounds composed.
  - Motifs from my older albums may be re-recorded as new material and noted in the devlog; old audio files aren't used.
- **AI tools used:** Claude (generative audio design, recording list).
- **Next:** Confirm theme → storyboard. Start recording audio stems in parallel when I'm away from the PC.

### Session 10: Thu Nov 12, 2026, 19:46 MST: Theme brainstorm

- **Goal:** Brainstorm how "Director's Cut" fits the game.
- **Done:** Brainstorm written to `docs/THEME_BRAINSTORM.md`.
- **Picks:**
  - Play with the ending, and foreshadow or reference a director's cut throughout the game.
  - Make it feel like the whole thing was an orchestrated movie.
  - Allude to a director / chairman.
- **Decisions & why:**
  - **The Chairman is never fully seen:** cropped portrait, signed memos, the empty director's chair creeping closer each take, the elevator "C" button that only lights at the end. Hints build dread without needing a character model.
  - **The Collector speaks for the Chairman** at the wrap party; the director's chair stays turned away.
  - **Yes = theatrical cut, No = director's cut** gives the choice a theme meaning.
  - Concept doc updated to v0.7. **Concept stage nearly closed**; only official theme confirmation is left.
- **AI tools used:** Claude (brainstorm, concept update).
- **Next:** Storyboard: campus map, opening scene, first Fear episode, ending beats.

### Session 12: Thu Nov 12, 2026, 19:52 MST: The workday

- **Goal:** Give the player a daily structure and a reason to move around.
- **Done:**
  - Messages signed "The Chairman" confirmed as a favorite.
  - **During the workday you check emails, do mandatory compliance training and team-building exercises, join video calls, walk around to talk to people, and attend meetings, and the puzzles appear as you do these activities.**
  - Setup progress: GitHub account is PhoenixLynx; Unreal project not created yet. Claude has no GitHub access from its workspace, so pushing waits on either connecting GitHub to Claude or pushing from my PC.
- **Decisions & why:**
  - **The day = 3 acts = 3 keys** (morning / midday / overtime). One structure gives pacing, escalation and a clear objective at every moment.
  - **Compliance Training is the Key 1 tutorial in disguise.** Teaching the XOR rule as a corporate training module is on-theme and solves onboarding.
  - **Phishing emails = false bits.** Cryptographic horror meets office life.
  - MVP keeps the inbox, training and one meeting or call; team building as its own sequence is a stretch goal.
- **AI tools used:** Claude (concept update, act structure).
- **Next:** Create the Unreal project + GitHub repo → storyboard.

### Session 13: Thu Nov 12, 2026, 19:54 MST: Theme confirmed + resources

- **Goal:** Lock the theme and gather free resources.
- **Done:**
  - **Theme confirmed: DIRECTOR'S CUT** (Epic's jammer email). Epic frames it as the feature that didn't make the cut. Our game is about the scenes cut from your life.
  - Favorites confirmed: the Chairman's camera always off in meetings, playing with time, and the morning / midday / overtime structure.
  - Sorted the partner perks in `docs/RESOURCES.md`.
- **Decisions & why:**
  - **Use:** PBRgen (materials, expires Nov 15), Hyper3D Rodin (3D props; redeem by Nov 16), fal (2D posters/portrait), Anthropic credits (claim, optional use).
  - **One 3D generator only** for consistent style.
  - **Skip WeLoveIndies music**: pre-made = unscored, and I'm composing everything.
  - **Diversion as a backup** if GitHub's free LFS storage runs out.
  - Codes and claim links kept out of the repo (Epic asked jammers not to share them).
- **Concept stage: ✅ complete.**
- **AI tools used:** Claude (email review, resource sorting).
- **Next:** Claim time-limited perks → create the Unreal project + GitHub repo → storyboard.

### Session 14: Thu Nov 12, 2026, 19:59 MST: Storyboard v0.1

- **Goal:** Turn the concept into scenes.
- **Done:**
  - `storyboard/STORYBOARD.md` v0.1: campus map (one floor: lobby, open office, conference room, break room, Corridor B, supply closet, Wellness Center, elevator), Act 1 (10 beats), Act 2 (9), Act 3 (6), ending (8).
  - Added `SourceArt/Blender`, `SourceArt/Ableton`, `SourceArt/Textures` with Git LFS for `.als .aif .aiff .flac .jpg .psd .exr`; ignored Ableton `Backup/` and `.asd` analysis files.
  - In parallel: I'm setting up the Unreal project, an Ableton project, and a Blender area.
- **Decisions & why:**
  - **One floor, reused by every act.** The same rooms get stranger instead of building new levels. Biggest scope saver.
  - **Act 1 order: Fear before the Key.** The flicker → energy → first chase comes before Compliance Training, so the scare lands in the first five minutes.
  - **Video-call captions show glyphs** instead of animating mouthed words: cheaper, still creepy.
  - **Source files committed in SourceArt/** so git history timestamps prove assets were made during the jam.
- **AI tools used:** Claude (storyboard draft, repo folders).
- **Next:** My review of the storyboard → finalize → prototype: player, grab, Fear value, loop corridor.
- **Review (20:05):** Approved the whole storyboard, mannequin coworkers included. Added: more hallways if there's time; keep it simple for now (stretch).
- **Additions (20:06):** Don't forget security guards and custodians. Turn recovered keys into cool-looking access passes. Claude folded these in: passes as the progression gates (closet → Wellness Center → "C"), guard and custodian behaviors, custodian props as debris. CONCEPT.md + STORYBOARD.md updated.
