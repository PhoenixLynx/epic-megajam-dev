"""
place_cpp_actors.py: TAKE 4471. Run AFTER the C++ module is compiled (see docs/TONIGHT.md step 8).

Places the gameplay actors into L_Campus_Greybox:
  - TakeFearDirector       (auto-wires to MPC_Fear, GB_PPV_Fear, fog, loop, stalker, FearLight lights)
  - ExponentialHeightFog   (Fear thickens it)
  - TakeLoopTrigger        (replaces the GB_LoopTrigger TriggerBox in Corridor B)
  - TakeStalker            (on GB_Stalker_Spawn_Act1, dormant + hidden until Fear stage 4)
  - TakeCoworker x N       (replaces every NPC-tagged DR_ mannequin; the original is hidden, not deleted)
  - WorkSpot TargetPoints  (coffee machine, fridge, water cooler, printer, filing, lobby: where coworkers wander)
  - DataTables             /Game/Data/DT_Emails and DT_NPCLines from Data/Text/*.csv (HALCYON Mail + dialogue)
  - TakeBeatTrigger x 2    (overnight run 1) ConferenceRoom (the 10:00 meeting slip) + CorridorB (entrance)
  - TakeWisp               (overnight run 1) on GB_SoulEnergy_Act1, the end of Corridor B, with /Game/Fear/M_Wisp
Safe to re-run (removes previous CPP_ actors). Re-run it after dress_campus.py.
"""
import unreal

actor_sys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
PREFIX = "CPP_"

import os
import random

for cls in ("TakeFearDirector", "TakeLoopTrigger", "TakeStalker", "TakeCoworker"):
    if not hasattr(unreal, cls):
        raise RuntimeError(f"unreal.{cls} not found: is the C++ module compiled and loaded?")

by_label = {}
for a in actor_sys.get_all_level_actors():
    label = a.get_actor_label()
    by_label[label] = a
    if label.startswith(PREFIX):
        actor_sys.destroy_actor(a)


def spawn(cls, label, loc, yaw=0.0):
    a = actor_sys.spawn_actor_from_class(cls, loc, unreal.Rotator(0, 0, yaw))
    a.set_actor_label(PREFIX + label)
    a.set_folder_path("Gameplay")
    return a


def loc_of(label, fallback):
    a = by_label.get(label)
    return a.get_actor_location() if a else unreal.Vector(*fallback)


fear = spawn(unreal.TakeFearDirector, "FearDirector", unreal.Vector(2000, 3000, 300))
if not any(isinstance(a, unreal.ExponentialHeightFog) for a in actor_sys.get_all_level_actors()):
    spawn(unreal.ExponentialHeightFog, "Fog", unreal.Vector(2000, 3000, 0))   # add_daylight.py usually made one

loop_loc = loc_of("GB_LoopTrigger", (1400, 4950, 150))
if "GB_LoopTrigger" in by_label:
    actor_sys.destroy_actor(by_label["GB_LoopTrigger"])
loop = spawn(unreal.TakeLoopTrigger, "LoopTrigger_CorridorB", loop_loc)
loop.set_editor_property("loop_offset", unreal.Vector(0, -1000, 0))

stalker_loc = loc_of("GB_Stalker_Spawn_Act1", (1400, 5650, 50))
stalker = spawn(unreal.TakeStalker, "Stalker", unreal.Vector(stalker_loc.x, stalker_loc.y, 120), yaw=-90)


# --- Act 1 story (overnight run 1): beat triggers + the Corridor B wisp -------------
def make_wisp_material():
    """/Game/Fear/M_Wisp: unlit, bright mint emissive. Made once; re-runs keep it."""
    path = "/Game/Fear/M_Wisp"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.EditorAssetLibrary.load_asset(path)
    mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset("M_Wisp", "/Game/Fear", unreal.Material, unreal.MaterialFactoryNew())
    if mat is None:
        unreal.log_warning("[TAKE 4471] Could not create M_Wisp; the wisp will use the default material")
        return None
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mel = unreal.MaterialEditingLibrary
    col = mel.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -400, 0)
    col.set_editor_property("constant", unreal.LinearColor(0.5, 0.95, 0.85, 1.0))
    mul = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -200, 0)
    mul.set_editor_property("const_b", 12.0)
    mel.connect_material_expressions(col, "", mul, "A")
    mel.connect_material_property(mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(path)
    return mat


story_placed = []
if hasattr(unreal, "TakeBeatTrigger"):
    # Room bounds from build_greybox_campus.py: ConferenceRoom x 200-1400, y 500-1700 (door on the lobby side, x 1400);
    # CorridorB x 1200-1600, y 3700-5700 (entrance at y 3700; the loop teleports you back to y ~3950, so the
    # entrance box stays south of that).
    for name, loc, half in (("ConferenceRoom", (750, 1100, 150), (450, 500, 150)),
                            ("CorridorB", (1400, 3780, 150), (190, 60, 150))):
        bt = spawn(unreal.TakeBeatTrigger, f"Beat_{name}", unreal.Vector(*loc))
        bt.set_editor_property("beat_name", unreal.Name(name))
        bt.set_editor_property("box_half_size", unreal.Vector(*half))
        if hasattr(bt, "rerun_construction_scripts"):
            bt.rerun_construction_scripts()
        story_placed.append(f"Beat_{name}")
else:
    unreal.log_warning("[TAKE 4471] unreal.TakeBeatTrigger not found: rebuild the C++ (overnight run 1) to get the Act 1 beats")
if hasattr(unreal, "TakeWisp"):
    wisp_loc = loc_of("GB_SoulEnergy_Act1", (1400, 5400, 0))
    wisp = spawn(unreal.TakeWisp, "Wisp_Act1", unreal.Vector(wisp_loc.x, wisp_loc.y, 140))
    wisp_mat = make_wisp_material()
    if wisp_mat:
        wisp.set_editor_property("orb_material", wisp_mat)
    story_placed.append("Wisp_Act1")
else:
    unreal.log_warning("[TAKE 4471] unreal.TakeWisp not found: rebuild the C++ (overnight run 1)")


# --- DataTables ----------------------------------------------------------------
def import_table(csv_name, struct_path):
    struct = unreal.load_object(None, struct_path)
    if struct is None:
        unreal.log_warning(f"[TAKE 4471] Row struct {struct_path} not found; skipping {csv_name}")
        return False
    src = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()), "Data", "Text", csv_name)
    if not os.path.exists(src):
        unreal.log_warning(f"[TAKE 4471] Missing {src}")
        return False
    settings = unreal.CSVImportSettings()
    settings.set_editor_property("import_row_struct", struct)
    settings.set_editor_property("import_type", unreal.CSVImportType.ECSV_DATA_TABLE)
    factory = unreal.CSVImportFactory()
    factory.set_editor_property("automated_import_settings", settings)
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", src)
    task.set_editor_property("destination_path", "/Game/Data")
    task.set_editor_property("destination_name", csv_name[:-4])
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)
    task.set_editor_property("factory", factory)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return True


tables = sum(import_table(n, st) for n, st in (("DT_Emails.csv", "/Script/DirectorsCut_58.TakeEmailRow"),
                                                ("DT_NPCLines.csv", "/Script/DirectorsCut_58.TakeNPCLineRow"),
                                                ("DT_ComplianceTraining.csv", "/Script/DirectorsCut_58.TakeTrainingRow")))

# --- WorkSpots: where coworkers go to look busy ---------------------------------
WORKSPOTS = [
    ("CoffeeMachine", 3140, 650), ("Fridge", 3620, 660), ("WaterCooler", 3620, 1250), ("KitchenCounter", 3200, 680),
    ("Printer", 3300, 1760), ("Filing", 1730, 3400), ("LobbyCoffeeTable", 1760, 1540), ("OfficeAisle", 2000, 2600),
]
for name, x, y in WORKSPOTS:
    tp = spawn(unreal.TargetPoint, f"WorkSpot_{name}", unreal.Vector(x, y, 50))
    tp.set_editor_property("tags", [unreal.Name("WorkSpot")])

# --- Coworkers: swap the dressed mannequins for living(ish) ones -----------------
STORY_NPCS = {"Receptionist", "Guard_Closet", "Custodian_Closet"}
rnd = random.Random(4471)
npcs = []
for a in actor_sys.get_all_level_actors():
    tags = [str(t) for t in a.tags]
    if "NPC" in tags and a.get_actor_label().startswith("DR_") and isinstance(a, unreal.StaticMeshActor):
        npcs.append((a, tags))
agents = set(rnd.sample(range(len(npcs)), min(2, len(npcs)))) if npcs else set()
made = 0
for i, (a, tags) in enumerate(npcs):
    role_tag = next((t for t in tags if t != "NPC"), "Coworker")
    if role_tag.startswith("Guard"):
        role, wanders = "Guard", False
    elif role_tag.startswith("Custodian"):
        role, wanders = "Custodian", False                              # custodians keep to their posts
    elif role_tag == "Receptionist":
        role, wanders = "Receptionist", False
    else:
        role, wanders = "Coworker", True
    comp = a.static_mesh_component
    loc, rot = a.get_actor_location(), a.get_actor_rotation()
    cw = actor_sys.spawn_actor_from_class(unreal.TakeCoworker, unreal.Vector(loc.x, loc.y, loc.z + 92), unreal.Rotator(0, 0, rot.yaw + 90))   # body sits at -90 inside the character
    cw.set_actor_label(f"{PREFIX}Coworker_{role_tag}")
    cw.set_folder_path("Gameplay/Coworkers")
    cw.set_editor_property("body_mesh", comp.get_editor_property("static_mesh"))
    cw.set_editor_property("npc_role", role)
    cw.set_editor_property("wanders", wanders)
    cw.set_editor_property("is_agent", role == "Coworker" and i in agents)
    # Story NPCs: the only ones you talk to with E (they show a "!"); everyone else talks as you pass
    cw.set_editor_property("story_npc", role_tag in STORY_NPCS)
    cw.set_editor_property("tags", [unreal.Name("NPC"), unreal.Name(role_tag)])
    if hasattr(cw, "rerun_construction_scripts"):
        cw.rerun_construction_scripts()
    # hide the dressed original (kept so this script can be re-run)
    comp.set_editor_property("visible", False)
    a.set_actor_enable_collision(False)
    made += 1

# --- Coworkers sitting at their desks, typing (need the rigged mannequin for the seated pose) ---
SEATED_CUBICLES = [(0, 1), (1, 0), (1, 3), (2, 2)]   # (row, col) in dress_campus.py's 3 x 4 grid
chairs = [a for a in actor_sys.get_all_level_actors()
          if a.get_actor_label().startswith("DR_OfficeChair")]
mannequin = unreal.EditorAssetLibrary.load_asset("/Game/Props/Meshes/SM_Mannequin") \
    if unreal.EditorAssetLibrary.does_asset_exist("/Game/Props/Meshes/SM_Mannequin") else None
seated = 0
for row, col in SEATED_CUBICLES:
    cx, cy = 1800 + col * 350, 2100 + row * 450
    target = unreal.Vector(cx, cy - 5, 0)
    chair = min(chairs, key=lambda c: (c.get_actor_location() - target).length(), default=None)
    if chair and (chair.get_actor_location() - target).length() < 80:
        # square the chair up to the desk (never a WrongWay chair) and hold it still
        chair.set_actor_location_and_rotation(target, unreal.Rotator(0, 0, 180), False, False)
        chair.static_mesh_component.set_simulate_physics(False)
        chair.set_editor_property("tags", [unreal.Name("SeatedChair")])
    # the sitting animation puts the hips ~10 cm behind the capsule centre: stand the capsule a little forward
    cw = actor_sys.spawn_actor_from_class(unreal.TakeCoworker, unreal.Vector(cx, cy - 13, 92), unreal.Rotator(0, 0, -90))
    cw.set_actor_label(f"{PREFIX}Coworker_Seated_{row}{col}")
    cw.set_folder_path("Gameplay/Coworkers")
    if mannequin:
        cw.set_editor_property("body_mesh", mannequin)
    cw.set_editor_property("npc_role", "Coworker")
    cw.set_editor_property("sits_at_desk", True)
    cw.set_editor_property("wanders", False)
    cw.set_editor_property("tags", [unreal.Name("NPC"), unreal.Name("Seated")])
    seated += 1

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log(f"[TAKE 4471] Seated {seated} coworker(s) at their desks.")
unreal.log(f"[TAKE 4471] Act 1 story actors: {', '.join(story_placed) or 'none'}")
unreal.log(f"[TAKE 4471] Placed Fear Director, fog, loop trigger, stalker, {made} coworker(s), {len(WORKSPOTS)} work spots, "
           f"{tables}/3 DataTables. Press Play: E talks / opens / picks up; E at your monitor logs in; 1-4 / 0 = Fear.")
