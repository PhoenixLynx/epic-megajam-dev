"""
import_ending_art.py: TAKE 4471. Sol's ending signage into the game.

  SourceArt/Textures/Ending/*.png  ->  /Game/Ending/T_*  +  MI_* (child of /Game/Props/Materials/M_ScreenTexture, texture "Tex")

  T_WrapBanner          the wrap party banner over the elevator (ART-010)
  T_DirectorCard_Blank  the director's chair name card, before you sit (ART-013)
  T_DirectorCard_4471   ...and after (ART-013)
  T_OvertimeMenu        the kitchen board in Act 3 (ART-016)
  T_CakeTop             the top of the cake you can cut (ART-011; the missing wedge filled in, the game cuts its own)
                        -> MI_CakeTop is a child of M_FaceTexture (masked, not glowing)
  T_Dailies_*           the director's floor monitors (ART-012a-d)
  T_PenthouseSkyline    the dusk skyline past the open side of the director's floor (ART-014)

The game looks for the MI_ versions at play (TakeDirector.cpp, TakeStory.cpp) and falls back to its 3D text if they're missing.
Run: Tools > Execute Python Script... Safe to re-run (replaces).
"""
import glob
import os

import unreal

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
DEST = "/Game/Ending"
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
mel = unreal.MaterialEditingLibrary
lib = unreal.EditorAssetLibrary

files = sorted(glob.glob(os.path.join(ROOT, "SourceArt/Textures/Ending/*.png")))
if not files:
    raise RuntimeError("[TAKE 4471] No PNGs in SourceArt/Textures/Ending")
tasks = []
for f in files:
    t = unreal.AssetImportTask()
    t.set_editor_property("filename", f)
    t.set_editor_property("destination_path", DEST)
    t.set_editor_property("replace_existing", True)
    t.set_editor_property("automated", True)
    t.set_editor_property("save", True)
    tasks.append(t)
asset_tools.import_asset_tasks(tasks)

parent = lib.load_asset("/Game/Props/Materials/M_ScreenTexture")
if not parent:
    raise RuntimeError("[TAKE 4471] /Game/Props/Materials/M_ScreenTexture is missing")
painted = lib.load_asset("/Game/Props/Materials/M_FaceTexture")   # masked, lit, no glow (run import_jam_assets.py first)
PAINTED = ("T_CakeTop",)   # things that are painted, not screens
for f in files:
    name = os.path.splitext(os.path.basename(f))[0]          # T_WrapBanner
    tex = lib.load_asset(f"{DEST}/{name}")
    if not isinstance(tex, unreal.Texture2D):
        unreal.log_warning(f"[TAKE 4471] {name} didn't import as a texture")
        continue
    tex.set_editor_property("srgb", True)
    if name in PAINTED or name.startswith("T_PenthouseSkyline"):
        tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_WORLD)
    else:
        tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)   # crisp text, no blurry mips at a distance
    lib.save_loaded_asset(tex)
    mi_name = "MI_" + name[2:]                                 # MI_WrapBanner
    mi_path = f"{DEST}/{mi_name}"
    mi = lib.load_asset(mi_path) if lib.does_asset_exist(mi_path) else asset_tools.create_asset(
        mi_name, DEST, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    mel.set_material_instance_parent(mi, painted if (name in PAINTED and painted) else parent)
    mel.set_material_instance_texture_parameter_value(mi, "Tex", tex)
    lib.save_loaded_asset(mi)
    unreal.log(f"[TAKE 4471] Ending art: {mi_name} ready")
