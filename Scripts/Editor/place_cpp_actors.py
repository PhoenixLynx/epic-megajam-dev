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
    at = chair.get_actor_location() if chair and (chair.get_actor_location() - target).length() < 80 else target
    cw = actor_sys.spawn_actor_from_class(unreal.TakeCoworker, unreal.Vector(at.x, at.y, 92), unreal.Rotator(0, 0, -90))
    cw.set_actor_label(f"{PREFIX}Coworker_Seated_{row}{col}")
    cw.set_folder_path("Gameplay/Coworkers")
    if mannequin:
        cw.set_editor_property("body_mesh", mannequin)
    cw.set_editor_property("npc_role", "Coworker")
    cw.set_editor_property("sits_at_desk", True)
    cw.set_editor_property("wanders", False)
    cw.set_editor_property("tags", [unreal.Name("NPC"), unreal.Name("Seated")])
    if chair:   # keep the chair under them still
        chair.static_mesh_component.set_simulate_physics(False)
    seated += 1

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log(f"[TAKE 4471] Seated {seated} coworker(s) at their desks.")
unreal.log(f"[TAKE 4471] Placed Fear Director, fog, loop trigger, stalker, {made} coworker(s), {len(WORKSPOTS)} work spots, "
           f"{tables}/3 DataTables. Press Play: E talks / opens / picks up; E at your monitor logs in; 1-4 / 0 = Fear.")
