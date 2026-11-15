# Overnight brief: Sat Nov 14 → Sun Nov 15, 2026

Jam deadline: **Sun Nov 15, 6:59 PM MST**. The director (PhoenixLynx) playtests in the morning.
Main is at the commit that added this file. Everything on main has been copied to the director's PC
(`C:\Users\casse\Documents\Agentic\EpicJam\DirectorsCut_58`, not a git checkout) and is being rebuilt there.

## Ground rules for the overnight run
- Work on a branch **`overnight/2026-11-15`** made from main. **Never push to main.** The morning session reviews and merges.
- There is **no Unreal compiler here.** Every change must be reviewed by eye for the things that broke builds before:
  - a local variable or lambda parameter with the same name as a class member (MSVC C4458/C4456 are errors in UE) — check `Public/TakeGrabComponent.h`, `TakeCoworker.h`, `TakeFearDirector.h` before naming locals;
  - non-ASCII characters inside `TEXT("...")` (use `\u00B7` etc.);
  - missing includes for every engine type you touch; balanced braces/parens (a quick Python counter is fine).
- Keep changes **runtime C++** (spawned at play) or **editor Python scripts** the director runs (Tools > Execute Python Script). Don't assume a script has been run.
- Keep each item small and self-contained, one commit per item, so the morning can take some and drop others.
- Match the code style: long explanatory comments in plain English, `UE_LOG(LogTake, Log, TEXT("[TAKE 4471] ..."))` for anything worth seeing in the log.
- Finish by writing **`docs/OVERNIGHT_REPORT_2026-11-15.md`**: per item, what changed (files), how to test it in game, and anything uncertain.

## Context you need
- Code: `Source/DirectorsCut_58/` (UTakeGrabComponent is split across TakeGrabComponent.cpp, TakeAct2.cpp, TakeAct3.cpp, TakeStory.cpp, TakeDirector.cpp).
- Level layout comes from `Scripts/Editor/build_greybox_campus.py` and `Scripts/Editor/dress_campus.py` (positions in cm; floor at z 0, ward at z -1500).
  Rooms: elevator car x 1800-2400, y 0-500; lobby/reception around (2100, 1050), reception desk ~(2100, 1400);
  kitchen x 3050-3750, y 550-1300; conference room x 500-1400, y 800-1400; open office x 1650-3150, y 1950-3500;
  Corridor B runs south from ~(1400, 3600) to the Wellness Center; supply closet ~(900-1100, 3400-3700).
- Playtest notes are in the director's `Saved/PlaytestNotes.txt` (not in the repo); the relevant ones are quoted below.
- Devlog style: `devlog/DEVLOG.md` (don't edit it overnight; the morning session will).

## The work, in priority order

1. **CONTINUE = the latest act checkpoint.** *"continue should reload the latest save checkpoint, and it should be after each act for now."*
   Today `TickAct3` (TakeAct3.cpp, top) autosaves on every beat change and every 60 s. Change it to save **only at act starts**
   (Act 1 start/Morning, Lunch = Act 2, Overtime = Act 3, Ward) and not periodically, and not during the ending.
   Make sure CONTINUE (OnTitleChoice in TakeAct3.cpp) restores the right state for each of those beats (the ward one calls EnterWard already).

2. **Food in the fridge.** *"have items in fridge"* (kitchen fridge near (3650, 680); the fridge is `SM_FridgeBody` + an `Openable` `SM_FridgeDoor`, see dress_campus.py).
   At play, put 5-7 props on its shelves using the body's bounds: milk carton, lunch box, soda cans, water bottle, fruit bowl,
   and a note using `MI_FridgeNote_Yogurt` / `MI_FridgeNote_Memo` (they exist). They should be pick-up-able like other food (tag `Throwable`, physics,
   `OnPropHit` bound, `CanCharacterStepUpOn = ECB_No`) but must not fall out before the door opens: spawn them non-simulating and turn physics on when
   the fridge door is opened (ToggleOpen in TakeGrabComponent.cpp), or keep them kinematic until grabbed.

3. **The wall clocks.** *"weird clock hands"* / *"clock looks funny, the hands and second hands"*. Find where the `SM_WallClock` hands are driven
   (grep `Clock` in the Private folder) and check pivots/axes: hands should rotate around the clock face's centre, in the face's plane,
   hour hand at 2:14 when frozen. Fix the math; describe in the report what was wrong.

4. **Corridor B, extra long.** *"let's make this hallway extra long"* (Act 1, around (1385, 5268)). Read how Corridor B and its loop trigger
   (`TakeLoopTrigger`, 6 posters) are built. Write an editor script `Scripts/Editor/extend_corridor_b.py` that lengthens the corridor
   (walls, floor, ceiling, lights, posters spaced along it) without breaking the loop or the Wellness Center doors at the end.
   If it's too risky to do blind, write the plan in the report instead and stop.

5. **Memorial wall.** *"some artwork on walls with memorial shots from the game"* and *"let's do something with this wall"* (lobby, near (2257, 1193)).
   - `Scripts/Editor/import_memorial.py`: imports `SourceArt/Textures/Memorial/*.png` as `/Game/Memorial/T_Memorial_*` and makes
     `MI_Memorial_*` from `/Game/Props/Materials/M_ScreenTexture` (texture parameter `Tex`).
   - Runtime: in the lobby, hang framed photos (thin cube frame + a plane/cube with the MI) for every `MI_Memorial_*` that exists
     (LoadObject; skip missing), with a small plaque: "HALCYON: OUR PEOPLE". Find the right wall from dress_campus.py; keep clear of the
     reception desk, the elevator, and the Chairman's portrait.
   - The morning session will copy the director's screenshots into `SourceArt/Textures/Memorial`.

6. **Easter eggs** (*"have some easter eggs?"*), small and in the game's voice (deadpan corporate, film-set reveals, the 4470/4471/4472 numbers):
   e.g. the microwave in Act 3 ("4470's lunch. Still spinning."), a sticky note under 4471's keyboard, a vending machine code,
   the mirror in the restroom after the party. 3-4 is plenty. Use the existing `Use=` / `Inspectable` systems.

7. **(Optional) A window to the outside** on the open office's west wall (~(1091, 2622)): *"this wall would be nice for a window... outside ppl could be
   taking a break, on their phone/vaping/lounging... showing more actors out of character"*. Design it in the report (fake window: emissive panel with a
   backdrop texture to come from Sol, or a real opening if the wall is separate geometry) and only build it if it's clean.

## Don't
- Don't change the endings' flow, the Fear Director's tuning, music cues or the stalker's AI: those were just playtested.
- Don't add new DataTable rows (they need a re-import); put new text in code like the rest of tonight's work.
- Don't touch `Content/` binaries or `.uasset` files.
