"""Quick check of the SourceArt folders (plain Python, no Blender or Unreal needed):
    python Scripts/validate/check_art.py

Flags: files in the wrong place, names without a prefix, textures that don't match an asset,
exports older than their .blend (you changed the model but didn't export).
"""
import os
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "SourceArt")
PREFIX = {"export": ("SM_", "SK_", "A_"), "40_Textures": ("T_",)}
TEX_SUFFIXES = ("_BaseColor", "_Normal", "_ORM", "_Emissive", "_Mask")


def main():
    problems, notes = [], []
    exports = set()
    for area in sorted(os.listdir(ROOT)):
        export = os.path.join(ROOT, area, "export")
        blend = os.path.join(ROOT, area, "blend")
        if os.path.isdir(export):
            for f in os.listdir(export):
                if f.startswith("."):
                    continue
                stem, ext = os.path.splitext(f)
                if ext.lower() != ".fbx":
                    problems.append(f"{area}/export/{f}: only .fbx belongs here")
                elif not stem.startswith(PREFIX["export"]):
                    problems.append(f"{area}/export/{f}: name needs SM_ / SK_ / A_")
                exports.add(stem.split("_", 1)[-1])
            if os.path.isdir(blend):
                newest_blend = max((os.path.getmtime(os.path.join(blend, b)) for b in os.listdir(blend) if b.endswith(".blend")), default=0)
                newest_fbx = max((os.path.getmtime(os.path.join(export, x)) for x in os.listdir(export) if x.endswith(".fbx")), default=0)
                if newest_blend > newest_fbx + 5:
                    notes.append(f"{area}: a .blend is newer than its exports: run export_all.py")
    tex = os.path.join(ROOT, "40_Textures")
    if os.path.isdir(tex):
        for f in os.listdir(tex):
            if f.startswith("."):
                continue
            stem, _ = os.path.splitext(f)
            if not stem.startswith("T_"):
                problems.append(f"40_Textures/{f}: name needs T_<Asset>_<Map>")
            elif stem.endswith(TEX_SUFFIXES):
                asset = stem[2:].rsplit("_", 1)[0]
                if exports and asset not in exports:
                    notes.append(f"40_Textures/{f}: no exported asset named *_{asset} (typo?)")
    for p in problems:
        print("X", p)
    for n in notes:
        print("!", n)
    print(f"check_art: {len(problems)} problem(s), {len(notes)} note(s)")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
