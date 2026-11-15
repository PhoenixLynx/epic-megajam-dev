"""
import_stalker_face.py: TAKE 4471. Bring in Sol's stalker face (approved by the director, Nov 14).

  SourceArt/Textures/Props/T_Face_Stalker.png  -> /Game/Props/Textures/T_Face_Stalker
  MI_Face_Stalker (child of M_FaceTexture, like the coworker faces) -> /Game/Props/Materials

Only the stalker uses it (TakeStalker.cpp loads MI_Face_Stalker, falling back to MI_Face_Fear).
The coworkers' Calm / Fear / Neutral / Frown faces are unchanged.
Source: Sol, creative/art/exports/ART-001_face_stalker_1024_v01.png (= fear_intense uncanny v07).

Run: Tools > Execute Python Script... Safe to re-run (replaces).
"""
import os

import unreal

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
SRC = os.path.join(ROOT, "SourceArt/Textures/Props/T_Face_Stalker.png")
TEX_DIR = "/Game/Props/Textures"
MAT_DIR = "/Game/Props/Materials"
PARENT = MAT_DIR + "/M_FaceTexture"

if not os.path.isfile(SRC):
    raise RuntimeError(f"[TAKE 4471] Missing {SRC}")
if not unreal.EditorAssetLibrary.does_asset_exist(PARENT):
    raise RuntimeError("[TAKE 4471] M_FaceTexture not found: run import_jam_assets.py once first")

task = unreal.AssetImportTask()
task.set_editor_property("filename", SRC)
task.set_editor_property("destination_path", TEX_DIR)
task.set_editor_property("replace_existing", True)
task.set_editor_property("automated", True)
task.set_editor_property("save", True)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
tex = unreal.EditorAssetLibrary.load_asset(TEX_DIR + "/T_Face_Stalker")
if not isinstance(tex, unreal.Texture2D):
    raise RuntimeError("[TAKE 4471] T_Face_Stalker didn't import")

mi_path = MAT_DIR + "/MI_Face_Stalker"
if unreal.EditorAssetLibrary.does_asset_exist(mi_path):
    mi = unreal.EditorAssetLibrary.load_asset(mi_path)
else:
    mi = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "MI_Face_Stalker", MAT_DIR, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
mel = unreal.MaterialEditingLibrary
mel.set_material_instance_parent(mi, unreal.EditorAssetLibrary.load_asset(PARENT))
mel.set_material_instance_texture_parameter_value(mi, "Tex", tex)
unreal.EditorAssetLibrary.save_loaded_asset(mi)
unreal.log("[TAKE 4471] Stalker face: T_Face_Stalker + MI_Face_Stalker ready (Play to see it on the stalker)")
