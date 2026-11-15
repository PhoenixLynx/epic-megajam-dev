"""
fix_prop_collision.py: TAKE 4471. Give every prop mesh simple collision so physics works.

Fixes the Play-time warning, repeated for every pick-up-able prop:
  "Trying to simulate physics on ... but it has ComplexAsSimple collision."
A mesh that only has its triangle (complex) collision can't be simulated, so the props froze
in mid-air instead of falling onto counters or being thrown.

For every static mesh in /Game/Props/Meshes:
  - no simple collision yet  -> add a box (fits the prop's bounds)
  - pick-up-able props       -> collision complexity = "Project Default" (simple for physics)
  - big furniture / walls    -> left on complex collision so you can walk snugly around them
Safe to re-run. import_jam_assets.py runs it automatically at the end.

The physics list is the fixed set below PLUS every mesh used by an actor tagged "Throwable" in the
open level, so a new throwable (brochures, paper balls, lunch...) can't be missed again. Run it with
the campus level open.
"""
import unreal

MESH_DIR = "/Game/Props/Meshes"
# Keep in step with THROWABLE in dress_campus.py, plus the doors that swing.
PHYSICS = {"SM_CoffeeBag", "SM_TeaBox", "SM_CupStack", "SM_SnackBag", "SM_SnackBox", "SM_SodaCan",
           "SM_WaterBottle", "SM_MilkCarton", "SM_LunchBox", "SM_FruitBowl", "SM_Mug", "SM_Keyboard",
           "SM_Stapler", "SM_TapeDispenser", "SM_PenCup", "SM_DeskLamp", "SM_DeskPhone", "SM_Binder",
           "SM_BinderMint", "SM_BinderRed", "SM_ReamOfPaper", "SM_FireExtinguisher", "SM_WetFloorSign",
           "SM_TrashBin", "SM_PlantDesk", "SM_Laptop", "SM_PCTower", "SM_CoffeePot", "SM_Kettle",
           "SM_Microwave", "SM_Monitor", "SM_MeetingChair", "SM_BarStool", "SM_CoffeeMachine",
           "SM_OfficeChair", "SM_MopBucket", "SM_FridgeDoor", "SM_Door",
           # added after the list drifted from dress_campus.THROWABLE (they froze: lunch, printouts, confetti)
           "SM_Brochure", "SM_PaperBall", "SM_Sandwich", "SM_PizzaSlice", "SM_SaladBowl", "SM_Donut"}

# ...and whatever the level actually tags as Throwable
_from_level = set()
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if unreal.Name("Throwable") in actor.tags and isinstance(actor, unreal.StaticMeshActor):
        mesh = actor.static_mesh_component.static_mesh
        if mesh:
            _from_level.add(mesh.get_name())
_new = _from_level - PHYSICS
if _new:
    unreal.log(f"[TAKE 4471] Prop collision: also simple collision for level throwables {sorted(_new)}")
PHYSICS |= _from_level

sme = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
added = switched = 0
for path in unreal.EditorAssetLibrary.list_assets(MESH_DIR, recursive=False):
    sm = unreal.EditorAssetLibrary.load_asset(path)
    if not isinstance(sm, unreal.StaticMesh):
        continue
    name = sm.get_name()
    changed = False
    if sme.get_simple_collision_count(sm) == 0:
        sme.add_simple_collisions(sm, unreal.ScriptCollisionShapeType.BOX)
        added += 1
        changed = True
    want = unreal.CollisionTraceFlag.CTF_USE_DEFAULT if name in PHYSICS else unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE
    body = sm.get_editor_property("body_setup")
    if body is not None:
        if body.get_editor_property("collision_trace_flag") != want:
            body.set_editor_property("collision_trace_flag", want)
            switched += 1
            changed = True
    elif hasattr(sme, "set_collision_complexity"):
        sme.set_collision_complexity(sm, want)
        switched += 1
        changed = True
    if changed:
        unreal.EditorAssetLibrary.save_loaded_asset(sm)

unreal.log(f"[TAKE 4471] Prop collision: added boxes to {added} mesh(es), set complexity on {switched}. "
           f"Physics props now use simple collision.")
