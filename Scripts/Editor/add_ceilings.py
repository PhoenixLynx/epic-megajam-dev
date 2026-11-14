"""
add_ceilings.py: TAKE 4471. Modern HALCYON ceilings, panel lighting and wayfinding signs.

- White flat ceiling over every campus room (not the Hospital ward: you fall into it).
- Recessed linear LED panels on a 3 m grid. They glow via M_CeilingPanel, which
  DIMS WITH FEAR (MPC_Fear). Every other panel carries a soft shadowless RectLight
  tagged FearLight, so the Fear Director dims and flickers them.
- Corridor B: fewer, dimmer panels (it should already feel off).
- Replaces the old over-bright room point lights.
- Hanging wayfinding signs over doorways: KITCHEN, CONFERENCE, OPEN OFFICE, CORRIDOR B ...

Run with L_Campus_Greybox open: Tools > Execute Python Script... Safe to re-run (labels CL_).
Signs read backwards? Set SIGN_YAW_FIX = 180 and run again.
Too dark / too bright? Change PANEL_LIGHT_CD (light per panel, candelas).
"""
import math

import unreal

PREFIX = "CL_"
CEILING_Z = 300.0
PANEL_SPACING = 300.0
PANEL_LIGHT_CD = 900.0        # per RectLight, candelas
CORRIDOR_LIGHT_CD = 350.0
SIGN_YAW_FIX = 0.0
CUBE = "/Engine/BasicShapes/Cube.Cube"

ROOMS = [  # name, x, y, w, d (from build_greybox_campus.py), z
    ("Elevator", 1800, 0, 600, 500, 0), ("Lobby", 1400, 500, 1400, 1200, 0),
    ("ConferenceRoom", 200, 500, 1200, 1200, 0), ("BreakRoom", 2800, 500, 1000, 1000, 0),
    ("OpenOffice", 800, 1700, 2600, 2000, 0), ("CorridorB", 1200, 3700, 400, 2000, 0),
    ("SupplyCloset", 2800, 3700, 500, 500, 0), ("WellnessCenter", 800, 5700, 1600, 1400, 0),
    ("HospitalWard", 400, 5300, 2800, 2400, -1500),
]

# Doorway signs: (text, x, y, viewer direction (dx, dy) = which side it's read from)
SIGNS = [
    ("ELEVATOR", 2100, 515, (0, 1)),
    ("KITCHEN", 2785, 1000, (-1, 0)),
    ("CONFERENCE", 1415, 1100, (1, 0)),
    ("OPEN OFFICE", 2100, 1685, (0, -1)),
    ("LOBBY  ·  KITCHEN", 2100, 1715, (0, 1)),
    ("CORRIDOR B", 1400, 3685, (0, -1)),
    ("SUPPLY", 3050, 3685, (0, -1)),
    ("WELLNESS CENTER", 1400, 5685, (0, -1)),
]

actor_sys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
lib = unreal.EditorAssetLibrary
mel = unreal.MaterialEditingLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()
cube = lib.load_asset(CUBE)


def get_or_create(name, folder, cls, factory):
    path = f"{folder}/{name}"
    return lib.load_asset(path) if lib.does_asset_exist(path) else tools.create_asset(name, folder, cls, factory)


# --- Materials: ceiling (plain white) + glowing panel that dims with Fear --------
mpc = lib.load_asset("/Game/Fear/MPC_Fear") if lib.does_asset_exist("/Game/Fear/MPC_Fear") else None

panel_mat = get_or_create("M_CeilingPanel", "/Game/Fear", unreal.Material, unreal.MaterialFactoryNew())
mel.delete_all_material_expressions(panel_mat)
colour = mel.create_material_expression(panel_mat, unreal.MaterialExpressionConstant3Vector, -700, 0)
colour.set_editor_property("constant", unreal.LinearColor(1.0, 0.98, 0.94, 1.0))
strength = mel.create_material_expression(panel_mat, unreal.MaterialExpressionConstant, -700, 150)
strength.set_editor_property("r", 6.0)
glow = mel.create_material_expression(panel_mat, unreal.MaterialExpressionMultiply, -450, 50)
mel.connect_material_expressions(colour, "", glow, "A")
mel.connect_material_expressions(strength, "", glow, "B")
if mpc:
    fear = mel.create_material_expression(panel_mat, unreal.MaterialExpressionCollectionParameter, -700, 300)
    fear.set_editor_property("collection", mpc)
    fear.set_editor_property("parameter_name", "Fear")
    dim = mel.create_material_expression(panel_mat, unreal.MaterialExpressionLinearInterpolate, -450, 250)
    dim.set_editor_property("const_a", 1.0)
    dim.set_editor_property("const_b", 0.12)
    mel.connect_material_expressions(fear, "", dim, "Alpha")
    out = mel.create_material_expression(panel_mat, unreal.MaterialExpressionMultiply, -200, 100)
    mel.connect_material_expressions(glow, "", out, "A")
    mel.connect_material_expressions(dim, "", out, "B")
    mel.connect_material_property(out, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
else:
    mel.connect_material_property(glow, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
mel.connect_material_property(colour, "", unreal.MaterialProperty.MP_BASE_COLOR)
mel.recompile_material(panel_mat)
lib.save_loaded_asset(panel_mat)

ceiling_mat = None
if lib.does_asset_exist("/Game/Fear/MI_Greybox_Walls"):
    ceiling_mat = lib.load_asset("/Game/Fear/MI_Greybox_Walls")   # off-white, rots with Fear

sign_bg = lib.load_asset("/Game/Fear/MI_Greybox_Props") if lib.does_asset_exist("/Game/Fear/MI_Greybox_Props") else None


# --- Clean up previous runs + the old over-bright point lights ------------------
removed = 0
for a in actor_sys.get_all_level_actors():
    label = a.get_actor_label()
    old_light = label.startswith("GB_Light_") and "Hospital" not in label
    if label.startswith(PREFIX) or old_light:
        actor_sys.destroy_actor(a)
        removed += 1

n = [0]


def box(label, x, y, z, sx, sy, sz, mat, folder):
    a = actor_sys.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator(0, 0, 0))
    a.static_mesh_component.set_static_mesh(cube)
    a.set_actor_scale3d(unreal.Vector(sx / 100.0, sy / 100.0, sz / 100.0))
    if mat:
        a.static_mesh_component.set_material(0, mat)
    a.static_mesh_component.set_editor_property("cast_shadow", False)
    a.set_actor_label(f"{PREFIX}{label}_{n[0]:03d}")
    a.set_folder_path(f"Ceilings/{folder}")
    n[0] += 1
    return a


lights = 0
for name, x, y, w, d, z0 in ROOMS:
    zc = z0 + CEILING_Z
    if name != "HospitalWard":   # the ward stays open above: the Act 3 trust fall drops you in from the Wellness Center
        box(f"Ceiling_{name}", x + w / 2, y + d / 2, zc + 5, w + 20, d + 20, 10, ceiling_mat, name)
    corridor = name == "CorridorB"
    if corridor:
        cols, rows = 1, int(d // 250)
        sx, sy = w, d / rows
    else:
        cols, rows = max(1, int(w // PANEL_SPACING)), max(1, int(d // PANEL_SPACING))
        sx, sy = w / cols, d / rows
    for c in range(cols):
        for r in range(rows):
            px, py = x + sx * (c + 0.5), y + sy * (r + 0.5)
            box(f"Panel_{name}", px, py, zc - 1, 120, 22, 3, panel_mat, name)          # linear LED strip
            if corridor or (c % 2 == 0 and r % 2 == 0):
                rl = actor_sys.spawn_actor_from_class(unreal.RectLight, unreal.Vector(px, py, zc - 4),
                                                      unreal.Rotator(0, -90, 0))     # (roll, pitch, yaw): pitch -90 = pointing down
                comp = rl.rect_light_component
                comp.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
                comp.set_intensity(CORRIDOR_LIGHT_CD if corridor else PANEL_LIGHT_CD * 4)   # each light covers ~4 panels
                comp.set_editor_property("source_width", 120.0)
                comp.set_editor_property("source_height", 22.0)
                comp.set_editor_property("attenuation_radius", 1000.0)
                comp.set_editor_property("cast_shadows", False)
                comp.set_light_color(unreal.LinearColor(1.0, 0.97, 0.92, 1.0))
                rl.set_actor_label(f"{PREFIX}Light_{name}_{n[0]:03d}")
                rl.set_folder_path(f"Ceilings/{name}")
                if z0 > -1000:
                    rl.set_editor_property("tags", [unreal.Name("FearLight")])
                n[0] += 1
                lights += 1

# --- Wayfinding signs ------------------------------------------------------------
for text, x, y, (dx, dy) in SIGNS:
    yaw = math.degrees(math.atan2(dy, dx)) + SIGN_YAW_FIX
    backing = box(f"SignBack_{text.replace(' ', '')}", x - dx * 3, y - dy * 3, 262,
                  4 if dx else 150, 150 if dx else 4, 34, sign_bg, "Signs")
    t = actor_sys.spawn_actor_from_class(unreal.TextRenderActor, unreal.Vector(x, y, 252), unreal.Rotator(0, 0, yaw))
    tc = t.text_render
    tc.set_editor_property("text", text)
    tc.set_editor_property("world_size", 16.0)
    tc.set_editor_property("horizontal_alignment", unreal.HorizTextAligment.EHTA_CENTER)
    tc.set_editor_property("text_render_color", unreal.Color(r=18, g=49, b=58, a=255))
    t.set_actor_label(f"{PREFIX}Sign_{text.replace(' ', '')}")
    t.set_folder_path("Ceilings/Signs")

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log(f"[TAKE 4471] Ceilings: {len(ROOMS)} rooms, {lights} panel lights, {len(SIGNS)} signs; removed {removed} old actors")
