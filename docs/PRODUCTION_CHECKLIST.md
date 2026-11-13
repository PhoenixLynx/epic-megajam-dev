# Production checklist: TAKE 4471 (Director's Cut)

The single list of everything the game needs, who does it, and when it's "done".
Claude updates this every session. `[x]` = done · **MVP** = must ship · *Stretch* = only if time.

**Owners:** 🎬 **You** (phoenixlynx) · 🤖 **Claude** · 🤝 **Both** (Claude builds or preps, you review or test in-editor)

**Deadline:** Sun Nov 15, 6:59 PM Phoenix. **We submit by 5:00 PM.**

---

## ⏱ Checkpoints (where we should be)

| When | Checkpoint | If we're behind |
|------|-----------|-----------------|
| **Fri 11 PM** | Campus built, Fear slider works, grab/throw works, git pushing from PC | Skip MCP; build by hand with recipes |
| **Sat 2 PM** | **Act 1 playable start to finish** (desk → loop → stalker → Key 1 → Pass 1) | Cut the meeting and team-building beats |
| **Sat 10 PM** | **Whole game playable, rough:** Acts 1–3 + ending, even if ugly | Act 3 = Wellness Center transforms in place, no separate Hospital |
| **Sun 12 PM** | **Content freeze.** No new features, only fixes, art, audio, polish | Ship what works |
| **Sun 2 PM** | **Packaged Windows build tested** on a clean run | Fix crashes only |
| **Sun 5 PM** | **Submitted** | 1h59m safety buffer |

**MVP "done" means:** someone can play from the TAKE 1 slate to the wrap party in ~15 minutes, get chased, solve three keys, and make the final choice.

---

## A. Setup
- [x] Repo, LFS, folders, devlog, checklist: 🤖
- [x] Concept, storyboard, audio plan, resources: 🤝
- [x] UE 5.8 project `DirectorsCut_58` (First Person template): 🎬
- [x] Plugins enabled in .uproject (Python, Editor Scripting, Unreal MCP, All Toolsets): 🤖
- [ ] Preflight script passes (`0 FAIL`): 🎬 runs → 🤖 fixes. **Fri**
- [ ] Unreal MCP connected to Claude (desktop app or Claude Code): 🤝. **Fri**
- [ ] Git on the PC: repo in `DirectorsCut_58`, first Unreal push with LFS: 🤝. **Fri**

## B. Core systems (prototype). MVP
- [ ] Grey-box campus built (`build_greybox_campus.py`): 🎬 runs. **Fri** · Done: walk every room in Play.
- [ ] Fear materials (`build_fear_assets.py`): 🎬 runs. **Fri**
- [ ] **Fear Director** (one 0→1 value → materials, post-process, fog, letterbox): 🤝. **Fri** · Done: slider rots the campus; keys 1–4 set stages.
- [ ] **Grab / throw** (+ telekinesis reach at 1 soul bit): 🤝. **Fri** · Done: throw the mug across the office.
- [ ] **Corridor B loop** (teleport + changed poster + break the loop): 🤝. **Sat AM** · Done: can't tell you looped; using the odd poster frees you.
- [ ] **Stalker** (stalk → chase → stun when hit; speed scales with soul bits): 🤝. **Sat AM** · Done: it hunts you; a thrown chair stuns it.
- [ ] **Rewind on capture** ("CUT. AGAIN." slate, take number, something changed): 🤝. **Sat AM**
- [ ] **Key Lock** in Blueprint (port of `keylock.py`) + terminal UI: 🤝. **Sat AM** · Done: Key 1 tutorial solvable; decoys corrupt.
- [ ] **Run seed** (clock-based, dev override): 🤖 spec ✔ → 🤝 build. **Sat**
- [ ] **Soul Sight** (meter, flicker at 0 bits, hold-to-see, check on locks): 🤝. **Sat**
- [ ] **Access passes** (badge slots; doors check passes): 🤝. **Sat**
- [ ] Health + damage (stalker grab hurts; rewind keeps damage): 🤝. **Sat**
- [ ] *Stretch:* fire extinguisher, hot-coffee burn, float, Dissolve/Patch

## C. Act 1: Morning. MVP · target **Sat 2 PM**
- [ ] TAKE 1 cold open (clapper, 8:59 → 9:00, PA): 🤝
- [ ] Badge HUD + task list (from mockup): 🤝
- [ ] Email inbox on the desk monitor (from `DT_Emails`): 🤝
- [ ] Coffee run + coworker small talk: 🤝
- [ ] First involuntary Soul Sight flicker → energy wisp in Corridor B: 🤝
- [ ] Corridor B Fear creep → loop → first stalker → escape or CUT: 🤝
- [ ] Compliance Training slideshow → Key 1 → Pass 1 → telekinesis: 🤝
- [ ] Clock jumps to 12:00, "Lunch is served": 🤝

## D. Act 2: Midday. MVP-lite
- [ ] Repeating PA + new tasks: 🤝
- [ ] Video call screen (frozen tile, "C" tile, glyph captions): 🤝
- [ ] Clue pieces placed (email footer, captions, whiteboard) + 1 agent with a false clue: 🤝
- [ ] Clocks freeze at 2:14; mannequins move when unseen: 🤝
- [ ] Key 2 in the Supply Closet (hidden target, Sight check) → Pass 2: 🤝
- [ ] Call sheet + director's chair appear: 🤝
- [ ] *Stretch:* full meeting scene, more coworkers, guard badge checks

## E. Act 3 + ending. MVP-lite · target **Sat 10 PM**
- [ ] Team-building trust fall → floor hides → land in the Hospital: 🤝
- [ ] Ward: script-page charts (clues + fakes), REC light, constant high Fear: 🤝
- [ ] Key 3 → Pass 3 → elevator "C" lights: 🤝
- [ ] **Wrap party** room: confetti, NPCs, stalker unmasked, frozen clock, director's chair turned away: 🤝
- [ ] **Collector offer + Yes/No** → theatrical cut / director's cut → Sight flicker → cut to black: 🤝
- [x] Credits + ending text written (`DT_Credits`, `DT_Ending`): 🤖
- [ ] Credits + ending widgets: 🤝
- [ ] *Stretch:* second stalker, nested loops

## F. Art
- [x] Glyph alphabet (8 × lit/unlit + hidden): 🤖
- [x] UI mockups (Key Lock, inbox, HUD): 🤖
- [ ] Import glyphs + build UI widgets from mockups: 🤝. **Sat**
- [x] 16 grey office props from a Blender script (`SourceArt/Blender/make_office_props.py`) + FBX exports: 🤖
- [ ] Import props (`import_jam_assets.py`) and swap into the grey-box: 🤝. **Fri/Sat**
- [ ] *Polish:* hero props refined in Blender / Rodin: 🎬
- [ ] Mannequin coworker + stalker models: 🎬 (or Rodin). **Sat**
- [ ] PBR materials from PBRgen (carpet, tiles, + grime versions): 🎬. **Before Sun** (perk ends Nov 15)
- [x] Posters (6 + loop twins) + Chairman portrait drafts: 🤖
- [ ] *Polish:* refine the Chairman portrait: 🎬
- [x] Call sheet + 4 script-page (chart) textures: 🤖
- [ ] Lighting pass (bright campus vs Fear vs Hospital): 🤝. **Sun AM**
- [x] TAKE slate + title screen + loading screen designs: 🤖
- [ ] Build them as widgets: 🤝. **Sat**

## G. Audio
- [x] Placeholder SFX (11): 🤖
- [ ] Bright ambient stems (campus): 🎬. **Sat**
- [ ] Dark ambient stems + chase stems + stingers: 🎬. **Sat–Sun**
- [ ] Soul Sight shimmer layer: 🎬
- [ ] Ending "corporate anthem" (+ one out-of-tune stem): 🎬
- [ ] MetaSounds: Fear-driven crossfade + phasing loops + shuffle: 🤝. **Sun AM**
- [ ] Replace placeholder SFX with real recordings: 🎬. **Sun AM**
- [ ] *Stretch:* scale-locked random-walk melody

## H. Polish. **Sun AM**
- [ ] Playtest full run ×2; fix blockers: 🎬 plays → 🤖 diagnoses from logs
- [ ] Difficulty tuning (stalker speed, Fear timing, puzzle sizes): 🤝
- [ ] Controls hint on screen; pause/quit menu: 🤝
- [ ] Remove dev keys / dev seed override from shipping: 🤝

## I. Packaging + submission. **Sun**
- [ ] Windows **Development** build packaged and launched once from the zip: 🎬. **by 2 PM**
- [ ] itch page: title, description, controls, screenshots, cover image, **public**: 🤖 drafts / 🎬 posts. **by 4 PM**
- [ ] Devlog linked (repo made public or devlog posted on itch): 🎬
- [ ] Only the Unreal Engine logo used; no other brands or IP: 🤝 check
- [ ] Pre-jam material declared (none planned) + sourced assets listed (none planned): 🤖 draft
- [ ] "Tools you used" field (Claude, Unreal MCP, Rodin/PBRgen/fal if used): 🤖 from the ledger
- [ ] Glass Box: devlog submitted with the entry: 🎬
- [ ] **Submitted by me (a human)**: 🎬. **by 5 PM**

## J. Record-keeping (all jam)
- [ ] Devlog entry + AI ledger every session: 🤖
- [ ] Screenshot/clip at each 📸 milestone: 🎬 captures, 🤖 links
- [ ] Commit after every meaningful change; push each session: 🤖 (docs) / 🎬 (Unreal assets, from PC)
- [ ] Update this checklist each session: 🤖

---

## Claude can do these any time, without the editor
1. ~~Act 2 + 3 text, call sheet, credits, Collector, endings~~ ✔
2. ~~Title / TAKE slate / loading designs~~ ✔
3. ~~Posters + Chairman portrait~~ ✔ (call sheet + script-page textures still to do)
4. ~~Blender Python scripts for office props~~ ✔ (16 props)
5. itch page copy + cover image draft + devlog summary for the Glass Box.
6. More builder scripts: wrap party room, mannequin placement, clue placement.
