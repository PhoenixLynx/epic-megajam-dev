"""
dress_campus.py: TAKE 4471. Furnish the grey-box campus with the real props.

Run AFTER build_greybox_campus.py and import_jam_assets.py
(Tools > Execute Python Script...). Safe to re-run: it removes everything it
placed before (labels start with DR_).

- Replaces grey-box placeholder cubes (desk, cabinets, beds, throwables...) with meshes.
- Dresses every room: your desk #4471, 12 cubicles, lobby, kitchen, conference room,
  supply closet, Corridor B, Wellness Center and the Hospital ward.
- Places mannequin coworkers, guards and a custodian.
- Throwables get physics + the 'Throwable' tag (see docs/BLUEPRINT_RECIPES.md #2).
- The director's chair is placed by your desk but HIDDEN IN GAME (tag 'Act2Reveal').

Facing convention: yaw 0 = the prop's front faces north (-Y, toward the top of the map).
yaw 180 faces south, 90 faces east, -90 faces west.
If imported props all face the wrong way, change YAW_FIX (try 90, -90 or 180).
If they're 100x too small or big, change SCALE_FIX.
"""
import random

import unreal

MESHES = "/Game/Props/Meshes"
PREFIX = "DR_"
YAW_FIX = 180.0   # confirmed on import: Blender fronts arrive facing backwards in Unreal
SCALE_FIX = 1.0
DESK_TOP = 76.0  # cm

# Throwable meshes get physics
THROWABLE = {"SM_Mug", "SM_Keyboard", "SM_OfficeChair", "SM_ReamOfPaper", "SM_FireExtinguisher", "SM_WetFloorSign",
             "SM_MopBucket", "SM_Binder", "SM_BinderMint", "SM_BinderRed", "SM_Stapler", "SM_TapeDispenser",
             "SM_PenCup", "SM_DeskLamp", "SM_DeskPhone", "SM_Laptop", "SM_PCTower", "SM_TrashBin", "SM_PlantDesk",
             "SM_Kettle", "SM_Microwave", "SM_Monitor", "SM_MeetingChair", "SM_BarStool", "SM_CoffeeMachine"}

# Grey-box cubes these meshes replace
REPLACE = ["GB_Desk_4471", "GB_Monitor_4471", "GB_FileCabinet_A", "GB_FileCabinet_B", "GB_BreakCounter",
           "GB_CoffeeMachine", "GB_WaterCooler", "GB_WellnessBed_1", "GB_WellnessBed_2", "GB_WellnessBed_3",
           "GB_Mug_4471", "GB_Keyboard_4471", "GB_Chair_4471", "GB_ReamOfPaper_A", "GB_ReamOfPaper_B",
           "GB_CoffeePot", "GB_WetFloorSign", "GB_FireExtinguisher", "GB_MopBucket"]
REPLACE_PREFIXES = ["GB_Cubicle_", "GB_HospBed_"]

P = []  # placements: (mesh, x, y, z, yaw, extra tags)


def put(mesh, x, y, z=0.0, yaw=0.0, tags=()):
    P.append((mesh, x, y, z, yaw, list(tags)))


# --- Your desk #4471 ----------------------------------------------------------
put("SM_Desk", 1300, 2300, 0, 180)
put("SM_Monitor", 1300, 2265, DESK_TOP, 180)
put("SM_Keyboard", 1300, 2325, DESK_TOP, 180)
put("SM_Mug", 1235, 2310, DESK_TOP, 0, ["ChangeOnRewind"])
put("SM_DeskPhone", 1390, 2290, DESK_TOP, 160)
put("SM_PenCup", 1200, 2270, DESK_TOP)
put("SM_DeskLamp", 1380, 2250, DESK_TOP, 200)
put("SM_PlantDesk", 1220, 2250, DESK_TOP)
put("SM_Stapler", 1355, 2330, DESK_TOP, 75)
put("SM_PCTower", 1410, 2280, 0, 180)
put("SM_OfficeChair", 1300, 2385, 0, 0, ["ChangeOnRewind"])
put("SM_TrashBin", 1170, 2370)
put("SM_DirectorsChair", 1480, 2470, 0, 210, ["Act2Reveal"])  # hidden in game until Act 2
put("SM_FilingCabinet_4", 1700, 3500, 0, 180)
put("SM_FilingCabinet_2", 1760, 3500, 0, 180)
put("SM_Printer", 3300, 1870, 0, -90)
put("SM_PlantTall", 860, 1780)
put("SM_PlantTall", 3330, 3620)
put("SM_WallClock", 1100, 1712, 240, 180)

# --- 12 cubicles (seeded so the layout is stable; the RUN seed varies clues, not furniture)
rnd = random.Random(4471)
SUPPLIES = ["SM_Binder", "SM_BinderMint", "SM_BinderRed", "SM_Stapler", "SM_TapeDispenser", "SM_PenCup",
            "SM_PlantDesk", "SM_ReamOfPaper", "SM_Laptop", "SM_Mug", "SM_DeskPhone"]
for row in range(3):
    for col in range(4):
        cx, cy = 1800 + col * 350, 2100 + row * 450
        put("SM_CubiclePanel", cx, cy - 150, 0, 0)
        put("SM_CubiclePanel", cx - 150, cy, 0, 90)
        put("SM_Desk", cx, cy - 95, 0, 180)
        put("SM_Monitor", cx, cy - 125, DESK_TOP, 180)
        put("SM_Keyboard", cx, cy - 75, DESK_TOP, 180)
        put("SM_OfficeChair", cx + rnd.uniform(-20, 20), cy - 5, 0, rnd.uniform(-25, 25))
        for _ in range(rnd.randint(2, 4)):
            put(rnd.choice(SUPPLIES), cx + rnd.uniform(-70, 70), cy - 95 + rnd.uniform(-25, 20), DESK_TOP,
                rnd.uniform(0, 360))
        if rnd.random() < 0.3:
            put("SM_TrashBin", cx + 60, cy + 30)
# coworkers standing a little too still
for i, (cx, cy) in enumerate([(1800, 2100), (2500, 2100), (2150, 2550), (2850, 3000), (1800, 3000)]):
    put("SM_Mannequin", cx + 70, cy + 70, 0, rnd.uniform(140, 220), ["NPC", f"Coworker_{i}"])

# --- Lobby: bright, calm, too clean --------------------------------------------
put("SM_HalcyonSign", 2788, 1300, 220, -90)
put("SM_LobbySofa", 1530, 1430, 0, 90)
put("SM_CoffeeTable", 1760, 1430)
put("SM_PlantTall", 1480, 580)
put("SM_PlantTall", 2720, 580)
put("SM_PlantTall", 2720, 1620)
put("SM_PlanterBox", 2700, 1000, 0, 90)
put("SM_WallClock", 1700, 512, 240, 180)
put("SM_PassReader", 1900, 512, 120, 180)
put("SM_Mannequin", 2100, 1220, 0, 180, ["NPC", "Receptionist"])
put("SM_MannequinGuard", 2350, 650, 0, 200, ["NPC", "Guard_Lobby"])
put("SM_ExitSign", 2100, 1688, 270, 0)

# --- Kitchen (the old break room) ------------------------------------------------
put("SM_KitchenCounter", 3300, 532, 0, 180)
put("SM_CoffeeMachine", 3140, 530, 90, 180)
put("SM_Microwave", 3400, 525, 90, 180)
put("SM_Kettle", 3500, 545, 90, 180)
put("SM_Mug", 3200, 560, 90)
put("SM_Mug", 3230, 575, 90)
put("SM_Fridge", 3710, 545, 0, 180)
put("SM_VendingMachine", 3750, 900, 0, -90)
put("SM_WaterCooler", 3730, 1250, 0, -90)
for tx, ty in ((3100, 1150), (3450, 1250)):
    put("SM_CafeTable", tx, ty)
    put("SM_BarStool", tx - 55, ty + 30)
    put("SM_BarStool", tx + 55, ty - 30)
put("SM_WetFloorSign", 3550, 1200, 0, 30)
put("SM_TrashBin", 2850, 1450)
put("SM_PlantTall", 2870, 580)
put("SM_WallClock", 3300, 1488, 240, 0)
put("SM_Mannequin", 3300, 1000, 0, 0, ["NPC", "Coworker_Kitchen"])

# --- Conference room ------------------------------------------------------------
for x in (600, 800, 1000):
    put("SM_MeetingChair", x, 960, 0, 180)
    put("SM_MeetingChair", x, 1240, 0, 0)
put("SM_Laptop", 700, 1100, 75, 180)
put("SM_Mug", 950, 1080, 75)
put("SM_PlantTall", 280, 580)
put("SM_PlantTall", 1320, 1620)
put("SM_WallClock", 800, 512, 240, 180)

# --- Supply closet ------------------------------------------------------------------
for i in range(7):
    put(["SM_Binder", "SM_BinderMint", "SM_BinderRed"][i % 3], 2880 + i * 6.5, 4170, 200, 90)
for i in range(4):
    put("SM_ReamOfPaper", 3100, 4170, 200 + i * 5, rnd.uniform(-8, 8))
put("SM_PassReader", 2950, 3688, 120, 0)
put("SM_MannequinGuard", 2950, 3590, 0, 0, ["NPC", "Guard_Closet"])

# --- Corridor B (before the loop section, so the loop stays symmetric) ---------------
put("SM_FireExtinguisher", 1230, 3780)
put("SM_MopBucket", 1550, 3800)
put("SM_MannequinCustodian", 1480, 3880, 0, 0, ["NPC", "Custodian"])
put("SM_PassReader", 1560, 5688, 120, 0)

# --- Wellness Center + Hospital ward ----------------------------------------------------
for x in (1100, 1400, 1700):
    put("SM_HospitalBed", x, 6100)
put("SM_PlantTall", 2300, 5800)
put("SM_WallClock", 1600, 5712, 240, 180)
for r in range(3):
    for c in range(5):
        put("SM_HospitalBed", 800 + c * 500, 5800 + r * 650, -1500)


# --- A few chairs turned the wrong way on purpose (about 1 in 7; never your own desk chair)
wrong = random.Random(214)
for i, (mesh, x, y, z, yaw, tags) in enumerate(P):
    if mesh in ("SM_OfficeChair", "SM_MeetingChair") and "ChangeOnRewind" not in tags and wrong.random() < 0.15:
        P[i] = (mesh, x, y, z, yaw + 180 + wrong.uniform(-25, 25), tags + ["WrongWay"])

# ----------------------------------------------------------------------------------
actor_sys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
asset_lib = unreal.EditorAssetLibrary


def main():
    removed = 0
    for a in actor_sys.get_all_level_actors():
        label = a.get_actor_label()
        if label.startswith(PREFIX) or label in REPLACE or any(label.startswith(p) for p in REPLACE_PREFIXES):
            actor_sys.destroy_actor(a)
            removed += 1

    cache, missing, placed = {}, set(), 0
    for n, (mesh, x, y, z, yaw, tags) in enumerate(P):
        if mesh not in cache:
            path = f"{MESHES}/{mesh}"
            cache[mesh] = asset_lib.load_asset(path) if asset_lib.does_asset_exist(path) else None
        sm = cache[mesh]
        if sm is None:
            missing.add(mesh)
            continue
        a = actor_sys.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x, y, z),
                                             unreal.Rotator(0, 0, yaw + YAW_FIX))
        comp = a.static_mesh_component
        comp.set_static_mesh(sm)
        if SCALE_FIX != 1.0:
            a.set_actor_scale3d(unreal.Vector(SCALE_FIX, SCALE_FIX, SCALE_FIX))
        a.set_actor_label(f"{PREFIX}{mesh[3:]}_{n:03d}")
        room = "Hospital" if z <= -1000 else "Campus"
        a.set_folder_path(f"Dressing/{room}/{'NPC' if 'NPC' in tags else 'Props'}")
        all_tags = list(tags)
        if mesh in THROWABLE:
            comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            comp.set_simulate_physics(True)
            comp.set_notify_rigid_body_collision(True)
            all_tags.append("Throwable")
        if "Act2Reveal" in tags:
            a.set_actor_hidden_in_game(True)
        if all_tags:
            a.set_editor_property("tags", [unreal.Name(t) for t in all_tags])
        placed += 1

    # Textures onto the grey-box boards. Posters 0-2 repeat as 3-5 so the loop looks identical.
    boards = {"GB_Poster_0": "MI_Poster_Energy", "GB_Poster_1": "MI_Poster_Present", "GB_Poster_2": "MI_Poster_CorridorB",
              "GB_Poster_3": "MI_Poster_Energy", "GB_Poster_4": "MI_Poster_Present", "GB_Poster_5": "MI_Poster_CorridorB",
              "GB_ChairmanPortrait": "MI_ChairmanPortrait", "GB_BulletinBoard": "MI_CallSheet"}
    for a in actor_sys.get_all_level_actors():
        mi_name = boards.get(a.get_actor_label())
        if mi_name and asset_lib.does_asset_exist(f"/Game/Props/Materials/{mi_name}"):
            a.static_mesh_component.set_material(0, asset_lib.load_asset(f"/Game/Props/Materials/{mi_name}"))

    unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
    unreal.log(f"[TAKE 4471] Dressed campus: placed {placed}, removed {removed} old actors")
    if missing:
        unreal.log_warning(f"[TAKE 4471] Missing meshes (run import_jam_assets.py?): {sorted(missing)}")


main()
