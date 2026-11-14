# Saturday morning: merging the overnight runs

## Overnight run 1 (branch `overnight-run1`): Act 1 progression
Not compiled yet. Nothing was copied into the PC project (no folder was connected to the bridge tonight), so it only exists on the branch.

### Merge
1. In Claude: "review and merge overnight-run1" (run 2's branch `overnight-run2` builds on it; if it exists, merge that one instead, since it contains run 1).
2. Or by hand, in the repo: `git fetch && git checkout main && git merge origin/overnight-run1`, then copy into `DirectorsCut_58`:
   - `Source/DirectorsCut_58/` (new: `TakeStory.cpp`, `TakeWisp.h/.cpp`, `TakeBeatTrigger.h/.cpp`; changed: GameInstance, GrabComponent, Coworker, FearDirector, LoopTrigger, Widgets, PauseMenu, SaveGame)
   - `Data/Text/DT_NPCLines.csv` + `make_text_tables.py`
   - `Scripts/Editor/place_cpp_actors.py`

### Rebuild
1. Close Unreal. Move `Binaries/Win64/UnrealEditor-DirectorsCut_58.dll` into `_to_delete/` (otherwise the old build loads).
2. Open the `.uproject` → Yes to rebuild. If it fails, paste the first error to Claude.
3. Run `place_cpp_actors.py` (re-imports the 97 dialogue lines and places `CPP_Beat_ConferenceRoom`, `CPP_Beat_CorridorB`, `CPP_Wisp_Act1` + makes `M_Wisp`). The log should end with `Act 1 story actors: Beat_ConferenceRoom, Beat_CorridorB, Wisp_Act1`.

### Test, in this order (Output Log filter: `TAKE 4471`)
1. **Cold open:** Play: black, clapper, **TAKE 1**, fade in, PA "Good morning, team…", then "New objective: Check in at Reception (Lobby)". The badge panel shows `NEXT · …`.
2. **Receptionist:** E pages through 4 lines (prompt "E  Continue"); after the last, the "!" goes and the objective becomes "Finish your badge tasks before noon". Guard and custodian: their own conversations; the custodian's changes once you have pass 1.
3. **Shortcut:** console `TakeBeat 1` (all tasks done) → objective "Attend the 10:00 meeting".
4. **Conference Room:** walk in: lights flicker, colours drain a little, "…Where is everyone?", objective "Something is at the end of Corridor B." Log: `Beat -> Corridor (the slip)`.
5. **Corridor B:** PA "perfectly safe", Fear creeps; walk on: you loop (`Looped (1)`), "…Wasn't I just here?". Find the changed poster → E Look closer → "That's the one" (`Loop broken`).
6. **Wisp:** a mint light at the end of the corridor. E "Reach for the light" → colour comes back, objective "Return to your desk."
7. **Rewind:** `TakeRewind` → "CUT. AGAIN." → TAKE 2 → PA line.
8. **Save/Load** mid-corridor: the objective and beat come back.

If a box is in the wrong place, move `CPP_Beat_*` in the level (Box Half Size is in Details). The loop needs Fear ≥ 0.3 during the beat (`Beat Loop Fear` on the Fear Director).

---

# Saturday morning runbook (Nov 14)

Made overnight Friday. Everything below is already on your PC.

## 0. The overnight package
- Did it finish? Look in the folder you picked (e.g. `Builds\Fri_night\Windows\`) for `DirectorsCut_58.exe`.
- **Yes:** double-click it and walk around for 30 seconds. That box on the submission checklist is basically ticked.
- **No / error:** tell Claude "package failed". The log is in `Saved\Logs`, so there's nothing to copy.

## 1. Rebuild (new C++ overnight)
1. Close Unreal.
2. (Done Friday 22:05: the package had finished, so the new C++ is already in `Source/` and the old DLL has been moved aside.)
3. Open `DirectorsCut_58.uproject` → **Yes** to rebuild. If it fails, say "failed".

## 2. Run, in this order
1. `import_jam_assets.py`: also imports the **rigged mannequins + 6 animations** into `/Game/Characters/Mannequin`.
2. `dress_campus.py`: the coffee machine is now an E-to-use station (not a physics prop).
3. `place_cpp_actors.py`: re-imports the dialogue (75 lines) and training.

## 3. Play and check
- **Badge task list** top left: Check email · Get coffee · Complete Compliance Training.
- **E at the coffee machine:** "You pour a coffee. It's exactly the right temperature. It always is."
- **Coworkers:** greet you the first time, then cycle through their small talk with no repeats until they've said everything. After your coffee some of them notice. The receptionist, guards and custodians have their own lines.
- **Finish all three tasks:** the PA sends you to the 10:00 meeting.
- **P** (or Esc in the packaged game) pauses with the **HALCYON break menu**: Resume · Tasks · Settings (volume, graphics Low–Epic, fullscreen/borderless/windowed, controls) · Save · Load · Quit. Save, quit Play, Play again, P → Load: you're back where you saved, with the same tasks and passes.
- Open `/Game/Characters/Mannequin` and double-click an animation (Idle, Walk, SitDown, SitType, Reach, Stare) to preview it 📸.

## 4. Today's plan (checkpoint: Act 1 playable by 2 PM)
1. Swap the coworkers' statues for the animated mannequin (walk when moving, type at desks, stare at high Fear).
2. The 10:00 meeting beat → first "slip" → Corridor B loop.
3. Afternoon: Act 2 start (Supply Closet Key Lock with hidden glyphs).
