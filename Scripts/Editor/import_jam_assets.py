"""
import_jam_assets.py: TAKE 4471. One click to bring everything Claude generated into Unreal.

  SourceArt/Blender/Export/*.fbx          -> /Game/Props/Meshes     (16 office props)
  SourceArt/Textures/UI/Glyphs/*.png      -> /Game/UI/Glyphs        (UI texture settings)
  SourceArt/Textures/Props/*.png          -> /Game/Props/Textures   (posters, portrait, paper)
  SourceArt/Audio/Placeholder/*.wav       -> /Game/Audio/Placeholder

Run: Tools > Execute Python Script... (after preflight). Safe to re-run (replaces).
"""
import glob
import os

import unreal

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
JOBS = [
    ("SourceArt/Blender/Export/*.fbx", "/Game/Props/Meshes"),
    ("SourceArt/Textures/UI/Glyphs/*.png", "/Game/UI/Glyphs"),
    ("SourceArt/Textures/Props/*.png", "/Game/Props/Textures"),
    ("SourceArt/Audio/Placeholder/*.wav", "/Game/Audio/Placeholder"),
]


def fbx_options():
    ui = unreal.FbxImportUI()
    ui.set_editor_property("import_mesh", True)
    ui.set_editor_property("import_materials", True)
    ui.set_editor_property("import_textures", False)
    ui.set_editor_property("import_as_skeletal", False)
    smd = ui.get_editor_property("static_mesh_import_data")
    smd.set_editor_property("combine_meshes", True)
    smd.set_editor_property("auto_generate_collision", True)   # simple collision for physics props
    return ui


tasks = []
for pattern, dest in JOBS:
    for f in sorted(glob.glob(os.path.join(ROOT, pattern))):
        t = unreal.AssetImportTask()
        t.set_editor_property("filename", f)
        t.set_editor_property("destination_path", dest)
        t.set_editor_property("automated", True)
        t.set_editor_property("replace_existing", True)
        t.set_editor_property("save", True)
        if f.lower().endswith(".fbx"):
            t.set_editor_property("options", fbx_options())
        tasks.append(t)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

# UI textures: crisp, no mipmaps, UI compression
for path in unreal.EditorAssetLibrary.list_assets("/Game/UI/Glyphs", recursive=False):
    tex = unreal.EditorAssetLibrary.load_asset(path)
    if isinstance(tex, unreal.Texture2D):
        tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
        tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
        unreal.EditorAssetLibrary.save_loaded_asset(tex)

done = sum(1 for t in tasks if t.get_editor_property("imported_object_paths"))
unreal.log(f"[TAKE 4471] Imported {done}/{len(tasks)} files")
if done < len(tasks):
    for t in tasks:
        if not t.get_editor_property("imported_object_paths"):
            unreal.log_warning(f"[TAKE 4471] Not imported: {t.get_editor_property('filename')}")
