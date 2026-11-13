"""
place_cpp_actors.py: TAKE 4471. Run AFTER the C++ module is compiled (see docs/TONIGHT.md step 8).

Places the gameplay actors into L_Campus_Greybox:
  - TakeFearDirector       (auto-wires to MPC_Fear, GB_PPV_Fear, fog, loop, stalker, FearLight lights)
  - ExponentialHeightFog   (Fear thickens it)
  - TakeLoopTrigger        (replaces the GB_LoopTrigger TriggerBox in Corridor B)
  - TakeStalker            (on GB_Stalker_Spawn_Act1, dormant + hidden until Fear stage 4)
Safe to re-run (removes previous CPP_ actors).
"""
import unreal

actor_sys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
PREFIX = "CPP_"

for cls in ("TakeFearDirector", "TakeLoopTrigger", "TakeStalker"):
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
spawn(unreal.ExponentialHeightFog, "Fog", unreal.Vector(2000, 3000, 0))

loop_loc = loc_of("GB_LoopTrigger", (1400, 4950, 150))
if "GB_LoopTrigger" in by_label:
    actor_sys.destroy_actor(by_label["GB_LoopTrigger"])
loop = spawn(unreal.TakeLoopTrigger, "LoopTrigger_CorridorB", loop_loc)
loop.set_editor_property("loop_offset", unreal.Vector(0, -1000, 0))

stalker_loc = loc_of("GB_Stalker_Spawn_Act1", (1400, 5650, 50))
stalker = spawn(unreal.TakeStalker, "Stalker", unreal.Vector(stalker_loc.x, stalker_loc.y, 120), yaw=-90)

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log("[TAKE 4471] Placed Fear Director, fog, loop trigger and stalker. Select CPP_FearDirector and drag Fear 0 -> 1.")
