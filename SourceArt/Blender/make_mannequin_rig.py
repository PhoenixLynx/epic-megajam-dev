"""
make_mannequin_rig.py: TAKE 4471. A rigged, animated mannequin made from scratch for the jam.

Builds the same faceless HALCYON mannequin as the static props, but as a SKELETAL mesh:
an articulated figure (each body part follows one bone, like a real shop mannequin),
a 16-bone skeleton, and a small shared animation set the coworkers, guards, custodian,
receptionist and (later) the player can all use:

  Idle      2 s loop  breathing, small weight shifts, head glances
  Walk      1 s loop  in place (the character moves, the animation doesn't)
  SitDown   1 s       standing -> seated on a 46 cm chair (play reversed to stand up)
  SitType   2 s loop  seated, forearms forward, typing / checking email
  Reach     1 s       right arm reaches forward (fridge, coffee machine, grabbing)
  Stare     2 s loop  perfectly still, head locked on target (Augmented Fear)

Variants share ONE skeleton (so every animation works on every variant):
  SK_Mannequin (mint suit + badge), SK_MannequinGuard (blue + cap), SK_MannequinCustodian (yellow)

Run:  python3 make_mannequin_rig.py
Out:  SourceArt/Blender/Characters/SK_*.fbx  (+ Characters/Mannequin_Rig.blend)
Facing matches the static props: front = -Y in Blender (+Y in Unreal before the -90 body turn).
"""
import math
import os
import sys

import bpy  # noqa: I001
from mathutils import Matrix, Vector

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from make_office_props import material  # noqa: E402  (same palette as the props)

OUT = os.path.join(HERE, "Characters")
FPS = 30

# --------------------------------------------------------------------------
# Skeleton (metres, figure 1.78 m, standing at the origin, facing -Y)
# name: (head, tail, parent)
# --------------------------------------------------------------------------
BONES = {
    "root":       ((0, 0, 0.0),        (0, 0.15, 0.0),     None),
    "pelvis":     ((0, 0, 0.92),       (0, 0, 1.02),       "root"),
    "spine":      ((0, 0, 1.02),       (0, 0, 1.22),       "pelvis"),
    "chest":      ((0, 0, 1.22),       (0, 0, 1.45),       "spine"),
    "neck":       ((0, 0, 1.45),       (0, 0, 1.55),       "chest"),
    "head":       ((0, 0, 1.55),       (0, 0, 1.78),       "neck"),
}
for side, sx in (("l", 1), ("r", -1)):
    BONES.update({
        f"upperarm_{side}": ((sx * 0.25, 0, 1.42), (sx * 0.27, 0, 1.14), "chest"),
        f"lowerarm_{side}": ((sx * 0.27, 0, 1.14), (sx * 0.27, 0, 0.86), f"upperarm_{side}"),
        f"hand_{side}":     ((sx * 0.27, 0, 0.86), (sx * 0.27, 0, 0.76), f"lowerarm_{side}"),
        f"thigh_{side}":    ((sx * 0.10, 0, 0.90), (sx * 0.10, 0, 0.48), "pelvis"),
        f"calf_{side}":     ((sx * 0.10, 0, 0.48), (sx * 0.10, 0, 0.07), f"thigh_{side}"),
        f"foot_{side}":     ((sx * 0.10, 0, 0.07), (sx * 0.10, -0.14, 0.03), f"calf_{side}"),
    })


def build_armature():
    arm_data = bpy.data.armatures.new("SK_Mannequin_Skeleton")
    arm = bpy.data.objects.new("Armature", arm_data)
    bpy.context.scene.collection.objects.link(arm)
    bpy.context.view_layer.objects.active = arm
    bpy.ops.object.mode_set(mode="EDIT")
    for name, (h, t, parent) in BONES.items():
        b = arm_data.edit_bones.new(name)
        b.head, b.tail = Vector(h), Vector(t)
        b.roll = 0.0
        if parent:
            b.parent = arm_data.edit_bones[parent]
            b.use_connect = False
    bpy.ops.object.mode_set(mode="OBJECT")
    return arm


# --------------------------------------------------------------------------
# Body parts: each is a primitive rigidly bound to one bone
# --------------------------------------------------------------------------
def _part(name, bone, kind, size, loc, mat, rot=(0, 0, 0)):
    if kind == "box":
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=loc, rotation=rot)
        bpy.context.object.scale = size
    elif kind == "cyl":
        r, depth = size
        bpy.ops.mesh.primitive_cylinder_add(vertices=12, radius=r, depth=depth, location=loc, rotation=rot)
    else:  # sphere
        bpy.ops.mesh.primitive_uv_sphere_add(segments=16, ring_count=8, radius=size, location=loc)
    o = bpy.context.object
    o.name = name
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    o.data.materials.append(material(mat))
    vg = o.vertex_groups.new(name=bone)
    vg.add(list(range(len(o.data.vertices))), 1.0, "REPLACE")
    for p in o.data.polygons:
        p.use_smooth = True
    return o


def _limb(name, bone, a, b, radius, mat):
    """Cylinder from point a to point b."""
    a, b = Vector(a), Vector(b)
    d = b - a
    rot = Vector((0, 0, 1)).rotation_difference(d.normalized()).to_euler()
    return _part(name, bone, "cyl", (radius, d.length), tuple((a + b) / 2), mat, rot=tuple(rot))


def build_body(name, suit, extras=()):
    parts = [
        _part("head", "head", "sphere", 0.11, (0, 0, 1.66), "M_Skin"),
        _limb("neck", "neck", (0, 0, 1.47), (0, 0, 1.57), 0.05, "M_Skin"),
        _part("chest", "chest", "box", (0.42, 0.22, 0.26), (0, 0, 1.33), suit),
        _part("belly", "spine", "box", (0.39, 0.21, 0.22), (0, 0, 1.11), suit),
        _part("hips", "pelvis", "box", (0.38, 0.20, 0.17), (0, 0, 0.93), suit),
    ]
    for side, sx in (("l", 1), ("r", -1)):
        parts += [
            _part(f"shoulder_{side}", f"upperarm_{side}", "sphere", 0.06, (sx * 0.25, 0, 1.41), suit),
            _limb(f"upperarm_{side}", f"upperarm_{side}", (sx * 0.25, 0, 1.41), (sx * 0.27, 0, 1.15), 0.05, suit),
            _part(f"elbow_{side}", f"lowerarm_{side}", "sphere", 0.047, (sx * 0.27, 0, 1.14), suit),
            _limb(f"lowerarm_{side}", f"lowerarm_{side}", (sx * 0.27, 0, 1.13), (sx * 0.27, 0, 0.88), 0.044, suit),
            _part(f"hand_{side}", f"hand_{side}", "sphere", 0.045, (sx * 0.27, 0, 0.81), "M_Skin"),
            _limb(f"thigh_{side}", f"thigh_{side}", (sx * 0.10, 0, 0.90), (sx * 0.10, 0, 0.49), 0.068, suit),
            _part(f"knee_{side}", f"calf_{side}", "sphere", 0.062, (sx * 0.10, 0, 0.48), suit),
            _limb(f"calf_{side}", f"calf_{side}", (sx * 0.10, 0, 0.47), (sx * 0.10, 0, 0.08), 0.058, suit),
            _part(f"foot_{side}", f"foot_{side}", "box", (0.10, 0.24, 0.06), (sx * 0.10, -0.05, 0.03), "M_PlasticBlack"),
        ]
    for e in extras:
        parts.append(_part(*e))
    bpy.ops.object.select_all(action="DESELECT")
    for p in parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    bpy.ops.object.join()
    body = bpy.context.object
    body.name = name
    body.data.name = name
    return body


def bind(body, arm):
    body.parent = arm
    mod = body.modifiers.new("Armature", "ARMATURE")
    mod.object = arm


# --------------------------------------------------------------------------
# Animation. Poses are written as (x, y, z) degrees per bone where +x always means
# "swing/bend FORWARD" (toward -Y); the sign per bone is measured from the rig itself.
# Locations are world-space offsets in metres (converted to bone space when keyed).
# --------------------------------------------------------------------------
SIGNS = {}


def measure_signs(arm):
    for name in BONES:
        pb = arm.pose.bones[name]
        pb.rotation_mode = "XYZ"
        before = (arm.matrix_world @ pb.tail).copy()
        pb.rotation_euler = (math.radians(30), 0, 0)
        bpy.context.view_layer.update()
        after = arm.matrix_world @ pb.tail
        pb.rotation_euler = (0, 0, 0)
        bpy.context.view_layer.update()
        dy, dz = after.y - before.y, after.z - before.z
        # downward bones: forward = -Y; upward bones: forward lean also = -Y
        SIGNS[name] = 1 if dy < -1e-6 else -1


def pose_key(arm, frame, pose, loc=None):
    for name, (rx, ry, rz) in pose.items():
        pb = arm.pose.bones[name]
        pb.rotation_mode = "XYZ"
        pb.rotation_euler = (math.radians(rx * SIGNS[name]), math.radians(ry), math.radians(rz))
        pb.keyframe_insert("rotation_euler", frame=frame)
    for name, v in (loc or {}).items():
        pb = arm.pose.bones[name]
        to_bone = arm.data.bones[name].matrix_local.to_3x3().inverted()
        pb.location = to_bone @ Vector(v)
        pb.keyframe_insert("location", frame=frame)


ZERO = {b: (0, 0, 0) for b in BONES}
ZLOC = {"pelvis": (0, 0, 0)}


def P(*layers):
    """Pose = rest, then each layer (dict bone -> (x, y, z)) on top, later layers win."""
    d = dict(ZERO)
    for layer in layers:
        d.update(layer)
    return d


# relaxed arm hang + a tiny bend so mannequins don't look like a T-pose
RELAX = dict(upperarm_l=(2, 0, -4), upperarm_r=(2, 0, 4), lowerarm_l=(8, 0, 0), lowerarm_r=(8, 0, 0))
SEAT = (0, 0.10, -0.42)    # pelvis offset when seated on a 46 cm chair (down and back)
SIT = dict(thigh_l=(90, 0, 3), thigh_r=(90, 0, -3), calf_l=(-90, 0, 0), calf_r=(-90, 0, 0), spine=(4, 0, 0))
TYPE_ARMS = dict(upperarm_l=(38, 0, -6), upperarm_r=(38, 0, 6), lowerarm_l=(52, 0, 6), lowerarm_r=(52, 0, -6))


def make_action(arm, name, frames, keys, loop=True):
    """keys: list of (frame, pose_dict, loc_dict)."""
    act = bpy.data.actions.new(name)
    arm.animation_data_create()
    arm.animation_data.action = act
    for f, pose, loc in keys:
        pose_key(arm, f, pose, loc)
    act.frame_range = (1, frames)
    act.use_frame_range = True
    act.use_fake_user = True
    return act


def build_actions(arm):
    acts = []

    # Idle: breathe (chest), weight shift (pelvis side), head glance
    keys = []
    for fr, br, sh, hy in ((1, 0, 0, 0), (16, 2, 1.5, 6), (31, 0, 0, 0), (46, 2, -1.5, -5), (60, 0, 0, 0)):
        keys.append((fr, P(RELAX, dict(chest=(br, 0, 0), pelvis=(0, 0, sh), head=(0, 0, hy))), ZLOC))
    acts.append(make_action(arm, "Idle", 60, keys))

    # Walk (in place): two steps per second
    keys = []
    for fr, s in ((1, 1), (8, 0), (16, -1), (23, 0), (30, 1)):
        pose = P(dict(thigh_l=(24 * s, 0, 0), thigh_r=(-24 * s, 0, 0),
                      calf_l=(-30 if s < 0 else -5, 0, 0), calf_r=(-30 if s > 0 else -5, 0, 0),
                      upperarm_l=(-18 * s, 0, -4), upperarm_r=(18 * s, 0, 4),
                      lowerarm_l=(15, 0, 0), lowerarm_r=(15, 0, 0),
                      chest=(0, 0, 4 * s), pelvis=(0, 0, -3 * s)))
        keys.append((fr, pose, {"pelvis": (0, 0, -0.02 if s == 0 else 0.0)}))
    acts.append(make_action(arm, "Walk", 30, keys))

    # SitDown: stand -> seated (play reversed to stand up)
    half = dict(thigh_l=(50, 0, 3), thigh_r=(50, 0, -3), calf_l=(-55, 0, 0), calf_r=(-55, 0, 0), spine=(18, 0, 0))
    keys = [(1, P(RELAX), ZLOC),
            (12, P(RELAX, half), {"pelvis": (0, 0.06, SEAT[2] * 0.55)}),
            (30, P(RELAX, SIT), {"pelvis": SEAT})]
    acts.append(make_action(arm, "SitDown", 30, keys, loop=False))

    # SitType: seated, hands on the keyboard, small taps and glances between screens
    keys = []
    for fr, tl, tr, hy in ((1, 0, 0, 0), (8, 6, 0, 2), (15, 0, 6, 2), (22, 5, 0, -3), (30, 0, 4, -3),
                           (38, 6, 0, 8), (46, 0, 5, 8), (53, 4, 0, 0), (60, 0, 0, 0)):
        pose = P(SIT, TYPE_ARMS, dict(lowerarm_l=(52 + tl, 0, 6), lowerarm_r=(52 + tr, 0, -6), head=(6, 0, hy)))
        keys.append((fr, pose, {"pelvis": SEAT}))
    acts.append(make_action(arm, "SitType", 60, keys))

    # Reach: right arm forward and up (fridge handle / coffee machine), then back
    reach = P(RELAX, dict(upperarm_r=(80, 0, 10), lowerarm_r=(10, 0, 0), chest=(6, 0, -6)))
    keys = [(1, P(RELAX), ZLOC), (12, reach, ZLOC), (20, reach, ZLOC), (30, P(RELAX), ZLOC)]
    acts.append(make_action(arm, "Reach", 30, keys, loop=False))

    # Stare: dead still, head tilted a few degrees too far
    stare = P(dict(head=(-3, 0, 0), neck=(0, 9, 0)))
    acts.append(make_action(arm, "Stare", 60, [(1, stare, ZLOC), (60, stare, ZLOC)]))
    return acts


VARIANTS = [
    ("SK_Mannequin", "M_FabricMint", [("badge", "chest", "box", (0.08, 0.01, 0.11), (0.1, -0.115, 1.28), "M_PlasticWhite")]),
    ("SK_MannequinGuard", "M_BinderBlue", [("cap", "head", "cyl", (0.12, 0.05), (0, 0, 1.78), "M_MetalDark")]),
    ("SK_MannequinCustodian", "M_Yellow", []),
]


def export(arm, body, path):
    bpy.ops.object.select_all(action="DESELECT")
    arm.select_set(True)
    body.select_set(True)
    bpy.context.view_layer.objects.active = arm
    bpy.ops.export_scene.fbx(
        filepath=path, use_selection=True, object_types={"ARMATURE", "MESH"},
        apply_scale_options="FBX_SCALE_UNITS", add_leaf_bones=False, primary_bone_axis="Y", secondary_bone_axis="X",
        bake_anim=True, bake_anim_use_all_actions=True, bake_anim_use_nla_strips=False,
        bake_anim_force_startend_keying=True, mesh_smooth_type="FACE")


def main():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.context.scene.render.fps = FPS
    os.makedirs(OUT, exist_ok=True)
    arm = build_armature()
    measure_signs(arm)
    acts = build_actions(arm)
    arm.animation_data.action = acts[0]
    bodies = []
    for name, suit, extras in VARIANTS:
        body = build_body(name, suit, extras)
        bind(body, arm)
        bodies.append(body)
    # export each variant with the shared skeleton + all actions
    for body in bodies:
        for other in bodies:
            other.hide_set(other is not body)
        export(arm, body, os.path.join(OUT, body.name + ".fbx"))
    for b in bodies:
        b.hide_set(False)
    for i, b in enumerate(bodies):
        b.location.x = 0
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT, "Mannequin_Rig.blend"))
    print(f"[TAKE 4471] Rig: {len(BONES)} bones, {len(acts)} actions ({', '.join(a.name for a in acts)}), "
          f"{len(bodies)} variants -> {OUT}")


if __name__ == "__main__":
    main()
