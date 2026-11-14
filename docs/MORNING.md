# Saturday morning runbook (Nov 14)

Made overnight Friday. Everything below is already on your PC.

## 0. The overnight package
- Did it finish? Look in the folder you picked (e.g. `Builds\Fri_night\Windows\`) for `DirectorsCut_58.exe`.
- **Yes:** double-click it and walk around for 30 seconds. That box on the submission checklist is basically ticked.
- **No / error:** tell Claude "package failed". The log is in `Saved\Logs`, so there's nothing to copy.

## 1. Rebuild (new C++ overnight)
1. Close Unreal.
2. Say "good morning" to Claude. The overnight C++ is waiting in `_to_delete\overnight\Source`, kept out of the project so it couldn't break the overnight package. Claude copies it in and moves the old DLL aside.
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
- Open `/Game/Characters/Mannequin` and double-click an animation (Idle, Walk, SitDown, SitType, Reach, Stare) to preview it 📸.

## 4. Today's plan (checkpoint: Act 1 playable by 2 PM)
1. Swap the coworkers' statues for the animated mannequin (walk when moving, type at desks, stare at high Fear).
2. The 10:00 meeting beat → first "slip" → Corridor B loop.
3. Afternoon: Act 2 start (Supply Closet Key Lock with hidden glyphs).
