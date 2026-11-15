# Folder guide: what to save where

Short version: **save only into 🎨 folders, give anything you want in the game a prefix (`SM_` / `SK_` / `A_` / `T_`), and never touch `export/` folders by hand.**

## How the export finds your assets

The export script looks at the **top-level collections** in the Outliner, the boxes directly under "Scene Collection". Any collection whose name starts with `SM_`, `SK_` or `A_` becomes **one FBX** with that name, made of **everything inside it**: all its meshes, its Armature, and any sub-collections. Everything else in the file is ignored.

So you don't need one mesh per scene. A file can hold many assets, and an asset can be many meshes:

```
CH_Nurse.blend
Scene Collection
├── SK_Nurse                ← ONE export: SK_Nurse.fbx
│   ├── Nurse_Armature      (the skeleton)
│   ├── Nurse_Body          (parented to the armature)
│   ├── Nurse_Scrubs        (parented to the armature)
│   └── Nurse_Hair          (parented to the armature)
├── A_Nurse_Wave            ← ONE export: A_Nurse_Wave.fbx (the armature's active action)
│   └── Nurse_Armature_Anim
├── Reference_Photos        ← ignored (no prefix)
└── Lights                  ← ignored
```

```
Kit_Office.blend
Scene Collection
├── SM_Desk                 ← SM_Desk.fbx (desk top + legs + drawer: one mesh in Unreal)
├── SM_Chair                ← SM_Chair.fbx
├── SM_Mug                  ← SM_Mug.fbx
└── WIP_lamp                ← ignored until you rename it SM_Lamp
```

Everything inside an `SM_` collection is **combined into one mesh** in Unreal. If you want something separate (so it can be picked up, or moved on its own), give it its own `SM_` collection.

## Folder by folder

| Folder | You save | Name it | Notes |
|--------|----------|---------|-------|
| `00_Reference/` 🎨 | screenshots, concept art, notes | anything descriptive | never goes into the game |
| `10_Characters/daz/` 🎨 | DAZ scenes (File → Save As → Scene) | `Nurse.duf` | one character per file |
| `10_Characters/blend/` 🎨 | the game-ready character | `CH_Nurse.blend` | collection `SK_Nurse` = Armature + its meshes; animations as `A_Nurse_*` collections |
| `10_Characters/export/` ⚙️ | nothing | `SK_*.fbx` appear here | written by the export script |
| `20_Props/blend/` 🎨 | prop kits | `Kit_Office.blend` | one `SM_` collection per prop |
| `20_Props/export/` ⚙️ | nothing | `SM_*.fbx` appear here | |
| `30_Environments/blend/` 🎨 | modular walls/floors/doors | `Env_Hospital.blend` | `SM_Wall_4m`, `SM_Floor_2x2`... built on a grid |
| `30_Environments/export/` ⚙️ | nothing | `SM_*.fbx` appear here | |
| `40_Textures/` 🎨 | PNG textures | `T_<Asset>_BaseColor.png`, `_Normal`, `_ORM` | `<Asset>` = the mesh name without its prefix: `SM_Desk` → `T_Desk_...` |
| `50_Animation/source/` 🎨 | mocap, Mixamo, iClone, keyframed .blend | anything; in .blend use `A_<Char>_<Action>` collections | |
| `50_Animation/export/` ⚙️ | nothing | `A_*.fbx` appear here | |

## Before you save a model (what the export checks)

1. Build it at the **world origin**, standing on the floor: its lowest point at z = 0.
2. Make the front face **−Y**, toward you in the Front view (numpad 1).
3. Use **real size**: 1 Blender unit = 1 m. A desk is about 0.75 high.
4. Press **Ctrl+A → All Transforms**, so the scale reads 1, 1, 1.
5. Stay within the triangle budgets: props 20k, characters 60k. Over budget is a warning; over double is a stop.
6. For characters, parent the meshes to the Armature (Ctrl+P → With Automatic Weights, or keep the DAZ weights).

If something's wrong, the export prints `X` with the reason and skips that asset. The rest still export.

## After you save: three steps

1. **Blender:** Scripting tab → Open `Scripts/blender/export_all.py` → Run ▶. Read the console: `OK` means exported, `X` means a problem, with the reason.
2. *(optional)* `python Scripts/validate/check_art.py` for a folder and name check.
3. **Unreal:** Tools → Execute Python Script → `Scripts/unreal/update_level.py`. It imports or replaces everything, builds materials from your textures, adds collision, and swaps any `BLOCK_` grey boxes for the real meshes.
