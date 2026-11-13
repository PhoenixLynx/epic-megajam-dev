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

# Prop materials: one simple textured material + an instance per texture
# (MI_Poster_Energy, MI_Poster_Energy_Changed, MI_ChairmanPortrait, MI_CallSheet, MI_ScriptPage_01 ...)
MAT_DIR = "/Game/Props/Materials"
mel = unreal.MaterialEditingLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()


def get_or_create(name, folder, cls, factory):
    path = f"{folder}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.EditorAssetLibrary.load_asset(path)
    return tools.create_asset(name, folder, cls, factory)


base = get_or_create("M_PropTexture", MAT_DIR, unreal.Material, unreal.MaterialFactoryNew())
mel.delete_all_material_expressions(base)
tex_param = mel.create_material_expression(base, unreal.MaterialExpressionTextureSampleParameter2D, -400, 0)
tex_param.set_editor_property("parameter_name", "Tex")
mel.connect_material_property(tex_param, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
rough = mel.create_material_expression(base, unreal.MaterialExpressionConstant, -400, 250)
rough.set_editor_property("r", 0.65)
mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
mel.recompile_material(base)
unreal.EditorAssetLibrary.save_loaded_asset(base)

made = 0
for path in unreal.EditorAssetLibrary.list_assets("/Game/Props/Textures", recursive=False):
    tex = unreal.EditorAssetLibrary.load_asset(path)
    if not isinstance(tex, unreal.Texture2D):
        continue
    name = tex.get_name()                       # T_Poster_Energy_Changed
    mi_name = "MI_" + name[2:] if name.startswith("T_") else "MI_" + name
    mi = get_or_create(mi_name, MAT_DIR, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    mel.set_material_instance_parent(mi, base)
    mel.set_material_instance_texture_parameter_value(mi, "Tex", tex)
    unreal.EditorAssetLibrary.save_loaded_asset(mi)
    made += 1
unreal.log(f"[TAKE 4471] Made {made} prop material instances in {MAT_DIR}")

done = sum(1 for t in tasks if t.get_editor_property("imported_object_paths"))
unreal.log(f"[TAKE 4471] Imported {done}/{len(tasks)} files")
if done < len(tasks):
    for t in tasks:
        if not t.get_editor_property("imported_object_paths"):
            unreal.log_warning(f"[TAKE 4471] Not imported: {t.get_editor_property('filename')}")
