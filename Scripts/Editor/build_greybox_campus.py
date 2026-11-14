"""
build_greybox_campus.py: TAKE 4471 / Director's Cut (2026 Epic AI MegaJam)

Builds the grey-box campus from storyboard/STORYBOARD.md in a new level:
    /Game/Maps/L_Campus_Greybox

Run it in the Unreal Editor (UE 5.8):
    Tools > Execute Python Script... > pick this file
  (needs the "Python Editor Script Plugin" enabled: Edit > Plugins)

Safe to re-run: it reopens the level and replaces everything it made before
(every actor it spawns is labelled with the GB_ prefix).

Units are centimetres (Unreal's default). Top-down layout:
  +X = east (right on the storyboard map), +Y = south (down on the map).
"""

import os

import unreal

# --------------------------------------------------------------------------
# Settings
# --------------------------------------------------------------------------
LEVEL_PATH = "/Game/Maps/L_Campus_Greybox"
PREFIX = "GB_"
WALL_HEIGHT = 300.0       # cm
WALL_THICK = 20.0         # cm
FLOOR_THICK = 10.0        # cm
GRID = 50.0               # wall layout resolution, cm
BUILD_CEILINGS = False    # keep off while grey-boxing so the top view stays readable
CUBE = "/Engine/BasicShapes/Cube.Cube"  # 100 x 100 x 100 cm

# --------------------------------------------------------------------------
# Layout: rooms are rectangles (x, y, width, depth). Doors are gaps cut into
# a room's side: (side, centre offset along that side from the room's
# corner, gap width). A door on a shared wall opens it for both rooms.
# --------------------------------------------------------------------------
ROOMS = [
    # name               x      y      w      d     doors
    ("Elevator",        1800,     0,   600,   500, [("S", 300, 300)]),
    ("Lobby",           1400,   500,  1400,  1200, [("W", 600, 200), ("E", 500, 200), ("S", 700, 400)]),
    ("ConferenceRoom",   200,   500,  1200,  1200, []),
    ("BreakRoom",       2800,   500,  1000,  1000, []),
    ("OpenOffice",       800,  1700,  2600,  2000, [("S", 600, 300), ("S", 2250, 150)]),
    ("CorridorB",       1200,  3700,   400,  2000, [("S", 200, 300)]),
    ("SupplyCloset",    2800,  3700,   500,   500, []),
    ("WellnessCenter",   800,  5700,  1600,  1400, []),
]

# Props: (label, x, y, size_x, size_y, size_z, yaw). x, y = centre on the floor.
PROPS = [
    # Open office: your desk, a row of cubicles, bulletin board
    ("Desk_4471",           1300, 2300, 160,  80,  75, 0),
    ("Monitor_4471",        1300, 2280,  60,  10,  40, 0),
    ("BulletinBoard",        820, 2700,  10, 200, 120, 0),
    ("FileCabinet_A",       1700, 3500,  50,  60, 130, 0),
    ("FileCabinet_B",       1760, 3500,  50,  60, 130, 0),
    # Lobby: reception desk, Chairman portrait, elevator button panel
    ("ReceptionDesk",       2100, 1300, 400,  80, 110, 0),
    ("ChairmanPortrait",    2550,  520, 180,  10, 240, 0),
    ("ElevatorPanel",       1900,  510,  30,  10,  60, 0),
    # Conference room: table + video-call screen
    ("ConferenceTable",      800, 1100, 600, 200,  75, 0),
    ("CallScreen",           220, 1100,  10, 400, 225, 0),
    # Break room: counter, coffee machine, water cooler
    ("BreakCounter",        3300,  540, 600,  60,  90, 0),
    ("CoffeeMachine",       3200,  540,  40,  40,  50, 0),
    ("WaterCooler",         3700, 1100,  40,  40, 130, 0),
    # Supply closet: shelving + Key Lock 2 panel
    ("SupplyShelf",         3050, 4170, 400,  40, 200, 0),
    ("KeyLock2_Panel",      2820, 3950,  10,  60,  80, 0),
    # Wellness Center (becomes the Hospital in Act 3)
    ("WellnessBed_1",       1100, 6100,  90, 200,  60, 0),
    ("WellnessBed_2",       1400, 6100,  90, 200,  60, 0),
    ("WellnessBed_3",       1700, 6100,  90, 200,  60, 0),
    ("WellnessDesk",        1600, 6900, 300,  80, 110, 0),
]
# Cubicle grid in the open office (partitions as low walls)
for row in range(3):
    for col in range(4):
        cx = 1800 + col * 350
        cy = 2100 + row * 450
        PROPS.append((f"Cubicle_{row}{col}_Back", cx, cy - 150, 300, 8, 140, 0))
        PROPS.append((f"Cubicle_{row}{col}_Side", cx - 150, cy, 8, 300, 140, 0))
        PROPS.append((f"Cubicle_{row}{col}_Desk", cx, cy - 100, 160, 70, 75, 0))

# Markers for systems we'll build next (Target Points: invisible in game)
MARKERS = [
    ("CorridorB_LoopStart",  1400, 3800),
    ("CorridorB_LoopEnd",    1400, 5600),
    ("SoulEnergy_Act1",      1400, 5400),
    ("KeyLock1_Desk",        1300, 2300),
    ("Stalker_Spawn_Act1",   1400, 5650),
]

PLAYER_START = (1300, 2550, 0)  # behind desk #4471, facing north (the monitor)

# Throwables: physics props you can grab and throw (label, x, y, z_bottom, sx, sy, sz, extra tags)
THROWABLES = [
    ("Mug_4471",          1250, 2290, 75,  10, 10, 12, ["ChangeOnRewind"]),  # on your desk
    ("Keyboard_4471",     1310, 2320, 75,  45, 15,  3, []),
    ("Chair_4471",        1300, 2380,  0,  50, 50, 90, ["ChangeOnRewind"]),
    ("ReamOfPaper_A",     2150, 2010, 75,  30, 21,  5, []),
    ("ReamOfPaper_B",     2500, 2460, 75,  30, 21,  5, []),
    ("CoffeePot",         3260,  540, 90,  15, 15, 20, []),
    ("WetFloorSign",      3550, 1200,  0,  30, 25, 60, []),
    ("FireExtinguisher",  1230, 3780,  0,  15, 15, 50, []),
    ("MopBucket",         1550, 3800,  0,  40, 30, 35, []),
]

# Corridor B loop. Posters repeat every LOOP_PERIOD cm so the stretch you're
# teleported into looks identical; one poster gets changed per loop (in BP).
LOOP_PERIOD = 1000
LOOP_TRIGGER_Y = 4950          # walk past this...
POSTERS = [                    # (label, x on wall, y, wall side)
    ("Poster_0", 1212, 4150), ("Poster_1", 1212, 4450), ("Poster_2", 1588, 4300),
    ("Poster_3", 1212, 5150), ("Poster_4", 1212, 5450), ("Poster_5", 1588, 5300),
]

# Act 1 blocker: keeps the Wellness Center shut (opens with Pass 2) and stops
# you seeing past the loop.
DOORS = [("Door_Wellness", 1400, 5700, 300, 20, 300)]

# Act 3: the Hospital sits directly BELOW the Wellness Center. Trust fall ->
# hide the Wellness floor -> you drop into the ward.
BUILD_HOSPITAL = True
HOSPITAL_Z = -1500.0
HOSPITAL_ROOMS = [("HospitalWard", 400, 5300, 2800, 2400, [])]
HOSPITAL_PROPS = []
for r in range(3):
    for c in range(5):
        bx, by = 800 + c * 500, 5800 + r * 650
        HOSPITAL_PROPS.append((f"HospBed_{r}{c}", bx, by, 90, 200, 60, 0))
        HOSPITAL_PROPS.append((f"HospCurtain_{r}{c}", bx + 160, by, 4, 220, 200, 0))
        HOSPITAL_PROPS.append((f"HospChart_{r}{c}", bx, by + 120, 30, 5, 110, 0))   # chart stand = script page
HOSPITAL_MARKERS = [("Hospital_Landing", 1400, 6400), ("Hospital_REC_Light", 3150, 5400),
                    ("Hospital_Exit", 3000, 7500), ("KeyLock3_Terminal", 2200, 7550)]

# --------------------------------------------------------------------------
# Helpers
# --------------------------------------------------------------------------
actor_sys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
level_sys = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
cube_mesh = unreal.EditorAssetLibrary.load_asset(CUBE)


def open_or_create_level():
    if unreal.EditorAssetLibrary.does_asset_exist(LEVEL_PATH):
        level_sys.load_level(LEVEL_PATH)
        for a in actor_sys.get_all_level_actors():
            if a.get_actor_label().startswith(PREFIX):
                actor_sys.destroy_actor(a)
    else:
        level_sys.new_level(LEVEL_PATH)


def spawn_box(label, cx, cy, cz, sx, sy, sz, folder, yaw=0.0):
    """Box centred at (cx, cy, cz) with size (sx, sy, sz) in cm."""
    actor = actor_sys.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(cx, cy, cz), unreal.Rotator(0, 0, yaw))
    actor.static_mesh_component.set_static_mesh(cube_mesh)
    actor.set_actor_scale3d(unreal.Vector(sx / 100.0, sy / 100.0, sz / 100.0))
    actor.set_actor_label(PREFIX + label)
    actor.set_folder_path(f"Greybox/{folder}")
    return actor


def spawn_simple(cls, label, x, y, z, folder, yaw=0.0):
    actor = actor_sys.spawn_actor_from_class(cls, unreal.Vector(x, y, z), unreal.Rotator(0, 0, yaw))
    actor.set_actor_label(PREFIX + label)
    actor.set_folder_path(f"Greybox/{folder}")
    return actor


def make_throwable(actor, extra_tags=()):
    """Movable + physics + hit events + 'Throwable' tag (see docs/BLUEPRINT_RECIPES.md #2)."""
    comp = actor.static_mesh_component
    comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    comp.set_simulate_physics(True)
    comp.set_notify_rigid_body_collision(True)
    actor.set_editor_property("tags", [unreal.Name("Throwable")] + [unreal.Name(t) for t in extra_tags])


def build_rooms(rooms, z0, folder_prefix):
    """Floors (+ optional ceilings) and merged walls for a set of rooms at height z0."""
    for name, x, y, w, d, _ in rooms:
        spawn_box(f"Floor_{name}", x + w / 2, y + d / 2, z0 - FLOOR_THICK / 2,
                  w, d, FLOOR_THICK, folder_prefix + "Floors")
        if BUILD_CEILINGS:
            spawn_box(f"Ceiling_{name}", x + w / 2, y + d / 2, z0 + WALL_HEIGHT + FLOOR_THICK / 2,
                      w, d, FLOOR_THICK, folder_prefix + "Ceilings")
    tag = folder_prefix.strip("/") or "Campus"
    for n, (orient, line, s0, e0) in enumerate(wall_segments(rooms)):
        length = e0 - s0
        if orient == "H":
            spawn_box(f"Wall_{tag}_{n:03d}", s0 + length / 2, line, z0 + WALL_HEIGHT / 2,
                      length + WALL_THICK, WALL_THICK, WALL_HEIGHT, folder_prefix + "Walls")
        else:
            spawn_box(f"Wall_{tag}_{n:03d}", line, s0 + length / 2, z0 + WALL_HEIGHT / 2,
                      WALL_THICK, length + WALL_THICK, WALL_HEIGHT, folder_prefix + "Walls")


def wall_segments(rooms):
    """Merge every room's sides onto a shared grid so shared walls are built
    once, and a door on either room opens the wall for both.
    Returns a list of (orientation, line, start, end): orientation 'H' runs
    along X at y=line, 'V' runs along Y at x=line."""
    cells = {}  # (orient, line, index) -> "wall" | "door"

    def mark(orient, line, start, end, kind):
        i0, i1 = int(round(start / GRID)), int(round(end / GRID))
        for i in range(i0, i1):
            key = (orient, line, i)
            if kind == "door" or key not in cells:
                cells[key] = kind

    for name, x, y, w, d, doors in rooms:
        sides = {"N": ("H", y, x, x + w), "S": ("H", y + d, x, x + w),
                 "W": ("V", x, y, y + d), "E": ("V", x + w, y, y + d)}
        for orient, line, s, e in sides.values():
            mark(orient, line, s, e, "wall")
        for side, offset, width in doors:
            orient, line, s, _ = sides[side]
            centre = s + offset
            mark(orient, line, centre - width / 2, centre + width / 2, "door")

    segments = []
    for (orient, line) in sorted({(k[0], k[1]) for k in cells}):
        idx = sorted(i for (o, l, i), kind in cells.items()
                     if o == orient and l == line and kind == "wall")
        run_start = prev = None
        for i in idx + [None]:
            if run_start is None:
                run_start = prev = i
            elif i is not None and i == prev + 1:
                prev = i
            else:
                segments.append((orient, line, run_start * GRID, (prev + 1) * GRID))
                run_start = prev = i
    return segments


# --------------------------------------------------------------------------
# Build
# --------------------------------------------------------------------------
def build():
    open_or_create_level()

    with unreal.ScopedSlowTask(6, "Building grey-box campus...") as task:
        task.make_dialog(True)

        task.enter_progress_frame(1, "Floors and walls")
        build_rooms(ROOMS, 0.0, "")


        task.enter_progress_frame(1, "Props")
        for label, x, y, sx, sy, sz, yaw in PROPS:
            spawn_box(label, x, y, sz / 2, sx, sy, sz, "Props", yaw)
        for label, x, y, z0, sx, sy, sz, tags in THROWABLES:
            a = spawn_box(label, x, y, z0 + sz / 2, sx, sy, sz, "Throwables")
            make_throwable(a, tags)
        for label, x, y, sx, sy, sz in DOORS:
            spawn_box(label, x, y, sz / 2, sx, sy, sz, "Doors")

        task.enter_progress_frame(1, "Corridor B loop")
        for label, x, y in POSTERS:
            poster = spawn_box(label, x, y, 160, 4, 60, 90, "CorridorB_Loop")
            poster.set_editor_property("tags", [unreal.Name("LoopPoster")])   # found by ATakeLoopTrigger
        trig = spawn_simple(unreal.TriggerBox, "LoopTrigger", 1400, LOOP_TRIGGER_Y, 150, "CorridorB_Loop")
        trig.set_actor_scale3d(unreal.Vector(380 / 64.0, 40 / 64.0, 300 / 64.0))  # TriggerBox is 64 cm by default

        task.enter_progress_frame(1, "Hospital (below the Wellness Center)")
        if BUILD_HOSPITAL:
            build_rooms(HOSPITAL_ROOMS, HOSPITAL_Z, "Hospital/")
            for label, x, y, sx, sy, sz, yaw in HOSPITAL_PROPS:
                spawn_box(label, x, y, HOSPITAL_Z + sz / 2, sx, sy, sz, "Hospital/Props", yaw)
            for label, x, y in HOSPITAL_MARKERS:
                spawn_simple(unreal.TargetPoint, label, x, y, HOSPITAL_Z + 50, "Hospital/Markers")
            hl = spawn_simple(unreal.PointLight, "Light_HospitalWard", 1800, 6500,
                              HOSPITAL_Z + WALL_HEIGHT - 30, "Hospital/Lights")
            hl.point_light_component.set_intensity(1500.0)          # dim on purpose
            hl.point_light_component.set_light_color(unreal.LinearColor(0.75, 0.9, 1.0, 1.0))
            hl.point_light_component.set_attenuation_radius(2600.0)

        task.enter_progress_frame(1, "Lights, markers, player start")
        for name, x, y, w, d, _ in ROOMS:
            light = spawn_simple(unreal.PointLight, f"Light_{name}",
                                 x + w / 2, y + d / 2, WALL_HEIGHT - 30, "Lights")
            light.point_light_component.set_intensity(5000.0)
            light.point_light_component.set_attenuation_radius(max(w, d))
            light.set_editor_property("tags", [unreal.Name("FearLight")])   # dims/flickers with Fear
        for label, x, y in MARKERS:
            spawn_simple(unreal.TargetPoint, label, x, y, 50, "Markers")
        # Unbound post-process volume: the Fear effects will drive this one.
        ppv = spawn_simple(unreal.PostProcessVolume, "PPV_Fear", 2000, 3000, 150, "Systems")
        ppv.set_editor_property("unbound", True)
        nav = spawn_simple(unreal.NavMeshBoundsVolume, "NavMeshBounds", 2000, 3700,
                           (HOSPITAL_Z if BUILD_HOSPITAL else 0) / 2 + 150, "Systems")
        # Covers the whole campus (and the Hospital). Press P in the viewport to see the walkable area.
        nav.set_actor_scale3d(unreal.Vector(44, 76, (abs(HOSPITAL_Z) + 600) / 200 if BUILD_HOSPITAL else 4))
        px, py, pyaw = PLAYER_START
        ps = spawn_simple(unreal.PlayerStart, "PlayerStart_Desk4471", px, py, 100, "Systems", yaw=-90)
        ps.set_editor_property("player_start_tag", unreal.Name("Desk4471"))   # rewind returns here

    level_sys.save_current_level()
    unreal.log(f"[TAKE 4471] Grey-box campus built and saved to {LEVEL_PATH}")

    # Bright daylight (sun, sky, real-time sky light, fog): same as running add_daylight.py
    daylight = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()),
                            "Scripts", "Editor", "add_daylight.py")
    if os.path.exists(daylight):
        exec(compile(open(daylight, encoding="utf-8").read(), daylight, "exec"), {"__name__": "__main__"})


build()
