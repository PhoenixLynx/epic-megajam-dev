"""Blender -> Unreal export (run inside Blender: Scripting tab > Open > Run, or from the command line).

THE ONE RULE: one top-level collection = one asset, named with its Unreal prefix.
    SM_Desk        a static mesh   -> SourceArt/<area>/export/SM_Desk.fbx
    SK_Nurse       a skeletal mesh (an Armature + its meshes) -> SK_Nurse.fbx
    A_Nurse_Wave   an animation (the Armature's active action) -> A_Nurse_Wave.fbx
Anything else (WIP, references, lights, cameras) is ignored, so the .blend can stay messy.

Every exported asset is checked first (validate_asset): wrong names, unapplied scale, pivot not
at the base, too many triangles. Problems are printed and that asset is skipped, so a bad mesh
never reaches Unreal.

Command line (exports every .blend in a folder, no UI):
    blender -b -P Scripts/blender/export_all.py -- SourceArt/20_Props/blend
"""
import os
import sys

import bpy

PREFIXES = ("SM_", "SK_", "A_")
MAX_TRIS = {"SM_": 20000, "SK_": 60000}   # per asset; raise for hero pieces (a warning, not a stop, below 2x)


def export_dir_for(blend_path):
    """SourceArt/20_Props/blend/Kit_Office.blend -> SourceArt/20_Props/export/"""
    area = os.path.dirname(os.path.dirname(os.path.abspath(blend_path)))
    out = os.path.join(area, "export")
    os.makedirs(out, exist_ok=True)
    return out


def tri_count(objs):
    n = 0
    for o in objs:
        if o.type == "MESH":
            n += sum(len(p.vertices) - 2 for p in o.data.polygons)
    return n


def validate_asset(name, objs):
    """Returns (ok, messages). ok=False means: don't export."""
    msgs, ok = [], True
    meshes = [o for o in objs if o.type == "MESH"]
    if not meshes:
        return False, [f"{name}: no mesh in the collection"]
    for o in meshes:
        if any(abs(s - 1.0) > 1e-4 for s in o.scale):
            msgs.append(f"{name}/{o.name}: scale {tuple(round(s, 3) for s in o.scale)} not applied (Ctrl+A > Scale)")
            ok = False
    if name.startswith("SM_"):
        # pivot at the base centre: the lowest point of the asset should sit at z = 0 (the floor)
        zs = [(o.matrix_world @ v.co).z for o in meshes for v in o.data.vertices]
        if zs and abs(min(zs)) > 0.01:
            msgs.append(f"{name}: lowest point is at z={min(zs):.3f} m, expected 0 (the pivot goes where it touches the floor)")
        size = max(zs) - min(zs) if zs else 0
        if size > 20:
            msgs.append(f"{name}: {size:.1f} m tall: is the scale right? (1 Blender unit = 1 m)")
            ok = False
    tris = tri_count(objs)
    for p, cap in MAX_TRIS.items():
        if name.startswith(p) and tris > cap:
            msgs.append(f"{name}: {tris} triangles (budget {cap})")
            if tris > cap * 2:
                ok = False
    if name.startswith(("SK_", "A_")) and not any(o.type == "ARMATURE" for o in objs):
        msgs.append(f"{name}: needs an Armature in the collection")
        ok = False
    return ok, msgs


def select_only(objs):
    bpy.ops.object.select_all(action="DESELECT")
    for o in objs:
        o.hide_set(False)
        o.select_set(True)
    bpy.context.view_layer.objects.active = objs[0]


def export_collection(coll, out_dir):
    objs = list(coll.all_objects)
    ok, msgs = validate_asset(coll.name, objs)
    for m in msgs:
        print(("  ! " if ok else "  X ") + m)
    if not ok:
        print(f"  SKIPPED {coll.name}")
        return False
    select_only(objs)
    is_anim = coll.name.startswith("A_")
    path = os.path.join(out_dir, coll.name + ".fbx")
    bpy.ops.export_scene.fbx(
        filepath=path,
        use_selection=True,
        apply_scale_options="FBX_SCALE_ALL",   # Blender metres -> Unreal centimetres
        axis_forward="-Y", axis_up="Z",         # Unreal's usual import orientation
        apply_unit_scale=True,
        bake_space_transform=False,
        object_types={"MESH", "ARMATURE", "EMPTY"},
        mesh_smooth_type="FACE",
        use_mesh_modifiers=True,
        use_triangles=False,
        add_leaf_bones=False,                   # extra end bones confuse Unreal's retargeter
        primary_bone_axis="Y", secondary_bone_axis="X",
        bake_anim=is_anim,
        bake_anim_use_all_actions=False,
        bake_anim_use_nla_strips=False,
        path_mode="COPY", embed_textures=False,
    )
    print(f"  OK {coll.name} -> {path}")
    return True


def export_blend(blend_path=None):
    blend_path = blend_path or bpy.data.filepath
    if not blend_path:
        print("Save the .blend inside SourceArt/<area>/blend/ first.")
        return 0
    out_dir = export_dir_for(blend_path)
    print(f"[export_all] {os.path.basename(blend_path)} -> {out_dir}")
    done = 0
    for coll in bpy.context.scene.collection.children:
        if coll.name.startswith(PREFIXES):
            done += export_collection(coll, out_dir)
    print(f"[export_all] {done} asset(s) exported")
    return done


if __name__ == "__main__":
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    if argv and os.path.isdir(argv[0]):
        for f in sorted(os.listdir(argv[0])):
            if f.endswith(".blend"):
                bpy.ops.wm.open_mainfile(filepath=os.path.join(argv[0], f))
                export_blend()
    else:
        export_blend()
