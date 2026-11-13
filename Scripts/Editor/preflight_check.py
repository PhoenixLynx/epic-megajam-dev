"""
preflight_check.py: TAKE 4471. Run this FIRST tonight (Tools > Execute Python Script...).

1. Checks every Unreal Python feature our builder scripts use, and reports
   PASS/FAIL for each, so we know what will break before building anything.
2. Starts the Unreal MCP server and generates the Claude Code client config
   (.mcp.json in the project folder).
3. Writes a report to  Saved/Take4471_preflight.txt , which Claude can read
   from the cloud and fix scripts from, even while you're away from the PC.
"""
import os
import traceback

import unreal

results = []


def check(label, fn):
    try:
        ok = fn()
        results.append(("PASS" if ok else "FAIL", label, ""))
    except Exception as e:  # noqa: BLE001 - we want every failure reported
        results.append(("FAIL", label, f"{type(e).__name__}: {e}"))


has = lambda obj, name: hasattr(obj, name)  # noqa: E731

# --- Engine + plugins ------------------------------------------------------
results.append(("INFO", "Engine version", unreal.SystemLibrary.get_engine_version()))
check("EditorActorSubsystem", lambda: unreal.get_editor_subsystem(unreal.EditorActorSubsystem) is not None)
check("LevelEditorSubsystem.new_level", lambda: has(unreal.LevelEditorSubsystem, "new_level"))
check("LevelEditorSubsystem.load_level", lambda: has(unreal.LevelEditorSubsystem, "load_level"))
check("EditorAssetLibrary (EditorScriptingUtilities)", lambda: has(unreal.EditorAssetLibrary, "load_asset"))
check("Cube mesh /Engine/BasicShapes/Cube", lambda: unreal.EditorAssetLibrary.does_asset_exist("/Engine/BasicShapes/Cube"))
check("First Person game mode asset",
      lambda: unreal.EditorAssetLibrary.does_asset_exist("/Game/FirstPerson/Blueprints/BP_FirstPersonGameMode"))

# --- Classes the grey-box builder spawns -----------------------------------
for cls in ("StaticMeshActor", "PointLight", "SkyLight", "TargetPoint", "PostProcessVolume",
            "PlayerStart", "TriggerBox", "NavMeshBoundsVolume", "ScopedSlowTask", "ComponentMobility"):
    check(f"class unreal.{cls}", lambda c=cls: has(unreal, c))
check("StaticMeshComponent.set_notify_rigid_body_collision",
      lambda: has(unreal.StaticMeshComponent, "set_notify_rigid_body_collision"))

# --- Fear materials script ---------------------------------------------------
for cls in ("MaterialParameterCollection", "MaterialParameterCollectionFactoryNew", "CollectionScalarParameter",
            "MaterialFactoryNew", "MaterialInstanceConstantFactoryNew", "MaterialEditingLibrary",
            "MaterialExpressionCollectionParameter", "MaterialExpressionVectorParameter",
            "MaterialExpressionNoise", "MaterialExpressionWorldPosition", "MaterialExpressionMultiply",
            "MaterialExpressionSubtract", "MaterialExpressionSaturate", "MaterialExpressionLinearInterpolate",
            "MaterialExpressionDesaturation"):
    check(f"class unreal.{cls}", lambda c=cls: has(unreal, c))
for fn in ("create_material_expression", "connect_material_expressions", "connect_material_property",
           "delete_all_material_expressions", "layout_material_expressions", "recompile_material",
           "set_material_instance_parent", "set_material_instance_vector_parameter_value"):
    check(f"MaterialEditingLibrary.{fn}", lambda f=fn: has(unreal.MaterialEditingLibrary, f))

# --- Unreal MCP ----------------------------------------------------------------
project_dir = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())


def run_cmd(cmd):
    unreal.SystemLibrary.execute_console_command(None, cmd)
    return True


check("MCP: ModelContextProtocol.StartServer", lambda: run_cmd("ModelContextProtocol.StartServer"))
check("MCP: GenerateClientConfig ClaudeCode", lambda: run_cmd("ModelContextProtocol.GenerateClientConfig ClaudeCode"))
check("MCP: .mcp.json exists in project folder", lambda: os.path.exists(os.path.join(project_dir, ".mcp.json")))

# --- Report ------------------------------------------------------------------
lines = [f"{status:4}  {label}" + (f"   ->  {detail}" if detail else "") for status, label, detail in results]
fails = sum(1 for r in results if r[0] == "FAIL")
lines.append("")
lines.append(f"{fails} FAIL / {sum(1 for r in results if r[0] == 'PASS')} PASS")
report = "\n".join(lines)
out = os.path.join(project_dir, "Saved", "Take4471_preflight.txt")
os.makedirs(os.path.dirname(out), exist_ok=True)
with open(out, "w", encoding="utf-8") as f:
    f.write(report + "\n")
unreal.log("[TAKE 4471] PREFLIGHT\n" + report + f"\nReport written to {out}")
if fails:
    unreal.log_warning(f"[TAKE 4471] {fails} preflight checks failed. Tell Claude 'read the preflight report'.")
