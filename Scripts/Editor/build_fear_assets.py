"""
build_fear_assets.py: TAKE 4471 / Director's Cut

Creates the assets the Fear system needs, then applies the Fear material to the grey-box:
  /Game/Fear/MPC_Fear          Material Parameter Collection: Fear, SoulSight (0..1)
  /Game/Fear/M_GreyboxFear     surface material that rots as Fear rises:
                               clean office colour -> grimy, desaturated, darker, wetter
  /Game/Fear/MI_Greybox_*      instances with different clean colours (walls, floors, props)

Run AFTER build_greybox_campus.py:  Tools > Execute Python Script...
Then drag BP_FearDirector's Fear slider (docs/BLUEPRINT_RECIPES.md #1) to watch it rot.
Safe to re-run.
"""
import unreal

FOLDER = "/Game/Fear"
tools = unreal.AssetToolsHelpers.get_asset_tools()
lib = unreal.EditorAssetLibrary
mel = unreal.MaterialEditingLibrary


def get_or_create(name, cls, factory):
    path = f"{FOLDER}/{name}"
    if lib.does_asset_exist(path):
        return lib.load_asset(path)
    return tools.create_asset(name, FOLDER, cls, factory)


# --- 1. Material Parameter Collection -------------------------------------
mpc = get_or_create("MPC_Fear", unreal.MaterialParameterCollection,
                    unreal.MaterialParameterCollectionFactoryNew())
params = []
for pname in ("Fear", "SoulSight"):
    p = unreal.CollectionScalarParameter()
    p.set_editor_property("parameter_name", pname)
    p.set_editor_property("default_value", 0.0)
    params.append(p)
mpc.set_editor_property("scalar_parameters", params)
lib.save_loaded_asset(mpc)

# --- 2. Master material ----------------------------------------------------
mat = get_or_create("M_GreyboxFear", unreal.Material, unreal.MaterialFactoryNew())
mel.delete_all_material_expressions(mat)


def node(cls, x, y, **props):
    e = mel.create_material_expression(mat, cls, x, y)
    for k, v in props.items():
        e.set_editor_property(k, v)
    return e


fear = node(unreal.MaterialExpressionCollectionParameter, -1200, 0,
            collection=mpc, parameter_name="Fear")
clean = node(unreal.MaterialExpressionVectorParameter, -1200, -300,
             parameter_name="CleanColor", default_value=unreal.LinearColor(0.82, 0.84, 0.86, 1))
grime = node(unreal.MaterialExpressionVectorParameter, -1200, -150,
             parameter_name="GrimeColor", default_value=unreal.LinearColor(0.20, 0.22, 0.19, 1))

# Grime creeps in unevenly: world-space noise decides where rot shows first.
noise = node(unreal.MaterialExpressionNoise, -1200, 200, scale=0.004, levels=4,
             output_min=0.0, output_max=1.0)
wp = node(unreal.MaterialExpressionWorldPosition, -1400, 200)
mel.connect_material_expressions(wp, "", noise, "Position")
# rot mask = saturate(Fear * 1.6 - noise * 0.6)
mul_f = node(unreal.MaterialExpressionMultiply, -950, 50, const_b=1.6)
mel.connect_material_expressions(fear, "", mul_f, "A")
mul_n = node(unreal.MaterialExpressionMultiply, -950, 200, const_b=0.6)
mel.connect_material_expressions(noise, "", mul_n, "A")
sub = node(unreal.MaterialExpressionSubtract, -800, 100)
mel.connect_material_expressions(mul_f, "", sub, "A")
mel.connect_material_expressions(mul_n, "", sub, "B")
mask = node(unreal.MaterialExpressionSaturate, -650, 100)
mel.connect_material_expressions(sub, "", mask, "")

lerp_col = node(unreal.MaterialExpressionLinearInterpolate, -500, -200)
mel.connect_material_expressions(clean, "", lerp_col, "A")
mel.connect_material_expressions(grime, "", lerp_col, "B")
mel.connect_material_expressions(mask, "", lerp_col, "Alpha")

# Colour drains with Fear (on top of the post-process desaturation)
desat = node(unreal.MaterialExpressionDesaturation, -300, -200)
mel.connect_material_expressions(lerp_col, "", desat, "")
mel.connect_material_expressions(fear, "", desat, "Fraction")
mel.connect_material_property(desat, "", unreal.MaterialProperty.MP_BASE_COLOR)

# Surfaces look damp as they rot: roughness 0.85 -> 0.35
rough = node(unreal.MaterialExpressionLinearInterpolate, -300, 100, const_a=0.85, const_b=0.35)
mel.connect_material_expressions(mask, "", rough, "Alpha")
mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)

mel.layout_material_expressions(mat)
mel.recompile_material(mat)
lib.save_loaded_asset(mat)

# --- 3. Instances + apply to the grey-box ----------------------------------
COLOURS = {  # folder keyword -> clean colour (bright corporate office)
    "Walls": unreal.LinearColor(0.86, 0.88, 0.90, 1),       # off-white
    "Floors": unreal.LinearColor(0.45, 0.55, 0.60, 1),      # blue-grey carpet
    "Props": unreal.LinearColor(0.55, 0.78, 0.74, 1),       # HALCYON mint
    "Throwables": unreal.LinearColor(0.95, 0.62, 0.32, 1),  # orange = you can grab it
    "Doors": unreal.LinearColor(0.70, 0.25, 0.25, 1),
    "CorridorB_Loop": unreal.LinearColor(0.70, 0.75, 0.95, 1),
}
instances = {}
for key, colour in COLOURS.items():
    mi = get_or_create(f"MI_Greybox_{key}", unreal.MaterialInstanceConstant,
                       unreal.MaterialInstanceConstantFactoryNew())
    mel.set_material_instance_parent(mi, mat)
    mel.set_material_instance_vector_parameter_value(mi, "CleanColor", colour)
    lib.save_loaded_asset(mi)
    instances[key] = mi

actor_sys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
count = 0
for a in actor_sys.get_all_level_actors():
    if not a.get_actor_label().startswith("GB_") or not isinstance(a, unreal.StaticMeshActor):
        continue
    folder = str(a.get_folder_path())
    for key, mi in instances.items():
        if folder.endswith("/" + key):
            a.static_mesh_component.set_material(0, mi)
            count += 1
            break

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.log(f"[TAKE 4471] Fear assets ready in {FOLDER}; material applied to {count} grey-box actors")
