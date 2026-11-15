# Blender → Unreal pipeline (with DAZ and other characters)

**The idea: you make things, scripts move them.** You only work in the folders marked 🎨.
Everything marked ⚙️ is written by scripts. Never edit those by hand: they get overwritten.

## Where you drop in

```
SourceArt/
  00_Reference/          🎨 moodboards, concept art, screenshots, notes
  10_Characters/
    daz/                 🎨 DAZ scenes (.duf), one per character
    blend/               🎨 CH_<Name>.blend: clothes, hair, fixes, game-ready pass
    export/              ⚙️ SK_<Name>.fbx
  20_Props/
    blend/               🎨 Kit_<Theme>.blend: one file per prop kit (office, ward, kitchen...)
    export/              ⚙️ SM_<Name>.fbx
  30_Environments/
    blend/               🎨 modular walls/floors/doors on a grid (1 m, 2 m, 4 m pieces)
    export/              ⚙️ SM_<Name>.fbx
  40_Textures/           🎨 T_<Asset>_BaseColor / _Normal / _ORM .png
  50_Animation/
    source/              🎨 mocap (Rokoko), Mixamo, iClone, DAZ poses, hand-keyed .blend
    export/              ⚙️ A_<Character>_<Action>.fbx
Scripts/
  blender/export_all.py  ⚙️ Blender → FBX (checks each asset first)
  unreal/update_level.py ⚙️ THE ONE BUTTON in Unreal: import → materials → collision → swap blockouts
  validate/check_art.py  ⚙️ folder/name check, runs anywhere
Content/                 ⚙️ Unreal's copy, mirrors SourceArt (Characters/, Props/, Environments/, Animation/)
```

## The loop

| Step | Who | What happens |
|------|-----|--------------|
| 1. Concept | 🎨 you | Sketch or describe it. Claude turns it into a named asset list (`SM_Desk`, `SK_Nurse`...). |
| 2. Blockout | ⚙️ Claude | Grey boxes at the right size, placed in the level as `BLOCK_SM_Desk`, so you can play the space before any art exists. |
| 3. Model | 🎨 you | In Blender: **one top-level collection = one asset**, named with its prefix. Anything else in the file is ignored. |
| 4. Export | ⚙️ | Run `export_all.py` in Blender (Scripting tab). It checks each asset, then writes the FBX to `export/`. |
| 5. Check | ⚙️ | `python Scripts/validate/check_art.py`: wrong names, stray files, exports older than their .blend. |
| 6. Update level | ⚙️ one button | In Unreal: Tools → Execute Python Script → `update_level.py`. Re-imports replace in place; `BLOCK_` boxes become the real mesh. |
| 7. Review | 🎨 you | Play. Write notes with **N** (tag `!` = critical, `~` = nice to have). The notes become the next batch. |

## Blender rules (export_all.py checks these)

- **Names:** `SM_` static mesh, `SK_` skeletal mesh (an Armature plus its meshes), `A_` animation (the Armature's active action).
- **Scale:** 1 Blender unit = 1 m. Model at real size and **apply scale** (Ctrl+A → Scale). The export converts to Unreal centimetres.
- **Pivot:** model each asset at the world origin, standing on the floor (lowest point at z = 0). That spot becomes its pivot in Unreal, so it drops straight onto floors.
- **Front:** the front of a prop faces Blender's −Y (toward you in the Front view).
- **Budgets** (warnings, and a stop at 2×): props 20k triangles, characters 60k. Raise them for hero pieces in `MAX_TRIS`.
- **Materials:** keep them simple in Blender (1–3 slots). Real textures go in `40_Textures` as `T_<Asset>_BaseColor/_Normal/_ORM`; the Unreal script builds `MI_<Asset>` from them.

## Characters: DAZ and friends

- **License first.** DAZ figures and store items in a game you distribute need DAZ's **Interactive License** for each product. Check before you build a cast.
- **Route A (control):** DAZ → Blender with the **Diffeomorphic DAZ Importer** add-on → game-ready pass → `SK_<Name>` collection → export.
- **Route B (speed):** DAZ's **DAZ to Unreal** bridge, straight into Unreal. Faster, less control, and harder to keep in this folder structure.
- **Game-ready pass** (in `10_Characters/blend`):
  - merge to 2–4 materials
  - delete hidden geometry under clothes
  - decimate to budget
  - make 1–2 LODs (lower-detail copies Unreal swaps to at a distance)
  - check the skeleton has no extra end bones
- **Animation:** everything plays on **one skeleton**. In Unreal, the **IK Retargeter** maps one skeleton's bones onto another's, so DAZ, Mixamo, Rokoko mocap and iClone animations all drive the same characters. Set `SKELETON` at the top of `update_level.py`.
- **Other sources use the same slots:** Character Creator/iClone, MetaHumans, your own sculpts. Each just needs to end up as an `SK_` collection or FBX.

## Day 1 of a project (from the TAKE 4471 retro)

1. Copy this template, make the git repo, create the itch draft page and the video channel.
2. Run **one test asset** through the whole loop (a crate: model → export → update level → in game) before making twenty.
3. Package a Windows build the same day, then every evening. Some bugs only show up in the .exe.
4. Do a 10-second capture test (Win+Alt+R in the .exe) to check the recording fills the frame.
5. Put the in-game checklist and notes (N) in from the first playtest.
6. Check the engine's default debug keys before choosing hotkeys (F1–F5 belong to Unreal in Development builds).
7. Pick a feature-freeze time. After it, only `!` notes get fixed.
