"""
extend_corridor_b.py: TAKE 4471 / Director's Cut. Make Corridor B extra long (playtest: "let's make this hallway extra long").

Run in the Unreal Editor with the campus level open: Tools > Execute Python Script... > this file.

    DRY_RUN = True  (the default): changes NOTHING. It only prints, in the Output Log, every actor it would move,
                    stretch or add (filter the log on "[TAKE 4471] CorridorB"). Read that list first.
    DRY_RUN = False: does it, inside one undo step (Ctrl+Z puts everything back). It does NOT save the level:
                     walk it in PIE first, then Ctrl+S. Then Build > Build Paths (the stalker's navmesh).

What it does (EXTRA cm, default 1500: the corridor goes from 20 m to 35 m):
  1. MOVE south by EXTRA, as one block, everything on the ground floor past the corridor's south end
     (x 700-2500, y >= 5640, z > -700): the Wellness Center (floor, walls, door + its PassBlock=2 blocker, beds,
     the trust-fall X and its circle of mannequins, clock, sign, pass reader, ceiling, lights, ambience) and the
     Act 1 stalker at the end of the corridor (+ its GB_Stalker_Spawn_Act1 marker).
     The ward below (z -1500) does NOT move: the trust fall teleports you there (UTakeGrabComponent::EnterWard),
     it never relied on the floors lining up, and every ward position in C++ stays right.
  2. STRETCH the pieces that run along the corridor and end at its south end (both side walls, the floor, the
     ceiling): their north end stays put, their south end follows the Wellness Center.
  3. ADD to the new stretch: ceiling LED strips + corridor RectLights every 250 cm (copies of the corridor's own,
     FearLight tags and all) and five posters (copies of GB_Poster_0..2, WITHOUT the LoopPoster tag).
  4. (MOVE_WISP_TO_END) the Act 1 wisp + its GB_SoulEnergy_Act1 marker move by EXTRA too, so "something at the end
     of Corridor B" is still at the end.

What it leaves alone, on purpose:
  - The loop: GB_Poster_0..5 (y 4150-5450, 1000 cm period), the LoopTrigger (y 4950, offset -1000) and the
    CorridorB beat trigger (y 3780) don't move. The new stretch is south of the trigger, so you only walk it
    once the loop is broken; while looping, the fog the Fear Director brings in hides the far end (it's further
    away now, from both sides of the jump, which only helps).
  - Nothing in C++ needs to change: the ground-floor Wellness pieces are all found by tag (PassBlock=2,
    Use=TrustFall, Circle_n, Act3Reveal), not by position.

Caveats (why this is a script you run, not something done for you):
  - Re-running build_greybox_campus.py, dress_campus.py or add_ceilings.py rebuilds their pieces at the OLD
    positions. Run this again afterwards (it refuses to run twice on the same level: it leaves a marker,
    GB_CorridorB_Extended; delete that marker only if those scripts have put the corridor back to 20 m).
  - place_cpp_actors.py reads the GB_ markers this script moves, so re-running it stays consistent.
"""

import unreal

DRY_RUN = True
EXTRA = 1500.0              # cm added to the corridor
MOVE_WISP_TO_END = True

CUT_Y = 5640.0              # the corridor's south end is y 5700 (its wall is 20 cm thick); the loop posters end at 5450
BLOCK_X = (700.0, 2500.0)   # the Wellness Center is x 800-2400
GROUND_Z = -700.0           # above this = the ground floor (the ward is at -1500)
CORRIDOR_X = (1150.0, 1650.0)
LIGHT_STEP = 250.0          # add_ceilings.py: Corridor B rows are 2000 / 8
MARKER = "GB_CorridorB_Extended"
NEW_POSTERS = [("GB_Poster_0", 1212.0, 6000.0), ("GB_Poster_2", 1588.0, 6250.0), ("GB_Poster_1", 1212.0, 6550.0),
               ("GB_Poster_0", 1588.0, 6850.0), ("GB_Poster_1", 1212.0, 7050.0)]   # final positions, for EXTRA = 1500 (new stretch y 5700-7200)

# Things in the block that must never be moved or stretched (huge / world-wide / systems)
SKIP_CLASSES = ("WorldSettings", "NavMeshBoundsVolume", "RecastNavMesh", "PostProcessVolume", "DirectionalLight",
                "SkyLight", "SkyAtmosphere", "ExponentialHeightFog", "VolumetricCloud", "AbstractNavData",
                "Brush", "LevelBounds", "GameplayDebuggerPlayerManager", "PlayerStart", "TakeFearDirector",
                "TakeLoopTrigger", "TakeBeatTrigger")

actor_sys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
log = lambda msg: unreal.log(f"[TAKE 4471] CorridorB: {msg}")


def bounds(a):
    origin, extent = a.get_actor_bounds(False)
    return origin, extent


def is_cube(a):
    if not isinstance(a, unreal.StaticMeshActor):
        return False
    sm = a.static_mesh_component.static_mesh
    return bool(sm) and sm.get_name() == "Cube"


def main():
    actors = actor_sys.get_all_level_actors()
    by_label = {a.get_actor_label(): a for a in actors}
    if MARKER in by_label:
        unreal.log_warning(f"[TAKE 4471] CorridorB: already extended ({MARKER} is in the level). Nothing done.")
        return
    if "GB_Floor_CorridorB" not in by_label:
        unreal.log_warning("[TAKE 4471] CorridorB: GB_Floor_CorridorB not found: is the campus level open? Nothing done.")
        return

    move, stretch = [], []
    for a in actors:
        cls = a.get_class().get_name()
        if any(cls.startswith(s) for s in SKIP_CLASSES):
            continue
        loc = a.get_actor_location()
        if loc.z <= GROUND_Z:
            continue
        origin, ext = bounds(a)
        if ext.x > 1500.0 or ext.y > 2500.0:   # something huge (sky, ground plane, a volume): never ours to move
            if not (is_cube(a) and CORRIDOR_X[0] <= loc.x <= CORRIDOR_X[1]):
                continue
        # Runs along the corridor and ends at its south end: stretch
        if (is_cube(a) and CORRIDOR_X[0] <= loc.x <= CORRIDOR_X[1] and origin.y - ext.y < CUT_Y - 100.0
                and CUT_Y < origin.y + ext.y < CUT_Y + 200.0):
            yaw = abs(a.get_actor_rotation().yaw) % 180.0
            if min(yaw, 180.0 - yaw) > 1.0:
                unreal.log_warning(f"[TAKE 4471] CorridorB: {a.get_actor_label()} is turned ({yaw:.0f} deg); not stretched")
                continue
            stretch.append(a)
            continue
        if BLOCK_X[0] <= loc.x <= BLOCK_X[1] and loc.y >= CUT_Y:
            move.append(a)
            continue
        if MOVE_WISP_TO_END and (cls.startswith("TakeWisp") or a.get_actor_label() == "GB_SoulEnergy_Act1"):
            move.append(a)

    # The corridor's own light strip pattern (the southernmost row is copied along the new stretch)
    panels = [a for a in actors if a.get_actor_label().startswith("CL_Panel_CorridorB")]
    lights = [a for a in actors if a.get_actor_label().startswith("CL_Light_CorridorB")]
    last_panel = max(panels, key=lambda a: a.get_actor_location().y) if panels else None
    last_light = max(lights, key=lambda a: a.get_actor_location().y) if lights else None

    log(f"{'DRY RUN, nothing changed' if DRY_RUN else 'extending'}: +{EXTRA:.0f} cm")
    for a in stretch:
        o, e = bounds(a)
        log(f"  stretch {a.get_actor_label()}  y {o.y - e.y:.0f}..{o.y + e.y:.0f} -> ..{o.y + e.y + EXTRA:.0f}")
    for a in move:
        l = a.get_actor_location()
        log(f"  move    {a.get_actor_label()} ({a.get_class().get_name()})  y {l.y:.0f} -> {l.y + EXTRA:.0f}")
    rows = []
    if last_panel:
        y = last_panel.get_actor_location().y + LIGHT_STEP
        while y < 5700.0 + EXTRA - LIGHT_STEP * 0.5 + 1.0:
            rows.append(y)
            y += LIGHT_STEP
    log(f"  add     {len(rows)} light row(s) (panel {'found' if last_panel else 'MISSING'}, light {'found' if last_light else 'MISSING'})"
        f" and {sum(1 for p in NEW_POSTERS if p[0] in by_label)} poster(s)")
    if DRY_RUN:
        log("set DRY_RUN = False at the top of the script and run it again to do it")
        return

    with unreal.ScopedEditorTransaction("Extend Corridor B") as _t:
        for a in move:
            a.modify()
            a.set_actor_location(a.get_actor_location() + unreal.Vector(0.0, EXTRA, 0.0), False, False)
        for a in stretch:
            a.modify()
            s = a.get_actor_scale3d()
            o, e = bounds(a)
            size_y = e.y * 2.0
            factor = (size_y + EXTRA) / size_y
            a.set_actor_scale3d(unreal.Vector(s.x, s.y * factor, s.z))
            a.set_actor_location(a.get_actor_location() + unreal.Vector(0.0, EXTRA * 0.5, 0.0), False, False)
        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
        added = 0
        for i, y in enumerate(rows):
            for src in (last_panel, last_light):
                if not src:
                    continue
                d = actor_sys.duplicate_actor(src, world, unreal.Vector(0.0, y - src.get_actor_location().y, 0.0))
                if d:
                    d.set_actor_label(f"CL_Ext{src.get_actor_label().split('_')[1]}_CorridorB_{i:02d}")
                    added += 1
        for i, (src_label, x, y) in enumerate(NEW_POSTERS):
            src = by_label.get(src_label)
            if not src or y > 5700.0 + EXTRA - 100.0:   # (a smaller EXTRA: skip the ones past the new end)
                continue
            sl = src.get_actor_location()
            d = actor_sys.duplicate_actor(src, world, unreal.Vector(x - sl.x, y - sl.y, 0.0))
            if not d:
                continue
            # Not a loop poster (the loop trigger collects everything tagged LoopPoster); still Inspectable
            d.set_editor_property("tags", [t for t in d.tags if str(t) != "LoopPoster"])
            if (x > 1400.0) != (sl.x > 1400.0):   # other wall: face the other way
                r = d.get_actor_rotation()
                d.set_actor_rotation(unreal.Rotator(r.roll, r.pitch, r.yaw + 180.0), False)
            d.set_actor_label(f"GB_PosterExt_{i}")
            added += 1
        m = actor_sys.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(1400.0, 5700.0 + EXTRA, 50.0))
        m.set_actor_label(MARKER)
        m.set_folder_path("Markers")
        m.set_editor_property("tags", [unreal.Name(f"Extra={EXTRA:.0f}")])
    log(f"done: moved {len(move)}, stretched {len(stretch)}, added {added}. Walk it in PIE, then Ctrl+S and Build > Build Paths.")


main()
