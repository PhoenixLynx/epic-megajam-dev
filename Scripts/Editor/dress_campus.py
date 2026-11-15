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
import math
import random

import unreal

MESHES = "/Game/Props/Meshes"
PREFIX = "DR_"
YAW_FIX = 180.0   # confirmed on import: Blender fronts arrive facing backwards in Unreal
SCALE_FIX = 1.0
DESK_TOP = 76.0  # cm

# Throwable meshes get physics
THROWABLE = {"SM_Brochure", "SM_PaperBall", "SM_Sandwich", "SM_PizzaSlice", "SM_SaladBowl", "SM_Donut", "SM_CoffeeBag", "SM_TeaBox", "SM_CupStack", "SM_SnackBag", "SM_SnackBox", "SM_SodaCan",
             "SM_WaterBottle", "SM_MilkCarton", "SM_LunchBox", "SM_FruitBowl", "SM_Mug", "SM_Keyboard", "SM_OfficeChair", "SM_ReamOfPaper", "SM_FireExtinguisher", "SM_WetFloorSign",
             "SM_MopBucket", "SM_Binder", "SM_BinderMint", "SM_BinderRed", "SM_Stapler", "SM_TapeDispenser",
             "SM_PenCup", "SM_DeskLamp", "SM_DeskPhone", "SM_Laptop", "SM_PCTower", "SM_TrashBin", "SM_PlantDesk",
             "SM_Kettle", "SM_Microwave", "SM_Monitor", "SM_MeetingChair", "SM_BarStool", "SM_CoffeeMachine"}

# Grey-box cubes these meshes replace
REPLACE = ["GB_ReceptionDesk", "GB_Desk_4471", "GB_Monitor_4471", "GB_FileCabinet_A", "GB_FileCabinet_B", "GB_BreakCounter",
           "GB_CoffeeMachine", "GB_WaterCooler", "GB_WellnessBed_1", "GB_WellnessBed_2", "GB_WellnessBed_3",
           "GB_Mug_4471", "GB_Keyboard_4471", "GB_Chair_4471", "GB_ReamOfPaper_A", "GB_ReamOfPaper_B",
           "GB_CoffeePot", "GB_WetFloorSign", "GB_FireExtinguisher", "GB_MopBucket", "GB_SupplyShelf"]
REPLACE_PREFIXES = ["GB_Cubicle_", "GB_HospBed_"]

SLOT_MI = {"SM_WetFloorSign": {"M_SignWet": "MI_WetFloor", "M_SignWetBack": "MI_WetFloor"},
           "SM_ExitSign": {"M_ExitFace": "MI_ExitSign"}}
P = []  # placements: (mesh, x, y, z, yaw, extra tags)


def put(mesh, x, y, z=0.0, yaw=0.0, tags=()):
    P.append((mesh, x, y, z, yaw, list(tags)))


def cabinet(x, y, yaw, drawers=4, drawer_tags=None):
    """A filing cabinet you can open: shell + one sliding SM_FileDrawer per drawer (E opens / closes).
    drawer_tags: {drawer index (0 = bottom): [extra tags]}"""
    import math
    put(f"SM_FilingShell_{drawers}", x, y, 0, yaw)
    a = math.radians(yaw + YAW_FIX)
    lx, ly = 0.0, 30.0                       # drawer fronts sit 30 cm in front of the pivot (Unreal local +Y)
    dx, dy = x + lx * math.cos(a) - ly * math.sin(a), y + lx * math.sin(a) + ly * math.cos(a)
    for i in range(drawers):
        extra = (drawer_tags or {}).get(i, [])
        put("SM_FileDrawer", dx, dy, 33 * i + 16.5, yaw, ["Openable", "Slide=40", "FileDrawer"] + extra)


# --- Your desk #4471 ----------------------------------------------------------
put("SM_Desk", 1300, 2300, 0, 180, ["Terminal", "Desk4471"])
put("SM_Monitor", 1300, 2272, DESK_TOP, 180, ["Terminal", "Monitor4471"])   # E here = sit and log in
# Desk top is x 1220-1380, y 2260-2340: everything stays inside it
put("SM_Keyboard", 1300, 2318, DESK_TOP, 180, ["DeskItem", "DeskFixed"])
put("SM_Mug", 1242, 2320, DESK_TOP, 0, ["ChangeOnRewind", "DeskItem"])
put("SM_DeskPhone", 1355, 2312, DESK_TOP, 160, ["Phone4471", "DeskItem", "DeskFixed"])   # your phone: it rings
put("SM_PenCup", 1238, 2290, DESK_TOP, 0, ["DeskItem", "DeskFixed"])
put("SM_DeskLamp", 1360, 2275, DESK_TOP, 200, ["DeskItem", "DeskFixed"])
put("SM_PlantDesk", 1250, 2270, DESK_TOP, 0, ["DeskItem", "DeskFixed"])
put("SM_Stapler", 1330, 2332, DESK_TOP, 75, ["DeskItem", "DeskFixed"])
put("SM_PCTower", 1410, 2280, 0, 180)
put("SM_OfficeChair", 1300, 2385, 0, 0, ["ChangeOnRewind"])
put("SM_TrashBin", 1170, 2370)
for i in range(5):                                          # a pile of crumpled drafts by your bin after lunch: the paper toss
    put("SM_PaperBall", 1215 + i * 11, 2420 + (i % 2) * 9, 0, 0, ["Act2Reveal"])   # (only after lunch: a clean desk in the morning)
put("SM_DirectorsChair", 1480, 2470, 0, 210, ["Act2Reveal"])  # hidden in game until Act 2
cabinet(1700, 3500, 0, 4, {3: ["InspectTex=MI_File_4470"]})   # top drawer: 4470's personnel file
cabinet(1760, 3500, 0, 2)
put("SM_Printer", 3300, 2300, 0, -90, ["Use=Printer"])   # (moved out of the restroom corner) E = print
put("SM_PlantTall", 860, 1780)
put("SM_PlantTall", 3330, 3620)
put("SM_WallClock", 870, 1712, 240, 180)    # (moved along: it overlapped the office print)

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
        if (row + col) % 2 == 0:                            # paper to throw (paper toss)
            put("SM_PaperBall", cx - 40, cy - 70, DESK_TOP)
# coworkers standing a little too still
for i, (cx, cy) in enumerate([(1800, 2100), (2500, 2100), (2150, 2550), (2850, 3000), (1800, 3000)]):
    put("SM_Mannequin", cx + 70, cy + 70, 0, rnd.uniform(140, 220), ["NPC", f"Coworker_{i}"])

# --- Restroom: NE corner of the open office (x 3050-3400, y 1700-2120), door in its west wall ----------
#     The north and east walls are the office's own; west/south walls, lintel and stall are engine cubes (BOXES below).
put("SM_DoorFrame", 3050, 1995, 0, -90)
put("SM_Door", 3050, 1950, 0, -90, ["Openable", "OpenYaw=-90"])                      # swings into the restroom
put("SM_SignPlaque", 3038, 2075, 150, -90, ["SlotMI=M_SignFace:MI_RestroomSign"])   # on the office side, by the door
put("SM_Toilet", 3320, 1765, 0, 180, ["Use=Toilet"])
put("SM_StallDoor", 3290, 1880, 0, 180, ["Openable", "Locked", "Occupied"])         # hinge at x 3290, always occupied
put("SM_Shoes", 3320, 1845, 0, 180, ["FearReveal"])                                  # under the stall door, only in Fear
put("SM_Sink", 3365, 2010, 0, -90, ["Use=Sink"])
put("SM_Mirror", 3394, 2010, 120, -90, ["Inspectable", "InspectTex=MI_Mirror"])
put("SM_HandDryer", 3200, 2108, 110, 0, ["Use=Dryer"])
put("SM_TrashBin", 3120, 2085)
put("SM_PaperBall", 3150, 2060, 0)
put("SM_WetFloorSign", 3210, 1965, 0, 40)
put("SM_MeetingChair", 3275, 2065, 0, 0)       # the inexplicable restroom chair (moved out of the new stall)
# The second stall (playtest note): west of the first, a mirror of it, and this one opens. Nobody's in it. Probably.
put("SM_Toilet", 3160, 1765, 0, 180, ["Use=Toilet"])
put("SM_StallDoor", 3130, 1880, 0, 180, ["Openable", "OpenYaw=-90"])                  # hinge at x 3130, swings in
put("SM_PlantDesk", 3375, 2085, 85)

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
# A little more "inviting": two armchairs, plants, a magazine, a rug, a promo screen (BOXES below)
put("SM_MeetingChair", 1700, 1590, 0, 0)
put("SM_MeetingChair", 1830, 1590, 0, 0)
put("SM_PlantDesk", 1742, 1432, 44)
for i, (yaw, dz) in enumerate(((15, 0.0), (-8, 1.2), (28, 2.4))):   # a little stack on the table, clear of the plant (they slid off)
    put("SM_Brochure", 1780, 1436, 46 + dz, yaw, [f"SlotMI=M_SignFace:{'MI_Brochure' if i < 2 else 'MI_Magazine'}"])
put("SM_Monitor", 2160, 1292, 109, 180, ["FixedScreen", "SlotMI=M_Screen:MI_Screen_CCTV"])   # reception sees everything (faces the receptionist, in reach, on the desk)
put("SM_Keyboard", 2150, 1328, 109, 180, ["DeskFixed"])
put("SM_PlantDesk", 2280, 1300, 109)
put("SM_PlantTall", 1440, 1650)
put("SM_WaterCooler", 2740, 1260, 0, -90, ["Use=Cooler"])
# Security desk by the elevator: two monitors showing the camera feeds (one of them is you, from behind)
put("SM_Desk", 1640, 620, 0, 180)        # security booth, NW corner (clear of the Chairman's portrait)
put("SM_Monitor", 1605, 595, 75, 180, ["FixedScreen", "SlotMI=M_Screen:MI_Screen_CCTV"])
put("SM_Monitor", 1675, 595, 75, 180, ["FixedScreen", "SlotMI=M_Screen:MI_Screen_CCTV"])
put("SM_OfficeChair", 1640, 710, 0, 0)   # a guard sits here (place_cpp_actors.py)
# Security cameras (they turn to follow you; C++ 'SecurityCam')
for x, y, z, yaw in ((2785, 515, 265, -45), (1415, 515, 265, 45), (815, 1715, 265, 45), (3385, 3685, 265, -135),
                     (3785, 515, 265, -45), (1385, 3715, 265, 45), (215, 515, 265, 45)):
    put("SM_SecurityCam", x, y, z, yaw + 180, ["SecurityCam"])

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
put("SM_VendingMachine", 3750, 900, 0, -90, ["Vending"])          # E = a snack (always the same one)
put("SM_WaterCooler", 3730, 1250, 0, -90, ["Use=Cooler"])
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
put("SM_SnackBag", 3085, 1135, 104, 20)          # (snacks live on the cafe tables; the island is for lunch)
put("SM_SnackBag", 3110, 1170, 104, -10)
put("SM_SnackBox", 3470, 1235, 104, 0)
put("SM_SnackBox", 3430, 1262, 104, 15)
put("SM_SodaCan", 3240, 860, 91)
put("SM_SodaCan", 3260, 862, 91)
put("SM_WaterBottle", 3290, 860, 91)
put("SM_SodaCan", 3100, 1150, 103)        # on the cafe tables
put("SM_WaterBottle", 3470, 1240, 103)
put("SM_SnackBag", 3430, 1270, 103, 60)
put("SM_Mannequin", 3300, 690, 0, 180, ["NPC", "Chef", "Act2Reveal"])   # the lunch attendant, behind the island (chef's hat)
# Lunch is served (Act 2): on the kitchen island (top 90 cm), under a TODAY'S SPECIALS board. Hidden until 12:00.
for mesh, x, y in (("SM_PizzaSlice", 3305, 800), ("SM_PizzaSlice", 3308, 848), ("SM_Sandwich", 3345, 800),
                   ("SM_Sandwich", 3348, 848), ("SM_SaladBowl", 3378, 805), ("SM_Donut", 3378, 852)):
    put(mesh, x, y, 91, rnd.uniform(-20, 20), ["Act2Reveal"])

# --- Conference room ------------------------------------------------------------
for x in (600, 800, 1000):
    put("SM_MeetingChair", x, 960, 0, 180)
    put("SM_MeetingChair", x, 1240, 0, 0)
put("SM_DirectorsChair", 1150, 1100, 0, 90)          # at the head of the table, empty
TABLE_Z = 75
for (x, y, yaw) in ((600, 1030, 0), (800, 1030, 0), (1000, 1170, 180)):   # screens toward the chairs (playtest: they faced away)
    put("SM_Laptop", x, y, TABLE_Z, yaw, ["CallScreen"])          # E = join the 12:30 call / 1:00 meeting (Act 2)
# the conference phone in the middle of the table: E here joins the call too (playtest: "we should start the call with
# the conference room phone the way they do in corporate offices")
put("SM_DeskPhone", 850, 1100, TABLE_Z, 90, ["CallScreen", "DeskFixed"])   # bolted down
for (x, y) in ((660, 1060), (950, 1080), (720, 1150), (1060, 1130)):
    put("SM_Mug", x, y, TABLE_Z, rnd.uniform(0, 360))
for (x, y) in ((880, 1090), (900, 1110), (520, 1100)):
    put("SM_WaterBottle", x, y, TABLE_Z)
put("SM_DeskPhone", 800, 1100, TABLE_Z, 180)          # conference phone in the middle
put("SM_ReamOfPaper", 560, 1150, TABLE_Z, 10)
put("SM_BinderRed", 1080, 1060, TABLE_Z, 80)
put("SM_PenCup", 980, 1000, TABLE_Z)
put("SM_PlantTall", 280, 580)
put("SM_PlantTall", 1320, 1620)
put("SM_TrashBin", 1340, 600)
put("SM_WallClock", 800, 512, 240, 180)

# Supply closet door (office south wall, opening x 2975-3125): access pass 1, like the guard says
BOXES_SUPPLY = [("SupplyWall_L", 2990, 3700, 150, 30, 20, 300), ("SupplyWall_R", 3110, 3700, 150, 30, 20, 300),
                ("SupplyLintel", 3050, 3700, 255, 90, 20, 90)]
put("SM_DoorFrame", 3050, 3700, 0, 180)
put("SM_Door", 3005, 3700, 0, 180, ["Openable", "OpenYaw=90", "NeedsPass=1"])   # hinge at x 3005, swings into the closet
# --- Supply closet: three steel shelves along the back wall, stocked -----------------------
SHELF_Z = (16, 61, 106, 151)   # top of each shelf you can put things on (the 5th is the roof)
for sx in (2890, 3050, 3210):
    put("SM_UtilityShelf", sx, 4172, 0, 0)
STOCK = ["SM_Binder", "SM_BinderMint", "SM_BinderRed", "SM_ReamOfPaper", "SM_ReamOfPaper", "SM_TapeDispenser",
         "SM_Stapler", "SM_CupStack", "SM_CoffeeBag", "SM_TeaBox", "SM_PenCup", "SM_SnackBox"]
for sx in (2890, 3050, 3210):
    for z in SHELF_Z[1:]:
        for k in range(4):
            mesh = rnd.choice(STOCK)
            if mesh.startswith("SM_Binder"):   # binders stand in a row
                for b in range(3):
                    put(rnd.choice(["SM_Binder", "SM_BinderMint", "SM_BinderRed"]), sx - 45 + k * 28 + b * 6.5, 4175, z + 1, 90)
            else:
                put(mesh, sx - 45 + k * 28, 4175 + rnd.uniform(-6, 6), z + 1, rnd.uniform(-20, 20))
for i in range(4):   # boxes of paper on the bottom shelf
    put("SM_ReamOfPaper", 3050 + rnd.uniform(-40, 40), 4175, SHELF_Z[0] + 1 + i * 5, rnd.uniform(-8, 8))
cabinet(3270, 3900, -90, 4)
put("SM_Printer", 2850, 3820, 0, 90, ["Use=Printer"])
put("SM_TrashBin", 3260, 4060)
put("SM_WetFloorSign", 3180, 3780, 0, 30)
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

# --- Act 3 ---------------------------------------------------------------------------------
# Team building (3.1): six coworkers in a circle around a mint X in the Wellness Center (hidden until Act 3)
CIRCLE = (1100, 6650)
for i in range(6):
    a = math.radians(i * 60 + 30)
    x, y = CIRCLE[0] + 240 * math.cos(a), CIRCLE[1] + 240 * math.sin(a)
    face = math.degrees(math.atan2(-math.cos(a), math.sin(a)))   # put-yaw that faces the centre
    put("SM_Mannequin", x, y, 0, face, ["NPC", f"Circle_{i}", "Act3Reveal"])
# The ward (3.3-3.6): nurses' station terminal (Key Lock 3) and the service elevator (BOXES below)
put("SM_Desk", 2200, 7520, -1500, 180, ["KeyLockPanel=3"])
put("SM_Monitor", 2200, 7495, -1425, 180, ["KeyLockPanel=3", "FixedScreen", "SlotMI=M_Screen:MI_Screen_Login"])


# --- Janitor closet: SW corner of the open office, door in its east wall --------------
#     walls/lintel are engine cubes (BOXES), the door is SM_Door on its hinge (Openable)
BOXES = []   # (label, x, y, z_centre, sx, sy, sz, material path, tags)
PM = "/Game/Props/Materials/"
BOXES += [
    ("TrustFallMark", 1100, 6650, 0.6, 90, 90, 1, PM + "MI_TrustMark", ["Use=TrustFall"]),
    ("ServiceElevatorDoor", 3000, 7690, -1390, 130, 4, 220, PM + "M_BrushedSteel", ["Use=ServiceElevator"]),
    ("Sign_Service", 3000, 7686, -1240, 40, 2, 40, PM + "MI_Sign_Service", []),
]
BOXES += [
    # Lunch board on the island (Act 2)
    ("LunchSpecials", 3245, 860, 115, 34, 2, 48, PM + "MI_LunchSpecials", ["Act2Reveal", "Inspectable"]),
    # Reception: the sign-in clipboard, and a sign
    ("SignInSheet", 1985, 1285, 109.6, 23, 32, 0.4, PM + "MI_SignInSheet", ["Inspectable", "Yaw=180"]),   # reads from the visitor's side
    ("Sign_Wait", 2045, 1268, 124, 30, 2, 30, PM + "MI_Sign_Wait", []),   # standing on the desk
    ("ScriptNotes", 2120, 1290, 109.6, 22, 30, 0.4, PM + "MI_ScriptNotes", ["Inspectable"]),
    # Kitchen: a bulletin board (west wall) and morale posters
    ("KitchenBoard", 2812, 1300, 160, 1, 150, 100, PM + "MI_KitchenBoard", ["Inspectable"]),
    ("PartyFlier", 2813.5, 1345, 178, 1, 30, 40, "/Game/Ending/MI_PartyFlier", ["Act2Reveal", "Inspectable"]),   # pinned up after lunch
    # the blank north wall of the open office (behind your desk's row): a corporate print, framed
    ("OfficeArt_Frame", 1100, 1711.5, 175, 420, 3, 115, "/Game/Props/Meshes/M_MetalDark", []),
    ("OfficeArt", 1100, 1713.5, 175, 400, 1, 100, "/Game/Ending/MI_OfficeArt", ["Inspectable"]),
    # a window in the open office (playtest: "add a window in the office"): on the east wall, a perfect day outside, always
    ("OfficeWindow_Frame", 3388.5, 2950, 172, 3, 530, 170, "/Game/Props/Meshes/M_MetalLight", []),
    ("OfficeWindow_View", 3386.5, 2950, 172, 1, 500, 150, "/Game/Ending/MI_WindowView", []),
    ("OfficeWindow_Mullion1", 3385.5, 2825, 172, 2, 5, 150, "/Game/Props/Meshes/M_MetalLight", []),
    ("OfficeWindow_Mullion2", 3385.5, 2950, 172, 2, 5, 150, "/Game/Props/Meshes/M_MetalLight", []),
    ("OfficeWindow_Mullion3", 3385.5, 3075, 172, 2, 5, 150, "/Game/Props/Meshes/M_MetalLight", []),
    ("OfficeWindow_Sill", 3380, 2950, 95, 20, 550, 4, "/Game/Props/Meshes/M_MetalLight", []),
    # (playtest: "looks like artwork, not an outside setting") a deep casing and half-drawn blinds make it read as a window
    ("OfficeWindow_Head", 3382, 2950, 250, 16, 550, 6, "/Game/Props/Meshes/M_MetalLight", []),
    ("OfficeWindow_JambL", 3382, 2678, 172, 16, 6, 160, "/Game/Props/Meshes/M_MetalLight", []),
    ("OfficeWindow_JambR", 3382, 3222, 172, 16, 6, 160, "/Game/Props/Meshes/M_MetalLight", []),
    ("OfficeWindow_BlindRail", 3380, 2950, 244, 6, 520, 6, "/Game/Props/Meshes/M_PlasticWhite", []),
    ("OfficeWindow_Slat0", 3381, 2950, 238, 3, 500, 1.2, "/Game/Props/Meshes/M_PlasticWhite", []),
    ("OfficeWindow_Slat1", 3381, 2950, 233, 3, 500, 1.2, "/Game/Props/Meshes/M_PlasticWhite", []),
    ("OfficeWindow_Slat2", 3381, 2950, 228, 3, 500, 1.2, "/Game/Props/Meshes/M_PlasticWhite", []),
    ("OfficeWindow_Slat3", 3381, 2950, 223, 3, 500, 1.2, "/Game/Props/Meshes/M_PlasticWhite", []),
    ("OfficeWindow_Slat4", 3381, 2950, 218, 3, 500, 1.2, "/Game/Props/Meshes/M_PlasticWhite", []),
    ("OfficeWindow_Slat5", 3381, 2950, 213, 3, 500, 1.2, "/Game/Props/Meshes/M_PlasticWhite", []),
    ("OfficeWindow_Slat6", 3381, 2950, 208, 3, 500, 1.2, "/Game/Props/Meshes/M_PlasticWhite", []),
    ("OfficeWindow_Slat7", 3381, 2950, 203, 3, 500, 1.2, "/Game/Props/Meshes/M_PlasticWhite", []),
    ("Poster_Smile_Kitchen", 3600, 1488, 165, 60, 1, 85, PM + "MI_Poster_Smile", ["Inspectable"]),
    ("Poster_Energy_Kitchen", 2812, 700, 165, 1, 60, 85, PM + "MI_Poster_Energy", ["Inspectable"]),
    # Lobby: rug, magazine, promo screen on the west wall
    ("Lobby_Rug", 1760, 1450, 0.6, 360, 240, 1, PM + "MI_Rug", []),
    ("Magazine", 1720, 1415, 44.4, 21, 27, 0.6, PM + "MI_Magazine", ["Inspectable"]),
    ("Lobby_PromoScreen", 1412, 1450, 175, 3, 160, 90, PM + "MI_Screen_Promo", []),
    # Elevator: a button panel inside (E), emergency signs
    ("ElevatorPanel_In", 2385, 380, 120, 2, 24, 48, PM + "MI_ElevatorPanel", ["Use=Elevator"]),
    ("Sign_ElevatorFire", 1815, 380, 150, 2, 40, 40, PM + "MI_Sign_ElevatorFire", []),
    ("Sign_ElevatorFire_Out", 1935, 512, 150, 30, 2, 30, PM + "MI_Sign_ElevatorFire", []),
    ("Sign_Emergency", 2100, 15, 170, 40, 2, 40, PM + "MI_Sign_Emergency", []),
    # Elevator car: brushed steel inside (walls, floor plate, handrail), so it reads as an elevator and reflects
    ("Elevator_SteelW", 1812, 250, 150, 4, 480, 300, PM + "M_BrushedSteel", []),
    ("Elevator_SteelE", 2388, 250, 150, 4, 480, 300, PM + "M_BrushedSteel", []),
    ("Elevator_SteelN", 2100, 12, 150, 576, 4, 300, PM + "M_BrushedSteel", []),
    ("Elevator_Floor", 2100, 250, 0.6, 580, 480, 1.2, PM + "M_BrushedSteel", []),
    ("Elevator_Rail", 2100, 22, 95, 520, 4, 5, PM + "M_BrushedSteel", []),
    # Security booth front: a low counter
    ("SecurityBooth_Front", 1640, 775, 55, 220, 8, 110, "/Game/Fear/MI_Greybox_Walls", []),
    # Conference room whiteboard (east wall); the scribbles hide Key 2's squares
    ("Whiteboard", 1388, 1400, 160, 2, 220, 110, PM + "MI_Whiteboard", ["Inspectable"]),
]
BOXES += [(l, x, y, z, sx, sy, sz, "/Game/Fear/MI_Greybox_Walls", []) for (l, x, y, z, sx, sy, sz) in BOXES_SUPPLY]
BOXES += [("RestroomWall_W1", 3050, 1825, 150, 20, 250, 300, "/Game/Fear/MI_Greybox_Walls", []),
          ("RestroomWall_W2", 3050, 2080, 150, 20, 80, 300, "/Game/Fear/MI_Greybox_Walls", []),
          ("RestroomLintel", 3050, 1995, 255, 20, 90, 90, "/Game/Fear/MI_Greybox_Walls", []),
          ("RestroomWall_S", 3225, 2120, 150, 370, 20, 300, "/Game/Fear/MI_Greybox_Walls", []),
          ("StallWall_W", 3240, 1790, 100, 4, 180, 180, "/Game/Fear/MI_Greybox_Walls", []),
          ("StallFront_L", 3265, 1880, 100, 50, 4, 180, "/Game/Fear/MI_Greybox_Walls", []),
          ("StallFront_R", 3371, 1880, 100, 38, 4, 180, "/Game/Fear/MI_Greybox_Walls", []),
          # the second stall: its west wall, and the front either side of its door (3130-3192)
          ("Stall2Wall_W", 3080, 1790, 100, 4, 180, 180, "/Game/Fear/MI_Greybox_Walls", []),
          ("Stall2Front_L", 3105, 1880, 100, 50, 4, 180, "/Game/Fear/MI_Greybox_Walls", []),
          ("Stall2Front_R", 3216, 1880, 100, 48, 4, 180, "/Game/Fear/MI_Greybox_Walls", [])]
WALL_MAT = "/Game/Fear/MI_Greybox_Walls"
BOXES += [("JanitorWall_N", 950, 3350, 150, 320, 20, 300, WALL_MAT, []),
          ("JanitorWall_E1", 1100, 3415, 150, 20, 130, 300, WALL_MAT, []),
          ("JanitorWall_E2", 1100, 3635, 150, 20, 130, 300, WALL_MAT, []),
          ("JanitorLintel", 1100, 3525, 255, 20, 90, 90, WALL_MAT, [])]
put("SM_DoorFrame", 1100, 3525, 0, -90)
put("SM_Door", 1100, 3480, 0, -90, ["Openable", "OpenYaw=90", "JanitorDoor", "NeedsPass=1"])     # hinge at y 3480, swings in
put("SM_UtilityShelf", 835, 3525, 0, -90)
put("SM_MopBucket", 1050, 3395)
put("SM_WetFloorSign", 850, 3655, 0, 15)
put("SM_TrashBin", 950, 3672)
for i in range(3):
    put(["SM_ReamOfPaper", "SM_SnackBox", "SM_TeaBox"][i], 835, 3470 + i * 40, 61)   # odd things stored on the shelf
put("SM_MannequinCustodian", 1030, 3600, 0, 90, ["NPC", "Custodian_Closet"])   # inside, by the door (you only reach him with it open)
# 4470: the employee before you. Their things are boxed up in here.
put("SM_MovingBox", 905, 3410, 0, -90, ["Inspectable", "InspectTex=MI_Label_4470"])
BOXES += [("Label_4470", 928, 3410, 16, 1, 30, 16, "/Game/Props/Materials/MI_Label_4470", ["Inspectable"]),          # on the box, facing the door
          ("Polaroid_4470", 890, 3440, 31, 9, 11, 0.4, "/Game/Props/Materials/MI_Polaroid_4470", ["Inspectable"]),    # dropped on top
          ("Sticky_4470", 835, 3500, 107, 8, 8, 0.3, "/Game/Props/Materials/MI_Sticky_4470", ["Inspectable"])]       # on the shelf
# Your nameplate: the 4471 sticker doesn't quite cover the old number
BOXES += [("Nameplate_4471", 1300, 2338, DESK_TOP + 4, 20, 1, 6, "/Game/Props/Materials/MI_Nameplate_4471", ["Inspectable"])]

# --- Boards (engine cube + texture material): logo, notes ---------------------------
PROPMAT = "/Game/Props/Materials/"
POSTER = lambda name: PROPMAT + f"MI_Poster_{name}"
BOXES += [("Poster_Smile_Office", 812, 2150, 165, 1, 60, 85, POSTER("Smile"), ["Inspectable"]),          # west wall
          ("Poster_Mark_Office", 3388, 2600, 165, 1, 60, 85, POSTER("Mark"), ["Inspectable"]),           # east wall
          ("Poster_Wellness_Office", 2000, 3688, 165, 60, 1, 85, POSTER("Wellness"), ["Inspectable"]),   # south wall
          ("Poster_Smile_Conference", 1050, 512, 165, 60, 1, 85, POSTER("Smile"), ["Inspectable"]),      # north wall
          ("Poster_Energy_Lobby", 2788, 760, 165, 1, 60, 85, POSTER("Energy"), ["Inspectable"]),        # lobby east wall
          ("Poster_Present_Kitchen", 3788, 1300, 165, 1, 60, 85, POSTER("Present"), ["Inspectable"])]    # kitchen east wall
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
# (playtest: "top shelf of fridge should be accessible, it's blocked": the freezer lip covers the top shelf from
#  1.15 m up, so what was on it, water and snacks, now sits on the middle shelf and the bottom)
for (lx, ly, z, mesh) in ((-14, 5, 46, "SM_MilkCarton"), (10, 5, 46, "SM_SodaCan"), (16, -5, 46, "SM_SodaCan"),
                          (-8, 2, 86, "SM_LunchBox"), (16, 0, 86, "SM_WaterBottle"), (-6, 0, 5, "SM_SnackBox"),
                          (10, 0, 126, "SM_SodaCan"), (-12, 2, 126, "SM_MilkCarton")):   # the top shelf, reachable now (SM_FridgeBody re-exported with a short lip)
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
        all_tags = [t for t in tags if not t.startswith("SlotMI=")]
        # Printed faces: per-mesh defaults, plus SlotMI=<slot>:<MI> tags for one-off signs
        slot_mis = dict(SLOT_MI.get(mesh, {}))
        for t in tags:
            if t.startswith("SlotMI="):
                slot, mi = t[7:].split(":")
                slot_mis[slot] = mi
        for slot, mi in slot_mis.items():
            idx = comp.get_material_index(slot)
            path = f"/Game/Props/Materials/{mi}"
            if idx >= 0 and asset_lib.does_asset_exist(path):
                comp.set_material(idx, asset_lib.load_asset(path))
        if "DeskFixed" in tags:   # your desk stays tidy (Fear can still throw it around; it puts itself back)
            comp.set_mobility(unreal.ComponentMobility.MOVABLE)
        elif mesh in THROWABLE and "Terminal" not in tags and "CoffeeMachine" not in tags and "FixedScreen" not in tags \
                and not any(str(t).startswith("KeyLockPanel") for t in tags):   # lock terminals are bolted down (playtest: knocking the ward's away could make it unsolvable)
            comp.set_mobility(unreal.ComponentMobility.MOVABLE)
            comp.set_simulate_physics(True)
            comp.set_notify_rigid_body_collision(True)
            all_tags.append("Throwable")
        if "SecurityCam" in tags:
            comp.set_mobility(unreal.ComponentMobility.MOVABLE)   # turned by C++ to follow you
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
        for t in tags:   # "Yaw=180": turn the board (the sign-in sheet reads from the visitor's side)
            if str(t).startswith("Yaw="):
                b.set_actor_rotation(unreal.Rotator(0, 0, float(str(t)[4:])), False)
        if asset_lib.does_asset_exist(mat_path):
            b.static_mesh_component.set_material(0, asset_lib.load_asset(mat_path))
        b.set_actor_label(f"{PREFIX}{label}_{k:02d}")
        b.set_folder_path("Dressing/Campus/Boards")
        box_tags = [t for t in tags if t != "AttachToFridgeDoor" and not str(t).startswith("Yaw=")] + (["Inspectable"] if label.startswith("FridgeNote") else [])
        if box_tags:
            b.set_editor_property("tags", [unreal.Name(t) for t in sorted(set(box_tags))])   # Inspectable: E = look closer
        if "AttachToFridgeDoor" in tags and fridge_door:
            b.static_mesh_component.set_mobility(unreal.ComponentMobility.MOVABLE)
            b.attach_to_actor(fridge_door, "", unreal.AttachmentRule.KEEP_WORLD, unreal.AttachmentRule.KEEP_WORLD,
                              unreal.AttachmentRule.KEEP_WORLD, False)

    # Textures onto the grey-box boards. Posters 0-2 repeat as 3-5 so the loop looks identical.
    boards = {"GB_Poster_0": "MI_Poster_Energy", "GB_Poster_1": "MI_Poster_Present", "GB_Poster_2": "MI_Poster_CorridorB",
              "GB_Poster_3": "MI_Poster_Energy", "GB_Poster_4": "MI_Poster_Present", "GB_Poster_5": "MI_Poster_CorridorB",
              "GB_ChairmanPortrait": "MI_ChairmanPortrait", "GB_BulletinBoard": "MI_CallSheet",
              "GB_CallScreen": "MI_Screen_Call", "GB_KeyLock2_Panel": "MI_Screen_Login"}
    # Act 3 ward charts: two carry Key 3's squares, one is a decoy, the rest are blank
    for r in range(3):
        for c in range(5):
            boards[f"GB_HospChart_{r}{c}"] = {(0, 1): "MI_Chart_4471", (1, 3): "MI_Chart_Stage", (2, 0): "MI_Chart_4470"}.get((r, c), "MI_Chart_Blank")
    for a in actor_sys.get_all_level_actors():
        mi_name = boards.get(a.get_actor_label())
        if mi_name and asset_lib.does_asset_exist(f"/Game/Props/Materials/{mi_name}"):
            a.static_mesh_component.set_material(0, asset_lib.load_asset(f"/Game/Props/Materials/{mi_name}"))
            if not mi_name.startswith("MI_Screen"):
                tags = [t for t in a.tags if str(t) != "Inspectable"] + [unreal.Name("Inspectable")]   # E: look closer
                a.set_editor_property("tags", tags)
        extra = {"GB_CallScreen": "CallScreen", "GB_KeyLock2_Panel": "KeyLockPanel", "GB_Door_Wellness": "PassBlock=2"}.get(a.get_actor_label())
        if extra and unreal.Name(extra) not in a.tags:                         # E: join the call / log in (Key Lock 2)
            a.set_editor_property("tags", list(a.tags) + [unreal.Name(extra)])

    # The ward needs light: cold ceiling lights in rows, and one red REC light by a doorway
    lights = [(x, y, -1240, 5000.0, (0.78, 0.88, 1.0)) for x in (900, 1600, 2300, 3000) for y in (5700, 6450, 7200)]
    lights += [(3150, 5400, -1300, 900.0, (1.0, 0.1, 0.08))]
    for k, (x, y, z, inten, col) in enumerate(lights):
        lt = actor_sys.spawn_actor_from_class(unreal.PointLight, unreal.Vector(x, y, z), unreal.Rotator(0, 0, 0))
        lt.set_actor_label(f"{PREFIX}WardLight_{k:02d}")
        lt.set_folder_path("Dressing/Hospital/Lights")
        try:
            lc = lt.get_editor_property("light_component")
            lc.set_editor_property("intensity", inten)
            lc.set_editor_property("light_color", unreal.Color(r=int(col[0] * 255), g=int(col[1] * 255), b=int(col[2] * 255), a=255))
            lc.set_editor_property("attenuation_radius", 900.0)
        except Exception as e:   # (a light with default settings is still a light)
            unreal.log_warning(f"[TAKE 4471] Ward light settings: {e}")

    unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
    unreal.log(f"[TAKE 4471] Dressed campus: placed {placed}, removed {removed} old actors")
    if missing:
        unreal.log_warning(f"[TAKE 4471] Missing meshes (run import_jam_assets.py?): {sorted(missing)}")


main()
