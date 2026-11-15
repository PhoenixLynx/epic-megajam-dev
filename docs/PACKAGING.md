# Packaging the Windows build (TAKE 4471)

Read on Sun Nov 15, 04:10, from the PC's `Config/DefaultGame.ini` and `Config/DefaultEngine.ini`. The `Config/` folder is **not** in git, so these changes are made on the PC directly.

## What's already right

- **Start map:** `GameDefaultMap` = `/Game/Maps/L_Campus_Greybox`, the only map. The title screen is a widget drawn over that map (`UTakeTitleWidget`), not a separate map, so nothing else needs to be cooked as a "title map".
- **Game instance:** `TakeGameInstance`. **Build configuration:** Development.
- **Cooked by path already:** `/Game/Data`, `/Game/UI`, `/Game/Props`, `/Game/Audio` (music included), `/Game/Fear`, `/Game/Characters`, plus `L_Campus_Greybox` under `MapsToCook`.

## Always-cook folders (status at 10:35: the first three were added to the PC's config at 8:45 AM)

The game loads a lot of assets by name from C++ (`LoadObject`). The cooker can't see those references, so any folder not on the list below is **left out of the packaged build**. In the game that means grey grids, missing art or silence.

| Folder | Loaded by | If it's missing from the build |
|--------|-----------|--------------------------------|
| `/Game/Ending` | wrap banner, director's chair cards, overtime menu, cake top, dailies, skyline | the party banner falls back to 3D text, the cake top turns plain mint, the monitors show office screens, no skyline print |
| `/Game/Memorial` | the memorial wall (overnight work) | no memorial photos |
| `/Engine/BasicShapes` | Cube / Cylinder / Sphere / BasicShapeMaterial: the party, the penthouse, the city, plates, balloons | parts of the ending can vanish if the level itself doesn't happen to use those shapes |
| `/Game/FirstPerson`, `/Game/Input` | the player (`BP_FirstPersonGameMode`, character, Enhanced Input) | usually cooked through the map/config anyway; listed so it's certain |

### Optional extra lines

`/Game/Ending`, `/Game/Memorial` and `/Engine/BasicShapes` are already on the PC (8:45 AM). If the packaged build's player feels wrong (no movement or input), add these two lines under `[/Script/UnrealEd.ProjectPackagingSettings]` in `Config/DefaultGame.ini`, with Unreal closed:

```ini
+DirectoriesToAlwaysCook=(Path="/Game/FirstPerson")
+DirectoriesToAlwaysCook=(Path="/Game/Input")
```

(A path that doesn't exist yet, like `/Game/Memorial` if `import_memorial.py` was never run, only logs a warning.)

## Package

1. In Unreal, open **Platforms > Windows > Package Project**, and pick `C:\Users\casse\Documents\Agentic\EpicJam\Builds\Final`. This uses the Development configuration, set above.
2. Wait for **"Packaging (Windows): BUILD SUCCESSFUL"** in the Output Log (about 10–20 min; Friday's test build was 1.1 GB).
3. Open `Builds\Final\Windows\` and run `DirectorsCut_58.exe`.
4. **The .exe test** (outside the editor, the true test):
   - title screen → START → slate → the morning plays
   - coworkers' faces (not grey) and the dark skin (not a grey grid)
   - music on the first coffee
   - F8 to skip ahead, then F5 for the YES ending: banner, cake top, director's floor monitors, skyline print, credits
   - F6 for the NO ending
5. Zip the `Windows` folder (right-click > Compress), named for example `TAKE4471_Windows.zip`, for itch.io.

The playtest keys (F5–F8, Ctrl+number Fear, N notes) are still in the Development build. That's fine for the jam: they're not mentioned on the page, and F8 doubles as a skip for anyone stuck.

## Config change on the PC (14:40, Sun): engine debug view-mode keys off
`Config/DefaultInput.ini` (not in git) now ends with:
```
[/Script/Engine.PlayerInput]
!DebugExecBindings=ClearArray

[/Script/EnhancedInput.EnhancedPlayerInput]
!DebugExecBindings=ClearArray
```
The engine binds F1–F5 to view modes in Development builds (F5 = shader complexity, the "green screen"), which clashed with
the game's F5/F6 ending shortcuts. Backup: `Config/DefaultInput.ini.bak-2026-11-15`. Needs an editor restart to take effect.
