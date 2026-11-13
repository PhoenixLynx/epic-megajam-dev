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
    ("Chair_4471",          1300, 2380,  50,  50,  90, 0),
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

    with unreal.ScopedSlowTask(4, "Building grey-box campus...") as task:
        task.make_dialog(True)

        task.enter_progress_frame(1, "Floors")
        for name, x, y, w, d, _ in ROOMS:
            spawn_box(f"Floor_{name}", x + w / 2, y + d / 2, -FLOOR_THICK / 2,
                      w, d, FLOOR_THICK, "Floors")
            if BUILD_CEILINGS:
                spawn_box(f"Ceiling_{name}", x + w / 2, y + d / 2, WALL_HEIGHT + FLOOR_THICK / 2,
                          w, d, FLOOR_THICK, "Ceilings")

        task.enter_progress_frame(1, "Walls")
        for n, (orient, line, s, e) in enumerate(wall_segments(ROOMS)):
            length = e - s
            if orient == "H":
                spawn_box(f"Wall_{n:03d}", s + length / 2, line, WALL_HEIGHT / 2,
                          length + WALL_THICK, WALL_THICK, WALL_HEIGHT, "Walls")
            else:
                spawn_box(f"Wall_{n:03d}", line, s + length / 2, WALL_HEIGHT / 2,
                          WALL_THICK, length + WALL_THICK, WALL_HEIGHT, "Walls")

        task.enter_progress_frame(1, "Props")
        for label, x, y, sx, sy, sz, yaw in PROPS:
            spawn_box(label, x, y, sz / 2, sx, sy, sz, "Props", yaw)

        task.enter_progress_frame(1, "Lights, markers, player start")
        for name, x, y, w, d, _ in ROOMS:
            light = spawn_simple(unreal.PointLight, f"Light_{name}",
                                 x + w / 2, y + d / 2, WALL_HEIGHT - 30, "Lights")
            light.point_light_component.set_intensity(5000.0)
            light.point_light_component.set_attenuation_radius(max(w, d))
        spawn_simple(unreal.SkyLight, "SkyLight", 2000, 3000, 1000, "Lights")
        for label, x, y in MARKERS:
            spawn_simple(unreal.TargetPoint, label, x, y, 50, "Markers")
        # Unbound post-process volume: the Fear effects will drive this one.
        ppv = spawn_simple(unreal.PostProcessVolume, "PPV_Fear", 2000, 3000, 150, "Systems")
        ppv.set_editor_property("unbound", True)
        px, py, pyaw = PLAYER_START
        spawn_simple(unreal.PlayerStart, "PlayerStart_Desk4471", px, py, 100, "Systems", yaw=-90)

    level_sys.save_current_level()
    unreal.log(f"[TAKE 4471] Grey-box campus built and saved to {LEVEL_PATH}")


build()
