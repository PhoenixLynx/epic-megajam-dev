"""
add_ambience.py: TAKE 4471. Room ambience (placeholders until SkyYamaha's recordings).
Run AFTER import_jam_assets.py, with L_Campus_Greybox open. Safe to re-run (labels AMB_).
Sounds live in /Game/Audio/Placeholder; this script also sets them to loop.
"""
import unreal

PREFIX = "AMB_"
AUDIO = "/Game/Audio/Placeholder/"
# (label, sound, x, y, z, inner radius, falloff, volume)
AMBIENCE = [
    ("Office", "amb_office_roomtone_loop", 2100, 2700, 150, 1300, 600, 0.7),
    ("Lobby", "amb_lobby_air_loop", 2100, 1100, 150, 650, 500, 0.6),
    ("Kitchen_Fridge", "amb_kitchen_fridge_loop", 3700, 560, 100, 150, 700, 0.6),
    ("Conference", "amb_office_roomtone_loop", 800, 1100, 150, 500, 400, 0.4),
    ("CorridorB_Hum", "amb_corridorb_hum_loop", 1400, 4700, 150, 300, 700, 0.7),
    ("CorridorB_Buzz", "sfx_fluoro_buzz_loop", 1400, 4300, 280, 100, 500, 0.35),
    ("Wellness", "amb_lobby_air_loop", 1600, 6400, 150, 700, 400, 0.4),
    ("Hospital", "amb_corridorb_hum_loop", 1800, 6500, -1350, 1400, 600, 0.8),
]

actor_sys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
lib = unreal.EditorAssetLibrary
for a in actor_sys.get_all_level_actors():
    if a.get_actor_label().startswith(PREFIX):
        actor_sys.destroy_actor(a)

placed, missing = 0, []
for label, sound, x, y, z, radius, falloff, volume in AMBIENCE:
    path = AUDIO + sound
    if not lib.does_asset_exist(path):
        missing.append(sound)
        continue
    wave = lib.load_asset(path)
    wave.set_editor_property("looping", True)
    lib.save_loaded_asset(wave)
    amb = actor_sys.spawn_actor_from_class(unreal.AmbientSound, unreal.Vector(x, y, z), unreal.Rotator(0, 0, 0))
    comp = amb.audio_component
    comp.set_sound(wave)
    comp.set_editor_property("volume_multiplier", volume)
    comp.set_editor_property("override_attenuation", True)
    att = comp.get_editor_property("attenuation_overrides")
    att.set_editor_property("attenuate", True)
    att.set_editor_property("attenuation_shape_extents", unreal.Vector(radius, 0, 0))
    att.set_editor_property("falloff_distance", falloff)
    comp.set_editor_property("attenuation_overrides", att)
    amb.set_actor_label(PREFIX + label)
    amb.set_folder_path("Audio/Ambience")
    placed += 1

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log(f"[TAKE 4471] Ambience: placed {placed}" + (f"; missing {missing} (run import_jam_assets.py)" if missing else ""))
