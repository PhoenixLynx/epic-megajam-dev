# C++ gameplay systems

Module `DirectorsCut_58` in `Source/` (switched on Fri Nov 13, 18:18).
About 1,400 lines. The Key Lock core is tested outside Unreal (`Scripts/Reference/test_keylock_core.cpp`, 20,000 seeds × 3 keys); everything else compiles for the first time on the PC.

| Class | What it does | Setup needed |
|-------|-------------|--------------|
| `UTakeGameInstance` | Run seed (clock-based, never shown; `DevSeedOverride` for bugs), take number, soul bits (0–3) = passes. **Console commands.** | Set as Game Instance class (Claude edits DefaultEngine.ini) |
| `ATakeFearDirector` | The single Fear value → `MPC_Fear`, post-process (saturation, vignette, grain, tint), fog, light dim/flicker, letterbox amount, stage events; turns the loop on at 0.5 and wakes the stalker at stage 4. **Previews in the editor** when you drag Fear. Gives the player the grab component at Play. | None: auto-wires everything |
| `UTakeGrabComponent` | **E** grab/drop, **Right Mouse** throw. Reach 3 m, 8 m with 1+ soul bit (telekinesis). Heavy things (> 40 kg) need 3 bits. | None: added to the player automatically |
| `ATakeLoopTrigger` | Corridor B loop: silent teleport back 10 m (keeps speed + view). Each loop swaps one poster to its `_Changed` material. `TryBreakLoop(poster)` breaks it. | None: finds `LoopPoster`-tagged posters and `MI_*_Changed` materials |
| `ATakeStalker` | Dormant + hidden → **Stalk** (lurks 6 m behind you) → **Chase** when you look straight at it or after 6 s → **Stunned** 1.5 s when a `Throwable` hits it at > 4 m/s → catch = rewind. Chase speed 350 + 80 per soul bit. `Slow()` for the extinguisher. Body = `SM_Stalker`. | Nav mesh (builder adds it; press **P**) |
| `UTakeRewindSubsystem` | "CUT. AGAIN.": fade to black, take +1, Fear 0, stalker reset, back to `PlayerStart` tagged `Desk4471`, one `ChangeOnRewind` object turned, fade in. `OnRewind(Take)` event for the slate widget. | None |
| `UTakeKeyLockLibrary` | Blueprint nodes: `GenerateKeyLockForRun`, `EvaluateKeyLock` (Wrong / Solved / Corrupted), `CurrentResult`, `SightCheck`, `IsGlyphLit`, `IsGlyphHidden`, `GlyphRowString`. | None |

## Console commands (press ` during Play)

| Command | Does |
|---------|------|
| `TakeFear 0.8` | Set Fear instantly |
| `TakeCreep 1 20` | Creep to Fear 1 over 20 s (the real feel) |
| `TakeStalker` | Wake the stalker now |
| `TakeRewind` | "CUT. AGAIN." |
| `TakeLoop 1` | Force the Corridor B loop on |
| `TakeSoulBits 1` | Pretend you have 1 soul bit (telekinesis reach) |
| `TakeSeed` | Print this run's seed and all three Key Locks to the log |

## Log lines to look for (Output Log, filter "TAKE 4471")
- `Fear Director: MPC ok, volume ok, fog ok, 1 loop(s), stalker ok, 8 light(s)`
- `Added Take Grab to BP_FirstPersonCharacter_C_0`, then `Grab keys bound`
- `Fear stage 1 -> 2`, `Corridor loop ON`, `Looped (1)`, `Stalker -> Chase`, `CUT. AGAIN. -> Take 2`

If E / Right Mouse don't respond (the template uses Enhanced Input), wire your own input actions to `Take Grab → TryGrabOrDrop` and `Throw` in BP_FirstPersonCharacter, and untick **Auto Bind Keys**.
