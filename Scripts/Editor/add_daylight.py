"""
add_daylight.py: TAKE 4471. Gives the grey-box campus proper bright daylight.

The grey-box has no ceilings, so a sun + sky floods the rooms like a glass-roofed
atrium: bright, even HALCYON light. Fixes the "Exposure -8.5 outside safe range"
warning (the level had no sun or sky, so auto-exposure maxed out).

Adds (or replaces, if re-run) in the current level:
  GB_Sun          DirectionalLight, atmosphere sun, soft angle
  GB_SkyAtmosphere
  GB_SkyLight     real-time capture (so the sky light sees the real sky, not black)
  GB_HeightFog    light fog (the Fear Director thickens fog with Fear)
and brightens the room point lights.

Run: Tools > Execute Python Script... (with L_Campus_Greybox open). Safe to re-run.
"""
import unreal

actor_sys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
LABELS = ("GB_Sun", "GB_SkyAtmosphere", "GB_SkyLight", "GB_HeightFog")

for a in actor_sys.get_all_level_actors():
    if a.get_actor_label() in LABELS:
        actor_sys.destroy_actor(a)


def spawn(cls, label, loc, rot=unreal.Rotator(0, 0, 0)):
    a = actor_sys.spawn_actor_from_class(cls, loc, rot)
    a.set_actor_label(label)
    a.set_folder_path("Greybox/Lights")
    return a


# Sun: high, slightly angled so shadows read; soft
sun = spawn(unreal.DirectionalLight, "GB_Sun", unreal.Vector(2000, 3000, 2000), unreal.Rotator(-15, -60, -35))
sc = sun.light_component
sc.set_intensity(6.0)                                         # lux
sc.set_editor_property("atmosphere_sun_light", True)
sc.set_editor_property("light_source_angle", 2.0)
sc.set_light_color(unreal.LinearColor(1.0, 0.97, 0.92, 1.0))   # warm white

spawn(unreal.SkyAtmosphere, "GB_SkyAtmosphere", unreal.Vector(0, 0, 0))

sky = spawn(unreal.SkyLight, "GB_SkyLight", unreal.Vector(2000, 3000, 800))
skc = sky.light_component
skc.set_editor_property("real_time_capture", True)
skc.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
skc.set_intensity(1.2)

fog = spawn(unreal.ExponentialHeightFog, "GB_HeightFog", unreal.Vector(2000, 3000, 0))
fog.component.set_fog_density(0.02)

# The old sky light that only saw black
for a in actor_sys.get_all_level_actors():
    if a.get_actor_label() == "GB_SkyLight" and a != sky:
        actor_sys.destroy_actor(a)

# Brighter office lights (still dim/flicker with Fear: they're tagged FearLight)
n = 0
for a in actor_sys.get_all_level_actors():
    label = a.get_actor_label()
    if label.startswith("GB_Light_") and isinstance(a, unreal.PointLight) and "Hospital" not in label:
        a.point_light_component.set_intensity(20000.0)
        n += 1

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log(f"[TAKE 4471] Daylight added (sun, sky atmosphere, real-time sky light, fog); {n} room lights brightened")
