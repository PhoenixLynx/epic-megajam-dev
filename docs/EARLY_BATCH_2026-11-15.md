# Early-morning batch: Sun Nov 15, 04:08–04:30 MST

Branch `early/2026-11-15`, off `main` at `709c997`. **Not merged and not on the PC**: the 8:45 debrief reviews it, merges it and syncs it.

## Changes

| # | What | Files | How to test |
|---|------|-------|-------------|
| 1 | **Packaging review.** The start map, game instance and most always-cook folders are right. **5 folders are missing**: `/Game/Ending`, `/Game/Memorial`, `/Engine/BasicShapes`, `/Game/FirstPerson`, `/Game/Input`. Without them the packaged build loses the ending art, and possibly party/penthouse shapes. `Config/` isn't in git, so this is the exact text to paste on the PC. | `docs/PACKAGING.md` | Add the lines (Unreal closed), package, run the .exe test in PACKAGING.md |
| 2 | **Coworker bump lines.** When a walking coworker gets within 75 cm of another one, it turns to them and says a short line ("Sorry. Sorry. After you.", "Oh! I thought you were 4470.", at the party "Oops. Cake."). Only when you're within 9 m to hear it, at most once every 25–45 s per coworker, never two at once, never while staring in Fear. | `TakeCoworker.h/.cpp` (`TickBumps`) | Rebuild, watch the cubicle aisles in Act 1 or the mingling at the party |
| 3 | **Second restroom stall**, west of the first: toilet, stall door (this one opens; swings in), its west wall and front pieces. The "inexplicable chair" moved out of the way to (3275, 2065). Checked with the mock harness: everything spawns at the intended spots. | `Scripts/Editor/dress_campus.py` | Run `dress_campus.py`, then `place_cpp_actors.py`, then visit the restroom and E the new stall door |
| 4 | Drafts: the itch.io page (title, tagline, description, controls, how it was made / AI tools, fonts, tags, settings) and a 10-shot, ~55 s gameplay video plan | `docs/ITCH_PAGE.md`, `docs/VIDEO_SHOTLIST.md` | Read, edit |
| 5 | Submission checklist: the gameplay video moved into **Required**; links to the new docs; a status line | `docs/SUBMISSION_CHECKLIST.md` | — |
| 6 | `place_cpp_actors.py`'s summary says Ctrl+1–4 for Fear (it still said 1–4). Devlog entry. | `Scripts/Editor/place_cpp_actors.py`, `devlog/DEVLOG.md` | — |

## Build-breaker check

- `TickBumps` uses `TActorIterator` (`EngineUtils.h` already included) and its own members only.
- Its locals don't shadow members (`Other`, `Face`, `Bumps`).
- The `TEXT()` lines are plain ASCII.
