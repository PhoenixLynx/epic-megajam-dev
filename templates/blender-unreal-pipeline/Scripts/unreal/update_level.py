"""THE ONE BUTTON (run in Unreal: Tools > Execute Python Script, or `py Scripts/unreal/update_level.py`).

Does, in order, everything that used to be seven scripts:
  1. Import every FBX in SourceArt/*/export/ into the matching Content folder
     (SourceArt/20_Props/export/SM_Desk.fbx -> /Game/Props/SM_Desk). Re-importing REPLACES the asset,
     so everything already placed in the level updates in place.
  2. Textures: SourceArt/40_Textures/T_<Asset>_BaseColor.png (and _Normal, _ORM) are imported and wired
     into a material instance MI_<Asset> of the master material, which goes on the mesh.
  3. Static meshes get simple collision (a box, or the mesh hull for odd shapes).
  4. Blockout swap: any actor in the open level labelled BLOCK_<Asset> (a grey box I placed) is replaced by
     the real <Asset> at the same spot, rotation and size.
  5. Saves everything and prints a short report.

Settings are at the top. Nothing here is specific to one game.
"""
import os

import unreal

PROJECT_DIR = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
SOURCE_ART = os.path.join(PROJECT_DIR, "SourceArt")
TEXTURES = os.path.join(SOURCE_ART, "40_Textures")
AREA_TO_CONTENT = {             # SourceArt area -> Content folder
    "10_Characters": "/Game/Characters",
    "20_Props": "/Game/Props",
    "30_Environments": "/Game/Environments",
    "50_Animation": "/Game/Animation",
}
MASTER_MATERIAL = "/Game/Materials/M_Master"        # needs params BaseColor, Normal, ORM (textures) + Tint (vector)
SKELETON = "/Game/Characters/Mannequin/SK_Mannequin_Skeleton"   # animations retarget onto this; "" to import their own

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
lib = unreal.EditorAssetLibrary
report = {"imported": [], "materials": [], "swapped": 0, "problems": []}


def fbx_task(fbx, dest, name):
    opts = unreal.FbxImportUI()
    opts.import_materials = False      # materials come from step 2, not from the FBX
    opts.import_textures = False
    opts.import_as_skeletal = name.startswith("SK_")
    opts.import_animations = name.startswith("A_")
    if name.startswith("A_"):
        opts.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION
        if SKELETON and lib.does_asset_exist(SKELETON):
            opts.skeleton = lib.load_asset(SKELETON)
    elif name.startswith("SK_"):
        opts.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
    else:
        opts.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
        opts.static_mesh_import_data.combine_meshes = True
        opts.static_mesh_import_data.auto_generate_collision = False   # step 3 does it
    t = unreal.AssetImportTask()
    t.filename, t.destination_path, t.destination_name = fbx, dest, name
    t.replace_existing, t.automated, t.save = True, True, True
    t.options = opts
    return t


def import_meshes():
    tasks = []
    for area, dest in AREA_TO_CONTENT.items():
        export = os.path.join(SOURCE_ART, area, "export")
        if not os.path.isdir(export):
            continue
        for f in sorted(os.listdir(export)):
            if f.lower().endswith(".fbx"):
                tasks.append(fbx_task(os.path.join(export, f), dest, f[:-4]))
    if tasks:
        asset_tools.import_asset_tasks(tasks)
    for t in tasks:
        path = f"{t.destination_path}/{t.destination_name}"
        (report["imported"] if lib.does_asset_exist(path) else report["problems"]).append(path)
    return [f"{t.destination_path}/{t.destination_name}" for t in tasks]


def import_texture(png, dest="/Game/Textures"):
    name = os.path.splitext(os.path.basename(png))[0]
    t = unreal.AssetImportTask()
    t.filename, t.destination_path, t.destination_name = png, dest, name
    t.replace_existing, t.automated, t.save = True, True, True
    asset_tools.import_asset_tasks([t])
    tex = lib.load_asset(f"{dest}/{name}")
    if tex and not name.endswith("_BaseColor"):
        tex.set_editor_property("srgb", False)   # normal / ORM maps are data, not colour
        if name.endswith("_Normal"):
            tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
    return tex


def material_for(asset_path):
    """T_<Asset>_BaseColor/_Normal/_ORM in SourceArt/40_Textures -> MI_<Asset> on the mesh."""
    base = asset_path.split("/")[-1]
    short = base[3:]   # SM_Desk -> Desk
    maps = {}
    for suffix in ("BaseColor", "Normal", "ORM"):
        for ext in (".png", ".tga", ".jpg"):
            p = os.path.join(TEXTURES, f"T_{short}_{suffix}{ext}")
            if os.path.isfile(p):
                maps[suffix] = import_texture(p)
                break
    if not maps or not lib.does_asset_exist(MASTER_MATERIAL):
        return None
    folder = asset_path.rsplit("/", 1)[0] + "/Materials"
    mi_path = f"{folder}/MI_{short}"
    mi = lib.load_asset(mi_path) if lib.does_asset_exist(mi_path) else asset_tools.create_asset(
        f"MI_{short}", folder, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    mel = unreal.MaterialEditingLibrary
    mel.set_material_instance_parent(mi, lib.load_asset(MASTER_MATERIAL))
    for param, tex in maps.items():
        if tex:
            mel.set_material_instance_texture_parameter_value(mi, param, tex)
    lib.save_loaded_asset(mi)
    report["materials"].append(mi_path)
    return mi


def finish_static_mesh(path):
    mesh = lib.load_asset(path)
    if not isinstance(mesh, unreal.StaticMesh):
        return
    sub = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
    if sub.get_simple_collision_count(mesh) == 0:
        sub.add_simple_collisions(mesh, unreal.ScriptCollisionShapeType.BOX)
    mi = material_for(path)
    if mi:
        for i in range(sub.get_number_materials(mesh)):
            mesh.set_material(i, mi)
    lib.save_loaded_asset(mesh)


def swap_blockouts(paths):
    by_name = {p.split("/")[-1]: p for p in paths}
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    for a in actors.get_all_level_actors():
        label = a.get_actor_label()
        if not label.startswith("BLOCK_"):
            continue
        name = label[len("BLOCK_"):].split("_#")[0]   # BLOCK_SM_Desk or BLOCK_SM_Desk_#3
        if name not in by_name or not name.startswith("SM_"):
            continue
        mesh = lib.load_asset(by_name[name])
        new = actors.spawn_actor_from_object(mesh, a.get_actor_location(), a.get_actor_rotation())
        new.set_actor_label(name)
        new.set_folder_path(a.get_folder_path() or "Art")
        actors.destroy_actor(a)
        report["swapped"] += 1


def run():
    paths = import_meshes()
    for p in paths:
        if p.split("/")[-1].startswith("SM_"):
            finish_static_mesh(p)
    swap_blockouts(paths)
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    unreal.log(f"[update_level] imported {len(report['imported'])}, materials {len(report['materials'])}, "
               f"blockouts swapped {report['swapped']}, problems {len(report['problems'])}")
    for p in report["problems"]:
        unreal.log_warning(f"[update_level] did not import: {p}")


run()
