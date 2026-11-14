# C++ gameplay systems

Module `DirectorsCut_58` in `Source/` (switched on Fri Nov 13, 18:18).
About 2,300 lines. The Key Lock core is tested outside Unreal (`Scripts/Reference/test_keylock_core.cpp`, 20,000 seeds × 3 keys); everything else compiles for the first time on the PC.

| Class | What it does | Setup needed |
|-------|-------------|--------------|
| `UTakeGameInstance` | Run seed (clock-based, never shown; `DevSeedOverride` for bugs), take number, soul bits (0–3) = passes. **Console commands.** | Set as Game Instance class (Claude edits DefaultEngine.ini) |
| `ATakeFearDirector` | The single Fear value → `MPC_Fear`, post-process (saturation, vignette, grain, tint), fog, light dim/flicker, letterbox amount, stage events; turns the loop on at 0.5 and wakes the stalker at stage 4. **Previews in the editor** when you drag Fear. Gives the player the grab component at Play. | None: auto-wires everything |
| `UTakeGrabComponent` | **One E key for everything** (batch 2): look at a coworker = talk, your desk/monitor (tag `Terminal`) = sit and log in, a door/fridge (tag `Openable`, `OpenYaw=N`) = swing open/closed, a loose prop = pick up/drop. **Right Mouse** throw. On-screen prompt ("E  Pick up"). Hides the template arms. **1–4 / 0** = Fear 0.25 / 0.5 / 0.75 / 0.95 / calm. Reach 3 m, 8 m with 1+ soul bit; > 40 kg needs 3 bits. | None: added to the player automatically |
| `UTakeTerminalWidget` | HALCYON Mail on desk #4471, built in C++ (no Widget Blueprint). Reads `/Game/Data/DT_Emails` (Act 1), fills `{EMP}`, `{TAKE}` and glyph marks; falls back to two built-in emails. **E or Tab** logs out (Esc also works in a packaged build, but in the editor Esc stops Play). | `place_cpp_actors.py` imports the DataTable |
| `UTakePromptWidget`, `UTakeIndexedButton` | The prompt under the crosshair; inbox buttons. | None |
| `ATakeCoworker` | Mannequin coworkers that look busy: typing at their desk (lean and glance), walking to `WorkSpot` points (coffee machine, fridge, cooler, printer, filing), waiting there, walking back. Bump into one or press E: a random `DT_NPCLines` line for its Role (Act 1, `Bump` lines added) floats above its head for 3.5 s. Receptionist and guards stay put. Two are secretly agents. **Above Fear 0.75 they stop pretending:** freeze and stare while you look, creep toward you when you don't. | `place_cpp_actors.py` swaps the NPC mannequins and adds the WorkSpots; nav mesh |
| `ATakeLoopTrigger` | Corridor B loop: silent teleport back 10 m (keeps speed + view). Each loop swaps one poster to its `_Changed` material. `TryBreakLoop(poster)` breaks it. | None: finds `LoopPoster`-tagged posters and `MI_*_Changed` materials |
| `ATakeStalker` | Dormant + hidden → **Stalk** (lurks 6 m behind you) → **Chase** when you look straight at it or after 6 s → **Stunned** 1.5 s when a `Throwable` hits it at > 4 m/s → catch = rewind. Chase speed 350 + 80 per soul bit. `Slow()` for the extinguisher. Body = `SM_Stalker`. | Nav mesh (builder adds it; press **P**) |
| `UTakeRewindSubsystem` | "CUT. AGAIN.": fade to black, take +1, Fear 0, stalker reset, back to `PlayerStart` tagged `Desk4471`, one `ChangeOnRewind` object turned, fade in. `OnRewind(Take)` event for the slate widget. | None |
| `ATakeBeatTrigger` | *(overnight run 1)* Invisible box with a `BeatName`; tells the player's Take Grab "you walked into ConferenceRoom / CorridorB". The story code decides what happens (the 10:00 slip, the Corridor B PA + creep). | `place_cpp_actors.py` places two |
| `ATakeWisp` | *(overnight run 1)* Soul-energy wisp: emissive sphere + point light, bobbing. Only there during its beat (`VisibleDuring`, default Corridor). E picks it up: Fear drains, objective "Return to your desk." | `place_cpp_actors.py` (on `GB_SoulEnergy_Act1`, makes `M_Wisp`) |
| Story (`TakeStory.cpp`) | *(overnight run 1)* The story half of `UTakeGrabComponent`: cold open slate + PA, objectives (`UTakeGameInstance::Objective`, `ETakeBeat`), story NPC conversations (`ATakeCoworker::Talk`, DT_NPCLines `Story1..`, `Pass1Story1..`, `DoneStory1..`, `SlipStory1..`), loop hints + breaking the loop by looking closer at the changed poster, rewind slate. | None |
| Act 2 doorway (`TakeStory.cpp`) | *(overnight run 2)* Beat 1.10: back at desk #4471 after the wisp + training → clock jumps to **12:00**, PA **"Lunch is served."** (then word for word twice more, and again now and then all Act 2: beat 2.1), the badge switches to the **Act 2 task list** (12:30 call, 1:00 meeting, check in with 3 coworkers, reply to the Chairman: beat 2.2). Story NPCs get a `LunchStory` conversation; finishing each = one check-in. Coworkers switch to Act 2 small talk. Act 2 mail arrives on the terminal (`ArrivesAfter`). The clock runs to **2:14** and stops for good (beat 2.6): PA, a flicker, and Fear never drops below `FrozenFearFloor` again. Knobs on Take Grab → Story. | None |
| `UTakeKeyLockLibrary` | Blueprint nodes: `GenerateKeyLockForRun`, `EvaluateKeyLock` (Wrong / Solved / Corrupted), `CurrentResult`, `SightCheck`, `IsGlyphLit`, `IsGlyphHidden`, `GlyphRowString`. | None |

## Console commands (press ` during Play)

| Command | Does |
|---------|------|
| `TakeFear 0.8` | Set Fear instantly |
| `TakeCreep 1 20` | Creep to Fear 1 over 20 s (the real feel) |
| `TakeStalker` | Wake the stalker now |
| `TakeRewind` | "CUT. AGAIN." |
| `TakeLoop 1` | Force the Corridor B loop on (otherwise it only runs during the Corridor B beat) |
| `TakeBeat 1` | Jump to a story beat: 0 morning, 1 meeting (all tasks done), 2 Corridor B, 3 after the wisp, 4 lunch (plays beat 1.10 now: Act 2) |
| `TakeTime 1410` | Set the clock (HHMM). In Act 2, 1410 = four game minutes (12 s) before the clocks freeze at 2:14 |
| `TakeSoulBits 1` | Pretend you have 1 soul bit (telekinesis reach) |
| `TakeSeed` | Print this run's seed and all three Key Locks to the log |

## Log lines to look for (Output Log, filter "TAKE 4471")
- `Fear Director: MPC ok, volume ok, fog ok, 1 loop(s), stalker ok, 8 light(s)`
- `Added Take Grab to BP_FirstPersonCharacter_C_0`, then `Grab keys bound`
- `Fear stage 1 -> 2`, `Corridor loop ON`, `Looped (1)`, `Stalker -> Chase`, `CUT. AGAIN. -> Take 2`

If E / Right Mouse don't respond (the template uses Enhanced Input), wire your own input actions to `Take Grab → TryGrabOrDrop` and `Throw` in BP_FirstPersonCharacter, and untick **Auto Bind Keys**.
