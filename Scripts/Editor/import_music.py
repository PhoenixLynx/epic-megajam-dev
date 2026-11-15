"""
import_music.py: TAKE 4471. Bring PhoenixLynx's original music into the game.

  SourceArt/Audio/Music/*.wav  ->  /Game/Audio/Music   (e.g. mus_halcyon_positive)

The melody is composed by PhoenixLynx (Ableton, Nov 14, 2026: "Halycion_Melody_Positive").
The game plays it from code (TakeStory.cpp, PlayMusicCue): it fades in the first time you get coffee and the
first time an energy sphere pulls you out of Fear, then fades out at its end (or early if Fear comes back).
/Game/Audio is already in DirectoriesToAlwaysCook, so it makes it into a packaged build.

Run: Tools > Execute Python Script... Safe to re-run (replaces).
"""
import glob
import os

import unreal

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
DEST = "/Game/Audio/Music"

tasks = []
for f in sorted(glob.glob(os.path.join(ROOT, "SourceArt/Audio/Music/*.wav"))):
    t = unreal.AssetImportTask()
    t.set_editor_property("filename", f)
    t.set_editor_property("destination_path", DEST)
    t.set_editor_property("replace_existing", True)
    t.set_editor_property("automated", True)
    t.set_editor_property("save", True)
    tasks.append(t)
if not tasks:
    raise RuntimeError("[TAKE 4471] No music in SourceArt/Audio/Music")
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

for path in unreal.EditorAssetLibrary.list_assets(DEST, recursive=False):
    wave = unreal.EditorAssetLibrary.load_asset(path)
    if isinstance(wave, unreal.SoundWave):
        wave.set_editor_property("looping", False)   # plays once per cue
        unreal.EditorAssetLibrary.save_loaded_asset(wave)
        unreal.log(f"[TAKE 4471] Music: {wave.get_name()} ready ({wave.get_editor_property('duration'):.1f} s)")
