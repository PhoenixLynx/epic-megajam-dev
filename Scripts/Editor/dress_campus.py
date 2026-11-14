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
THROWABLE = {"SM_CoffeeBag", "SM_TeaBox", "SM_CupStack", "SM_SnackBag", "SM_SnackBox", "SM_SodaCan",
             "SM_WaterBottle", "SM_MilkCarton", "SM_LunchBox", "SM_FruitBowl", "SM_Mug", "SM_Keyboard", "SM_OfficeChair", "SM_ReamOfPaper", "SM_FireExtinguisher", "SM_WetFloorSign",
             "SM_MopBucket", "SM_Binder", "SM_BinderMint", "SM_BinderRed", "SM_Stapler", "SM_TapeDispenser",
             "SM_PenCup", "SM_DeskLamp", "SM_DeskPhone", "SM_Laptop", "SM_PCTower", "SM_TrashBin", "SM_PlantDesk",
             "SM_Kettle", "SM_Microwave", "SM_Monitor", "SM_MeetingChair", "SM_BarStool", "SM_CoffeeMachine"}

# Grey-box cubes these meshes replace
REPLACE = ["GB_ReceptionDesk", "GB_Desk_4471", "GB_Monitor_4471", "GB_FileCabinet_A", "GB_FileCabinet_B", "GB_BreakCounter",
           "GB_CoffeeMachine", "GB_WaterCooler", "GB_WellnessBed_1", "GB_WellnessBed_2", "GB_WellnessBed_3",
           "GB_Mug_4471", "GB_Keyboard_4471", "GB_Chair_4471", "GB_ReamOfPaper_A", "GB_ReamOfPaper_B",
           "GB_CoffeePot", "GB_WetFloorSign", "GB_FireExtinguisher", "GB_MopBucket"]
REPLACE_PREFIXES = ["GB_Cubicle_", "GB_HospBed_"]

P = []  # placements: (mesh, x, y, z, yaw, extra tags)


def put(mesh, x, y, z=0.0, yaw=0.0, tags=()):
    P.append((mesh, x, y, z, yaw, list(tags)))


# --- Your desk #4471 ----------------------------------------------------------
put("SM_Desk", 1300, 2300, 0, 180, ["Terminal", "Desk4471"])
put("SM_Monitor", 1300, 2272, DESK_TOP, 180, ["Terminal", "Monitor4471"])   # E here = sit and log in
# Desk top is x 1220-1380, y 2260-2340: everything stays inside it
put("SM_Keyboard", 1300, 2318, DESK_TOP, 180)
put("SM_Mug", 1242, 2320, DESK_TOP, 0, ["ChangeOnRewind"])
put("SM_DeskPhone", 1355, 2312, DESK_TOP, 160, ["Phone4471"])   # your phone: it rings
put("SM_PenCup", 1238, 2290, DESK_TOP)
put("SM_DeskLamp", 1360, 2275, DESK_TOP, 200)
put("SM_PlantDesk", 1250, 2270, DESK_TOP)
put("SM_Stapler", 1330, 2332, DESK_TOP, 75)
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
            put(rnd.choice(SUPPLIES), cx + rnd.uniform(-60, 60), cy - 95 + rnd.uniform(-20, 20), DESK_TOP,
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
put("SM_PlanterBox", 2700, 1450, 0, 90)   # clear of the kitchen doorway (y 900-1100)
put("SM_WallClock", 1700, 512, 240, 180)
put("SM_PassReader", 1900, 512, 120, 180)
put("SM_ReceptionDesk", 2100, 1300, 0, 0)
put("SM_Mannequin", 2100, 1400, 0, 0, ["NPC", "Receptionist"])   # behind the desk, facing the elevator
put("SM_MannequinGuard", 2350, 650, 0, 200, ["NPC", "Guard_Lobby"])
put("SM_ExitSign", 2100, 1688, 270, 0)

# --- Kitchen (the old break room) ------------------------------------------------
# North wall run, west to east, no gaps: short 3175-3295 | main (sink) 3295-3535 | short 3535-3655 | fridge
put("SM_KitchenCounterShort", 3235, 532, 0, 180)
put("SM_KitchenCounter", 3415, 532, 0, 180)
put("SM_KitchenCounterShort", 3595, 532, 0, 180)
put("SM_CoffeeMachine", 3215, 530, 91, 180, ["CoffeeMachine"])   # E = pour a coffee (task)
put("SM_Mug", 3262, 540, 91)
put("SM_Mug", 3280, 548, 91)
put("SM_Microwave", 3385, 525, 91, 180)
put("SM_Kettle", 3515, 545, 91, 180)          # sink is 3440-3490
put("SM_KitchenIsland", 3300, 830, 0, 180)         # more worktop; ~2 m aisle to the counter
put("SM_FridgeBody", 3710, 545, 0, 180)
FRIDGE = (3710, 545, 180)   # door, notes and food are placed relative to this below
put("SM_VendingMachine", 3750, 900, 0, -90)
put("SM_WaterCooler", 3730, 1250, 0, -90)
for tx, ty in ((3100, 1150), (3450, 1250)):
    put("SM_CafeTable", tx, ty)
    put("SM_BarStool", tx - 55, ty + 30)
    put("SM_BarStool", tx + 55, ty - 30)
put("SM_WetFloorSign", 3550, 1200, 0, 30)
put("SM_TrashBin", 2850, 1450)
put("SM_PlantTall", 2900, 580)
put("SM_WallClock", 3300, 1488, 240, 0)
put("SM_Mannequin", 3300, 1000, 0, 0, ["NPC", "Coworker_Kitchen"])
# Stock the counters (all tops at 90 cm): short 3175-3295, main 3295-3535 (sink 3440-3490),
# short 3535-3655, island 3207-3393 x 782-878. Props spawn 1 cm above and settle.
put("SM_CupStack", 3318, 535, 91)
put("SM_CupStack", 3340, 548, 91)
put("SM_TeaBox", 3560, 530, 91, 10)
put("SM_TeaBox", 3585, 530, 91, -5)
put("SM_TeaBox", 3572, 530, 99, 3)
put("SM_FruitBowl", 3630, 540, 91)
for i, x in enumerate((3225, 3250, 3275)):
    put("SM_CoffeeBag", x, 805, 91, rnd.uniform(-15, 15))
put("SM_SnackBag", 3320, 800, 91, 20)
put("SM_SnackBag", 3345, 820, 91, -10)
put("SM_SnackBag", 3330, 855, 91, 75)
put("SM_SnackBox", 3375, 805, 91, 0)
put("SM_SnackBox", 3375, 855, 91, 15)
put("SM_SodaCan", 3240, 860, 91)
put("SM_SodaCan", 3260, 862, 91)
put("SM_WaterBottle", 3290, 860, 91)
put("SM_SodaCan", 3100, 1150, 103)        # on the cafe tables
put("SM_WaterBottle", 3470, 1240, 103)
put("SM_SnackBag", 3430, 1270, 103, 60)

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


# --- Janitor closet: SW corner of the open office, door in its east wall --------------
#     walls/lintel are engine cubes (BOXES), the door is SM_Door on its hinge (Openable)
BOXES = []   # (label, x, y, z_centre, sx, sy, sz, material path, tags)
WALL_MAT = "/Game/Fear/MI_Greybox_Walls"
BOXES += [("JanitorWall_N", 950, 3350, 150, 320, 20, 300, WALL_MAT, []),
          ("JanitorWall_E1", 1100, 3415, 150, 20, 130, 300, WALL_MAT, []),
          ("JanitorWall_E2", 1100, 3635, 150, 20, 130, 300, WALL_MAT, []),
          ("JanitorLintel", 1100, 3525, 255, 20, 90, 90, WALL_MAT, [])]
put("SM_DoorFrame", 1100, 3525, 0, -90)
put("SM_Door", 1100, 3480, 0, -90, ["Openable", "OpenYaw=90", "JanitorDoor", "NeedsPass=1"])     # hinge at y 3480, swings in
put("SM_UtilityShelf", 835, 3525, 0, -90)
put("SM_MopBucket", 1000, 3420)
put("SM_WetFloorSign", 1040, 3640, 0, 15)
put("SM_TrashBin", 870, 3660)
for i in range(3):
    put(["SM_ReamOfPaper", "SM_SnackBox", "SM_TeaBox"][i], 835, 3470 + i * 40, 61)   # odd things stored on the shelf
put("SM_MannequinCustodian", 960, 3560, 0, 90, ["NPC", "Custodian_Closet"])

# --- Boards (engine cube + texture material): logo, notes ---------------------------
PROPMAT = "/Game/Props/Materials/"
BOXES += [("Logo_ReceptionFront", 2100, 1261, 62, 250, 1, 62, PROPMAT + "MI_HalcyonLogoPanel", []),
          ("Logo_LobbyWall", 1412, 800, 190, 1, 110, 110, PROPMAT + "MI_HalcyonLogo", []),
          ("Logo_Kitchen", 3300, 1488, 200, 90, 1, 90, PROPMAT + "MI_HalcyonLogo", [])]


def local_to_world(px, py, yaw, lx, ly):
    """Offset (lx, ly) in a prop's own frame (as Unreal sees it) -> world, for a prop at yaw (+YAW_FIX)."""
    import math
    a = math.radians(yaw + YAW_FIX)
    return px + lx * math.cos(a) - ly * math.sin(a), py + lx * math.sin(a) + ly * math.cos(a)


# Fridge door on its hinge (front-left corner), notes stuck on it, food inside
fx, fy, fyaw = FRIDGE
dx, dy = local_to_world(fx, fy, fyaw, -35, 35)
put("SM_FridgeDoor", dx, dy, 0, fyaw, ["Openable", "OpenYaw=100", "FridgeDoor"])
nx, ny = local_to_world(fx, fy, fyaw, -12, 41)
mx, my = local_to_world(fx, fy, fyaw, 14, 41)
BOXES += [("FridgeNote_Yogurt", nx, ny, 140, 20, 0.5, 20, PROPMAT + "MI_FridgeNote_Yogurt", ["AttachToFridgeDoor"]),
          ("FridgeNote_Memo", mx, my, 112, 18, 0.5, 24, PROPMAT + "MI_FridgeNote_Memo", ["AttachToFridgeDoor"])]
for (lx, ly, z, mesh) in ((-14, 5, 46, "SM_MilkCarton"), (10, 5, 46, "SM_SodaCan"), (16, -5, 46, "SM_SodaCan"),
                          (-8, 2, 86, "SM_LunchBox"), (12, 0, 126, "SM_WaterBottle"), (-14, 0, 126, "SM_SnackBox")):
    ix, iy = local_to_world(fx, fy, fyaw, lx, ly)
    put(mesh, ix, iy, z, fyaw)

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
        if mesh in THROWABLE and "Terminal" not in tags and "CoffeeMachine" not in tags:
            comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            comp.set_simulate_physics(True)
            comp.set_notify_rigid_body_collision(True)
            all_tags.append("Throwable")
        if "Openable" in tags:
            comp.set_mobility(unreal.ComponentMobility.MOVABLE)   # swung open by the interact system
        if "Act2Reveal" in tags:
            a.set_actor_hidden_in_game(True)
        if all_tags:
            a.set_editor_property("tags", [unreal.Name(t) for t in all_tags])
        placed += 1

    cube = asset_lib.load_asset("/Engine/BasicShapes/Cube.Cube")
    fridge_door = None
    for a in actor_sys.get_all_level_actors():
        if a.get_actor_label().startswith(PREFIX) and "FridgeDoor" in [str(t) for t in a.tags]:
            fridge_door = a
    for k, (label, x, y, z, sx, sy, sz, mat_path, tags) in enumerate(BOXES):
        b = actor_sys.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator(0, 0, 0))
        b.static_mesh_component.set_static_mesh(cube)
        b.set_actor_scale3d(unreal.Vector(sx / 100.0, sy / 100.0, sz / 100.0))
        if asset_lib.does_asset_exist(mat_path):
            b.static_mesh_component.set_material(0, asset_lib.load_asset(mat_path))
        b.set_actor_label(f"{PREFIX}{label}_{k:02d}")
        b.set_folder_path("Dressing/Campus/Boards")
        if label.startswith("FridgeNote"):
            b.set_editor_property("tags", [unreal.Name("Inspectable")])   # E: look closer
        if "AttachToFridgeDoor" in tags and fridge_door:
            b.static_mesh_component.set_mobility(unreal.ComponentMobility.MOVABLE)
            b.attach_to_actor(fridge_door, "", unreal.AttachmentRule.KEEP_WORLD, unreal.AttachmentRule.KEEP_WORLD,
                              unreal.AttachmentRule.KEEP_WORLD, False)

    # Textures onto the grey-box boards. Posters 0-2 repeat as 3-5 so the loop looks identical.
    boards = {"GB_Poster_0": "MI_Poster_Energy", "GB_Poster_1": "MI_Poster_Present", "GB_Poster_2": "MI_Poster_CorridorB",
              "GB_Poster_3": "MI_Poster_Energy", "GB_Poster_4": "MI_Poster_Present", "GB_Poster_5": "MI_Poster_CorridorB",
              "GB_ChairmanPortrait": "MI_ChairmanPortrait", "GB_BulletinBoard": "MI_CallSheet"}
    for a in actor_sys.get_all_level_actors():
        mi_name = boards.get(a.get_actor_label())
        if mi_name and asset_lib.does_asset_exist(f"/Game/Props/Materials/{mi_name}"):
            a.static_mesh_component.set_material(0, asset_lib.load_asset(f"/Game/Props/Materials/{mi_name}"))
            tags = [t for t in a.tags if str(t) != "Inspectable"] + [unreal.Name("Inspectable")]   # E: look closer
            a.set_editor_property("tags", tags)

    unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
    unreal.log(f"[TAKE 4471] Dressed campus: placed {placed}, removed {removed} old actors")
    if missing:
        unreal.log_warning(f"[TAKE 4471] Missing meshes (run import_jam_assets.py?): {sorted(missing)}")


main()
