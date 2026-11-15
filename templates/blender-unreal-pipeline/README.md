# Blender → Unreal game template

A starting folder for the next game: Blender (and DAZ) for the art, Unreal for the game, scripts in between.

- **How it works, and where you drop in:** [docs/PIPELINE.md](docs/PIPELINE.md)
- **Blender → FBX:** `Scripts/blender/export_all.py` (run in Blender)
- **The one button in Unreal:** `Scripts/unreal/update_level.py`
- **Quick check:** `python Scripts/validate/check_art.py`

To start a project: copy this folder next to the new `.uproject`, run `git init` and `git lfs install`, then follow "Day 1" in PIPELINE.md.

Tested: `export_all.py` exported a good asset and refused one with unapplied scale and a floating pivot (Blender 5.2). `check_art.py` caught a badly named texture. `update_level.py` hasn't been run in Unreal yet; the crate test on day 1 is its first run.
