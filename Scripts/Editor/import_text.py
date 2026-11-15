"""
import_text.py: TAKE 4471. Re-import only the game's text tables (mail, NPC lines, training) from Data/Text/*.csv.

The same import place_cpp_actors.py does, without re-placing any actors. Use it after editing make_text_tables.py
(run `python make_text_tables.py` first, it writes the CSVs).

Run: Tools > Execute Python Script... Safe to re-run (replaces).
"""
import os

import unreal

ROOT = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
TABLES = (("DT_Emails.csv", "/Script/DirectorsCut_58.TakeEmailRow"),
          ("DT_NPCLines.csv", "/Script/DirectorsCut_58.TakeNPCLineRow"),
          ("DT_ComplianceTraining.csv", "/Script/DirectorsCut_58.TakeTrainingRow"))

done = 0
for csv_name, struct_path in TABLES:
    struct = unreal.load_object(None, struct_path)
    src = os.path.join(ROOT, "Data", "Text", csv_name)
    if not struct or not os.path.exists(src):
        unreal.log_warning(f"[TAKE 4471] Skipping {csv_name} (struct or file missing)")
        continue
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
    done += 1
unreal.log(f"[TAKE 4471] Text tables re-imported: {done}/{len(TABLES)}")
