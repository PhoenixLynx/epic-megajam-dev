# Overnight report: Sun Nov 15, 2026

Branch **`overnight/2026-11-15`** (from main at `fc9939a`). One commit per item, in brief order. Nothing pushed to main.
No Unreal compiler here: every change was reviewed by eye for the known build breakers (member shadowing, non-ASCII in
`TEXT()`, includes, braces), plus a brace/paren counter, plus a second independent read of the whole diff. That read found one
real break (unescaped quotes in an easter-egg line), now fixed in that item's commit. **Build it before anything else.**

| # | Item | Commit | Kind |
|---|------|--------|------|
| 1 | CONTINUE = latest act checkpoint | `0a0dde2` | C++ |
| 2 | Food in the fridge | `52081d0` | C++ (+ Blender script fix) |
| 3 | Wall clocks | `67ba227` | C++ (+ Blender script fix) |
| 4 | Corridor B extra long | `5b24b2b` | editor script, **dry run by default** |
| 5 | Memorial wall | `64cd90d` | editor script + C++ |
| 6 | Easter eggs (4) | `deffa4d` | C++ |
| 7 | Window to the outside | - | design only (below) |
| 8 | Cake in hand | `8caf32b` | C++ |

No header changes in any item, so Live Coding should handle all of it. Cherry-picking: items 2, 5 and 6 each add one line to
`CollectFunProps` (TakeAct2.cpp) next to each other. If you drop one of them, expect a one-line conflict there. Keep the
other lines.

---

## 1. CONTINUE = the latest act checkpoint
**Files:** `Private/TakeAct3.cpp` (TickAct3 autosave block, OnTitleChoice)

- Autosave now runs **only when the story reaches an act start**: Morning (the first playing tick after the title), Lunch,
  Overtime and Ward. The 60-second save is gone. Wrap and the ending are never saved. Log: `Checkpoint saved (beat N)`
  (0 Morning, 4 Lunch, 5 Overtime, 6 Ward).
- CONTINUE puts back what each act start sets up. The rest comes from the save as before.
  - **Morning:** the cold-open slate (same take number).
  - **Lunch:** the 12:00 time card, then "Lunch is served." x3.
  - **Overtime:** the 2:14 OVERTIME card, and the footsteps foreshadow re-armed (35 s).
  - **Ward:** the WARD card, then EnterWard as before.
- CONTINUE after the credits ("back to title") now clears the finished run first (`ResetRun`, then load). Before this,
  `bEndingSeen` stayed true, so that run never saved again.

**Test:**
1. START, watch the log for beat 0, then quit to the title and CONTINUE: you get the slate.
2. Dev-jump to lunch, wait for beat 4, restart and CONTINUE: 12:00 card and the PA burst.
3. Do the same for Overtime and Ward.
4. Check that nothing logs a save while you just walk around.

**Uncertain / note:**
- START overwrites the checkpoint with Morning (it *is* the latest checkpoint), so START by mistake loses an Act 3 save.
- The pause menu's manual **Save** still writes the same slot at any position. Remove that button if CONTINUE should only
  ever mean checkpoints.
- Saves made by last night's build can sit at any beat (e.g. Corridor). Those still load the old way.

## 2. Food in the fridge
**Files:** `Private/TakeAct2.cpp` (`StockTheFridge`, `SpawnFridgePart`, call in CollectFunProps),
`Private/TakeGrabComponent.cpp` (ToggleOpen), `SourceArt/Blender/make_office_props.py` (fridge_body)

**What was wrong:** `SM_FridgeBody`'s liner is one **solid box** that fills the inside, with its front face 7 cm behind the
opening. Open the door and you see a white wall. The food dress_campus.py put on the shelves is inside that box.

**Fix (runtime):**
- The body is hidden and rebuilt from engine cubes, at the same size (taken from its bounds) and with an open inside: steel
  shell, thin white liner panels, three glass shelves, the freezer lip. The front is whichever side the door is on.
- Six props go on the shelves: fruit bowl, water bottle, milk carton, 2 soda cans, lunch box. A yogurt note sits on the back
  wall (Inspectable).
- The food is Throwable but **not simulating** until the fridge door is first opened. ToggleOpen then turns physics on and
  binds `OnPropHit`.
- Old dress_campus food found inside the box is hidden (tagged `FridgeOld`).
- Fridge food doesn't "come back" and isn't put away in Act 3. Eaten means gone.
- The Blender script now builds the liner as panels, for a future re-export. Not needed tonight.

**Test:**
1. Kitchen, open the fridge: shelves with food.
2. Grab, eat and throw the items.
3. Before you open it, nothing should rattle or fall.
4. Log: `Fridge: rebuilt open (front +Y/-Y), 7 item(s)...`.

**Uncertain:**
- Material paths `/Game/Props/Meshes/M_Steel|M_PlasticWhite|M_Glass` (M_PlasticWhite/M_Glass are already used this way).
  A missing one shows as the default grey.
- If M_Glass is too transparent the shelves may look odd. It falls back to white only when missing.

## 3. The wall clocks
**Files:** `Private/TakeGrabComponent.cpp` (`namespace TakeWallClockHands`, called from TickClock),
`SourceArt/Blender/make_office_props.py` (wall_clock)

**What was wrong:** nothing drives the hands. They're baked into `SM_WallClock`, and the bake is off in three ways.
- **Hour hand:** the box is rotated **-67 deg** about Y, which lays it along the 4-to-10 o'clock line, while its centre is
  pushed toward 1 o'clock. So it crosses the pin at a strange angle.
- **Minute hand:** rotated for **16 min** (96 deg, not 84), with its centre 1.7 cm below the pin.
- **No second hand at all**, even though the 2:14 line says "the second hand isn't moving".

**Fix (runtime):**
- Each clock gets a clean face just in front of the baked hands, plus three hands (hour, minute, red second) and a pin. They
  pivot on the face centre, in the face plane.
- Hour and minute follow the game clock. At 2:14 frozen, the hour hand is at 67 deg and the minute hand at 84 deg.
- The second hand ticks in real seconds and stops dead when the clocks freeze.
- Front side: a short trace each way finds the wall behind. If no wall is found, it uses local +Y.
- The Blender script math is fixed too (hands start at the centre and are turned +angle), for a future re-export.

**Test:**
1. Look at any wall clock in Act 1: the second hand ticks, the minute hand sits at about 9:55.
2. After beat 2.6, every hand shows 2:14 and the second hand stops.
3. Log: `Wall clock ...: new hands (face looks along +/- local Y)`.

**Uncertain:** if a clock's log says `-` but its face really looks along +Y (or the reverse), the new face is behind the
clock. Tell me which clock and I'll pin it. The new face is a 30 cm disc 1.8 cm proud of the rim, so it may read slightly
thick up close.

## 4. Corridor B, extra long (script only, not run)
**Files:** `Scripts/Editor/extend_corridor_b.py` (new)

This is too many interlocking scripts to do blind, so it's a script with **`DRY_RUN = True`**. A dry run only logs what it
would do (filter the log on `[TAKE 4471] CorridorB`).

When run for real (`EXTRA = 1500`, so the corridor goes from 20 m to 35 m):
1. **Moves** everything on the ground floor past the corridor's south end (x 700-2500, y >= 5640, z > -700) south as one
   block. That's the Wellness Center (walls, floor, door + PassBlock=2, beds, trust-fall X and circle, clock, sign, pass
   reader, ceiling, lights, ambience) and the Act 1 stalker + its marker.
2. **Stretches** the corridor's two side walls, floor and ceiling.
3. **Adds** corridor LED strips + RectLights every 250 cm (copies of the corridor's own, FearLight tags included) and five
   posters copied from GB_Poster_0..2 **without** the LoopPoster tag.
4. **Moves** the Act 1 wisp to the new end (`MOVE_WISP_TO_END`).
5. Runs as one undo step, does not save, and leaves a `GB_CorridorB_Extended` marker so it won't run twice.

**Left alone on purpose:**
- **The loop:** posters 0-5, the trigger at y 4950 / offset -1000, and the CorridorB beat trigger at y 3780. The new stretch
  is past the trigger, and fog hides the far end while looping.
- **The ward (z -1500):** it doesn't move. The trust fall teleports (EnterWard), so it never needed the floors to line up,
  and all the ward coordinates in C++ stay valid.
- **C++:** no changes needed. The Wellness pieces are found by tag.

**To do it:**
1. Run once and read the list.
2. Set `DRY_RUN = False` and run again.
3. Walk it in PIE.
4. Ctrl+S, then **Build > Build Paths**.

**Risks:**
- Re-running build_greybox_campus / dress_campus / add_ceilings puts their pieces back at the old positions. Run this script
  again after them (delete the marker first).
- I couldn't see the level, so check that nothing else sits in the moved box.

## 5. Memorial wall
**Files:** `Scripts/Editor/import_memorial.py` (new), `SourceArt/Textures/Memorial/README.md`, `Private/TakeAct2.cpp`
(`HangMemorialWall`), `Private/TakeGrabComponent.cpp` (inspect caption)

- The import gives the PNGs **numbers**, not their file names: `T_Memorial_01..`, plus `MI_Memorial_01..` (M_ScreenTexture,
  `Tex`). Sorting is by file name, and stale higher numbers are deleted.
- Runtime loads `MI_Memorial_01..09` and skips any that are missing.
- **Where:** the lobby's **south wall, east half** (x 2340-2670, face y 1690), three per row in two rows. That's clear of
  the reception desk, the elevator and the portrait (all on the north side or the desk itself). Photos 7-9 go on the south
  wall's west half, over the armchairs.
- Each photo is 96x54 in a dark frame. A wood plaque under the east half reads **HALCYON: OUR PEOPLE**.
- E on a photo shows it full size with a caption.
- Nothing is hung until at least one MI exists.

**Test:**
1. Copy the screenshots into `SourceArt/Textures/Memorial`.
2. Run `import_memorial.py`.
3. Play and go to the lobby, facing south: the frames are there. E on one.

**Uncertain:**
- Is this the wall you meant at (2257, 1193)? It's the blank one beside the desk. If you meant the east wall (HALCYON sign),
  change the `Slots`.
- Packaged builds: make sure `/Game/Memorial` gets cooked (same as `/Game/Ending`), since it's only loaded by path.

## 6. Easter eggs
**Files:** `Private/TakeAct2.cpp` (UseThing, Vend, `StickNoteUnderKeyboard`), `Private/TakeStory.cpp` (Act 3 tag on the
microwave), `Private/TakeGrabComponent.cpp` (prompts, mirror captions)

1. **Microwave, Act 3.** When lunch is put away, the kitchen microwave gets `Use=Microwave`. E: "...a lunch box with 4470
   on the lid. Turning, slowly." / "0:00 since 2:14" / ... / "4470's lunch. Still spinning." It stops being grabbable in
   Act 3 only.
2. **Sticky note half under 4471's keyboard** (the keyboard is DeskFixed, so about 4 cm of the note shows at the front
   edge). E reads three lines in turn: "DON'T LOOK UNDER THE KEYBOARD", then a reply signed 4470, then one for `{NEXT}`.
3. **Vending machine code.** The 8th vend (after every line has been seen once, calm and not frozen): "You key in 4-4-7-1...
   a mug. It says 4472 on it." It drops a real mug.
4. **Restroom mirror.**
   - During the party, your reflection wears a party hat.
   - In the next take after YES, it has a DIRECTOR lanyard.
   - After NO, it mouths "AGAIN".

**Test:** each logs `Easter egg: ...` the first time. For the mirror after an ending, use PLAY AGAIN from the credits.
**Uncertain:** the note uses `MI_Sticky_4470` (whatever is drawn on that sticky shows). The microwave "ding" reuses
`sfx_badge_ok`.

## 7. A window on the open office west wall (design only, not built)
Not built, because the spot is taken: the west wall (x 800) already has the **bulletin board at (820, 2700), y 2600-2800**,
exactly where the note was, plus the Smile poster at y 2150 and the janitor closet in the SW corner. The wall is one long
merged greybox segment, so a real opening means changing `wall_segments` in build_greybox_campus.py (a "W" door entry on
OpenOffice) and rebuilding. Too much for tonight.

**Proposal: a fake window.**
- Move the bulletin board to y ~3200.
- At (812, 2650, 165), hang a 260x150 frame (4 thin cubes, M_MetalLight) with a sill.
- Behind the glass, put an **emissive backdrop panel** (unlit material, texture `T_Window_Outside` from Sol: a courtyard with
  actors on break, phones out, someone vaping, someone lounging on a planter, still in costume). A faint second panel
  10 cm further back with a parallax tree layer sells the depth.
- Optional: one or two flat "cutout" coworkers on cards between the layers, sliding a few cm now and then.
- Act 3: swap to a night version. With Fear: the people outside are all turned toward the window.
- It's all runtime spawned, one function, about 60 lines.

Say the word and send the texture, and I'll build it.

## 8. Cake in hand
**Files:** `Private/TakeDirector.cpp` (`GivePlateOfCake`, CutTheCake, StartRefusal)

- After the first slice (CutTheCake), **every other mingling coworker** (not the Collector) holds a plate. That's a white
  plate + slice + frosting, attached to the capsule, 32 cm in front, 14 cm to the right, 102 cm off the floor, with no
  collision and no shadow. Every third plate has a shorter, bitten slice. Log: `Cake cut: N coworker(s) holding a plate`.
- In the **NO ending**, the stalker lurking by reception holds a plate too, at 125 cm, untouched. Its AI is not changed.

**Test:**
1. At the party, have a slice: about half the crowd now has plates as they mingle.
2. Say NO: the dark shape by reception is holding a plate.

**Uncertain:** the hand height assumes the coworkers' and stalker's capsule centre sits at half their height (true for the
Collector's hat placement). If a plate floats or sinks, change `102.f` / `125.f`.

---

### Not touched
Endings flow, Fear Director tuning, music cues, the stalker's AI, DataTables, `Content/`, and devlog/DEVLOG.md.
