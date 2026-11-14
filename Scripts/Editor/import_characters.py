"""
import_characters.py: TAKE 4471. Bring the rigged mannequins + their animations into Unreal.

  SourceArt/Blender/Characters/SK_Mannequin.fbx           -> skeleton, skeletal mesh, 6 animations
  SourceArt/Blender/Characters/SK_MannequinGuard.fbx      -> skeletal mesh on the SAME skeleton
  SourceArt/Blender/Characters/SK_MannequinCustodian.fbx  -> skeletal mesh on the SAME skeleton
  all into /Game/Characters/Mannequin

Animations (from SK_Mannequin.fbx): Idle, Walk, SitDown, SitType, Reach, Stare.
Run on its own, or let import_jam_assets.py run it. Safe to re-run.
"""
import os

import unreal

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
SRC = os.path.join(ROOT, "SourceArt", "Blender", "Characters")
DEST = "/Game/Characters/Mannequin"
lib = unreal.EditorAssetLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()


def options(skeleton=None, animations=True):
    ui = unreal.FbxImportUI()
    ui.set_editor_property("import_mesh", True)
    ui.set_editor_property("import_as_skeletal", True)
    ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    ui.set_editor_property("import_materials", True)
    ui.set_editor_property("import_textures", False)
    ui.set_editor_property("import_animations", animations)
    ui.set_editor_property("create_physics_asset", True)
    if skeleton:
        ui.set_editor_property("skeleton", skeleton)
    return ui


def run(fbx, skeleton=None, animations=True):
    t = unreal.AssetImportTask()
    t.set_editor_property("filename", os.path.join(SRC, fbx))
    t.set_editor_property("destination_path", DEST)
    t.set_editor_property("automated", True)
    t.set_editor_property("replace_existing", True)
    t.set_editor_property("save", True)
    t.set_editor_property("options", options(skeleton, animations))
    tools.import_asset_tasks([t])
    return list(t.get_editor_property("imported_object_paths") or [])


if not os.path.isdir(SRC):
    unreal.log_warning(f"[TAKE 4471] No characters folder at {SRC}")
else:
    first = run("SK_Mannequin.fbx")
    skeleton = None
    for path in lib.list_assets(DEST, recursive=False):
        a = lib.load_asset(path)
        if isinstance(a, unreal.Skeleton):
            skeleton = a
            break
    others = []
    for fbx in ("SK_MannequinGuard.fbx", "SK_MannequinCustodian.fbx"):
        others += run(fbx, skeleton, animations=False)
    # Give the animations stable names the C++ can find: AS_Idle, AS_Walk, AS_SitDown, AS_SitType, AS_Reach, AS_Stare
    for path in lib.list_assets(DEST, recursive=False):
        a = lib.load_asset(path)
        if not isinstance(a, unreal.AnimSequence) or a.get_name().startswith("AS_"):
            continue
        for key in ("SitDown", "SitType", "Idle", "Walk", "Reach", "Stare"):
            if key.lower() in a.get_name().lower():
                target = f"{DEST}/AS_{key}"
                if lib.does_asset_exist(target):
                    lib.delete_asset(target)
                lib.rename_asset(path.split(".")[0], target)
                break
    anims = [p for p in lib.list_assets(DEST, recursive=False)
             if isinstance(lib.load_asset(p), unreal.AnimSequence)]
    unreal.log(f"[TAKE 4471] Characters: skeleton {'ok' if skeleton else 'MISSING'}, "
               f"3 mesh file(s) imported, {len(anims)} animation(s) in {DEST}")
    for p in anims:
        unreal.log(f"[TAKE 4471]   anim {p}")
