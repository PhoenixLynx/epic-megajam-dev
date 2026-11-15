"""
import_memorial.py: TAKE 4471. The memorial wall in the lobby ("HALCYON: OUR PEOPLE").

  SourceArt/Textures/Memorial/*.png  ->  /Game/Memorial/T_Memorial_01, _02, ...  (sorted by file name)
                                     +  /Game/Memorial/MI_Memorial_01, _02, ... (child of /Game/Props/Materials/M_ScreenTexture, texture "Tex")

Numbered, not named after the files, because the game finds them by number at play (LoadObject on MI_Memorial_01..09,
missing ones are skipped; TakeAct2.cpp, HangMemorialWall). Up to 9 are hung; extras are imported but not shown.
Renaming or removing a PNG and re-running renumbers them; stale higher numbers from an earlier run are deleted.
Run: Tools > Execute Python Script... Safe to re-run (replaces).
"""
import glob
import os

import unreal

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
DEST = "/Game/Memorial"
MAX_SHOWN = 9
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary
lib = unreal.EditorAssetLibrary

files = sorted(glob.glob(os.path.join(ROOT, "SourceArt/Textures/Memorial/*.png")))
if not files:
    raise RuntimeError("[TAKE 4471] No PNGs in SourceArt/Textures/Memorial (copy the screenshots there first)")
parent = lib.load_asset("/Game/Props/Materials/M_ScreenTexture")
if not parent:
    raise RuntimeError("[TAKE 4471] /Game/Props/Materials/M_ScreenTexture is missing")

for i, f in enumerate(files, start=1):
    name = f"T_Memorial_{i:02d}"
    t = unreal.AssetImportTask()
    t.set_editor_property("filename", f)
    t.set_editor_property("destination_path", DEST)
    t.set_editor_property("destination_name", name)
    t.set_editor_property("replace_existing", True)
    t.set_editor_property("automated", True)
    t.set_editor_property("save", True)
    asset_tools.import_asset_tasks([t])
    tex = lib.load_asset(f"{DEST}/{name}")
    if not isinstance(tex, unreal.Texture2D):
        unreal.log_warning(f"[TAKE 4471] {os.path.basename(f)} didn't import as a texture")
        continue
    tex.set_editor_property("srgb", True)
    tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)   # sharp on the wall, no blurry mips
    lib.save_loaded_asset(tex)
    mi_name = f"MI_Memorial_{i:02d}"
    mi_path = f"{DEST}/{mi_name}"
    mi = lib.load_asset(mi_path) if lib.does_asset_exist(mi_path) else asset_tools.create_asset(
        mi_name, DEST, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    mel.set_material_instance_parent(mi, parent)
    mel.set_material_instance_texture_parameter_value(mi, "Tex", tex)
    lib.save_loaded_asset(mi)
    shown = "on the wall" if i <= MAX_SHOWN else "imported, not shown (more than 9)"
    unreal.log(f"[TAKE 4471] Memorial: {mi_name} <- {os.path.basename(f)} ({shown})")

# Fewer files than last time: remove the leftovers so the wall doesn't show an old photo
for i in range(len(files) + 1, 100):
    stale = [p for p in (f"{DEST}/MI_Memorial_{i:02d}", f"{DEST}/T_Memorial_{i:02d}") if lib.does_asset_exist(p)]
    if not stale:
        break
    for p in stale:
        lib.delete_asset(p)
        unreal.log(f"[TAKE 4471] Memorial: removed stale {p}")
