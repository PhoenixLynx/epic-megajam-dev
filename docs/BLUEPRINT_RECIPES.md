# Blueprint recipes: TAKE 4471 prototype

Step-by-step node guides for tonight. Names in `code` are the exact Unreal node or asset names.
Build order: **Fear → Grab/Throw → Loop → Stalker → Rewind.** Each one works on its own.

Assets created by `Scripts/Editor/build_fear_assets.py` (run it first): `MPC_Fear`, `M_GreyboxFear`.

---

## 1. Fear Director (`BP_FearDirector`)

One actor in the level that owns the single Fear value (0 → 1) and pushes it everywhere.

**Variables**

| Name | Type | Default | Notes |
|------|------|---------|-------|
| `Fear` | Float | 0 | Instance Editable, slider 0–1 (Details → Slider Range) |
| `TargetFear` | Float | 0 | where Fear is heading |
| `CreepSpeed` | Float | 0.05 | units per second; 0.05 = full creep in about 20 s |
| `FearPPV` | PostProcessVolume ref | | Instance Editable; pick `GB_PPV_Fear` |
| `Fog` | ExponentialHeightFog ref | | optional |

**Event Tick**
1. `FInterp To Constant` (Current = `Fear`, Target = `TargetFear`, Delta Time, Interp Speed = `CreepSpeed`) → `Set Fear`.
2. Call function **`ApplyFear`**.

**Function `ApplyFear`**
1. `Set Scalar Parameter Value` (Collection = `MPC_Fear`, Parameter = `Fear`, Value = `Fear`). Every `M_GreyboxFear` surface rots with it.
2. From `FearPPV` → `Get Settings` → `Set Members in PostProcessSettings`, ticking the override boxes:
   - `Color Saturation` = Lerp (1,1,1,1) → (0.3,0.3,0.3,1) by Fear
   - `Vignette Intensity` = Lerp 0.4 → 1.2
   - `Film Grain Intensity` = Lerp 0 → 0.6
   - `Scene Color Tint` = Lerp white → (0.85, 0.95, 1.0)
   → `Set Settings` back onto `FearPPV`.
3. `Fog` → `Set Fog Density` = Lerp 0.02 → 0.25 by Fear.
4. Letterbox: if Fear > 0.75, show `WBP_Letterbox` (two black bars) with bar height = `Map Range Clamped` (Fear 0.75–1 → 0–110 px).

**Construction Script:** call `ApplyFear` too. Now dragging the `Fear` slider in the editor previews the rot **without pressing Play**. 📸 Capture Fear 0 / 0.5 / 1.

**Public functions:** `StartCreep(Target, Seconds)` sets `TargetFear` and `CreepSpeed = Target / Seconds`. `Calm()` sets `TargetFear = 0`.

**Debug keys (Play mode):** keys `1–4` → `StartCreep(0.25 / 0.5 / 0.75 / 1.0, 3s)`, `0` → `Calm`.

Stage thresholds (from the concept): **0.25** off by a little · **0.5** draining · **0.75** watched (letterbox starts) · **0.9** stalked (stalker activates).

---

## 2. Grab, throw, telekinesis (in `BP_FirstPersonCharacter`)

**Add component:** `PhysicsHandle`.

**Variables:** `HeldComp` (Primitive Component ref), `GrabRange` = 300, `HoldDistance` = 180, `ThrowStrength` = 1800, `SoulBits` = 0 (int).

**Input `IA_Grab` (E / left mouse) → Started**
1. If `HeldComp` is valid → **Release** (see Throw step 1 without the impulse) and stop.
2. Range = `SoulBits ≥ 1 ? 800 : GrabRange` (telekinesis is just a longer reach).
3. `Line Trace By Channel` (Start = camera world location, End = Start + camera forward × Range, Visibility).
4. Branch: hit component `Is Simulating Physics`?
5. `Grab Component At Location With Rotation` (Component = hit component, Location = Impact Point, Rotation = component's world rotation) → `Set HeldComp`.

**Event Tick:** if `HeldComp` valid → `Set Target Location And Rotation` (camera location + forward × `HoldDistance`, camera rotation).

**Input `IA_Throw` (right mouse)**
1. `Release Component` on the PhysicsHandle.
2. `Add Impulse` on `HeldComp` (Impulse = camera forward × `ThrowStrength`, **Vel Change = true** so light and heavy things fly the same).
3. `Set HeldComp` = none.

**Making a prop throwable:** select it → Mobility **Movable** → **Simulate Physics** on → **Simulation Generates Hit Events** on. For the grey-box, the builder tags throwables `Throwable`.

**Hot coffee (`BP_CoffeeMug`):** on hit with the stalker → apply a "Burn" effect: 3 ticks of small damage over 1.5 s (a looping timer).

---

## 3. Corridor B loop (`BP_LoopTrigger`)

The trick: walking past a trigger silently teleports you **back by a fixed distance** into an identical stretch of corridor. Speed and view direction carry over, so you can't feel the jump.

1. `BP_LoopTrigger`: a `Box Collision`, scaled to fill the corridor's width and height. The builder already places a TriggerBox, `GB_LoopTrigger`, at the right spot. Either swap it for your BP or reference it from the Fear Director.
2. Variable `LoopOffset` (Vector) = (0, −1000, 0): one corridor section back.
3. `On Component Begin Overlap` → cast to the player →
   `Set Actor Location` (player location + `LoopOffset`, **Teleport = true**). Keep rotation and velocity untouched.
4. `LoopCount` += 1 → call `ChangeDetail(LoopCount)`: swap one poster's material (`GB_Poster_0…5`, made by the builder) so each loop has exactly one difference.
5. **Breaking the loop:** the changed poster gets an interact prompt. Using it → disable the trigger, `BP_FearDirector.Calm()`, and open the way on.

The corridor between the two points must look identical at both ends. The builder places the posters every 1000 cm so they repeat, and closes the Wellness door with `GB_Door_Wellness` (Act 1).

**Important: only switch the loop on once Fear ≥ 0.5.** By then the fog and the dimmed corridor light limit what you can see to about 6 m. Without that, the jump is visible: right before the teleport the end wall is 7 m ahead, right after it's 17 m ahead. With fog hiding the end wall, both views look the same. (Enable `BP_LoopTrigger`'s collision from the Fear Director when Fear crosses 0.5, and dim `GB_Light_CorridorB` at the same time.)

---

## 4. The stalker (`BP_Stalker` + `BP_StalkerAI`)

**Setup:** `BP_Stalker` = Character (tall grey capsule for now). AI Controller Class = `BP_StalkerAI`. The level needs a `NavMeshBoundsVolume`; the builder adds one (press **P** in the viewport to see the green walkable area).

**States (enum `E_StalkerState`):** Dormant · Stalk · Chase · Stunned.

**`BP_StalkerAI`**
- `Activate()` (called by the Fear Director at 0.9): set Stalk, show the stalker at `GB_Stalker_Spawn_Act1`.
- **Stalk:** Max Walk Speed 180. Every 1.0 s: `AI Move To` a point ~600 cm behind the player. It should be seen at the edge of the screen, not reach you. After 6 s, or if the player looks straight at it (dot product of player forward and direction to stalker > 0.9) → Chase.
- **Chase:** Max Walk Speed = `350 + 80 × SoulBits` (scales with progress). Every 0.25 s: `AI Move To` the player.
- **Stunned:** Max Walk Speed 0 for 1.5 s, then back to Chase.

**Getting hit:** `Event Hit` on the stalker's capsule → if Other has tag `Throwable` and its velocity length > 400 → Stunned. Fire extinguisher (later): a cone trace from the player → stalker speed × 0.4 for 3 s.

**Capture:** the capsule overlaps the player → call `Rewind` on the game mode (next recipe).

---

## 5. Rewind on capture ("CUT. AGAIN.")

In the **Game Instance** (survives level reloads): `TakeNumber` (int, starts 1), `RunSeed` (int, from `Now` → ticks on first start), `SoulBits`.

**`Rewind()`** (on the game mode):
1. Disable player input. `Start Camera Fade` (0 → 1, 0.15 s, black).
2. Play `sfx_clapper_snap`. Show `WBP_Slate`: **"CUT. AGAIN."** then **TAKE {TakeNumber + 1}**.
3. `TakeNumber` += 1. `BP_FearDirector` → Fear = 0 instantly.
4. Teleport the player to `GB_PlayerStart_Desk4471`; health stays reduced (rewind is not a free heal).
5. **One thing changed:** pick a random actor tagged `ChangeOnRewind` and swap its state (mug already on the desk, a chair turned, a poster changed).
6. Clock back to 9:00, PA line `PA_A1_Rewind`. Fade back in, restore input.
