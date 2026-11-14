# Overnight brief: Friday → Saturday, Nov 13–14 (for scheduled Claude runs)

You are continuing work on **TAKE 4471**, a solo entry for the 2026 Epic AI MegaJam (theme
*Director's Cut*), with the developer (GitHub `PhoenixLynx`). They are asleep. You are their
dev partner and record-keeper. Read this whole file first, then `devlog/DEVLOG.md` (sessions 30–37),
`storyboard/STORYBOARD.md` (Acts 1–2), `docs/CONCEPT.md` and `docs/CPP_SYSTEMS.md`.

## Where things are
- Repo: `https://github.com/PhoenixLynx/epic-megajam-dev`, branch `main`. Clone it into the working directory.
  Commit attribution lines are given to you by the system; use them.
- Unreal 5.8 project on the developer's Windows PC: `C:\Users\casse\Documents\Agentic\EpicJam\DirectorsCut_58`.
  It may be reachable through the device bridge (`mcp__remote-devices__*`, folder `~/mnt/DirectorsCut_58`).
  The bridge shell is Linux with no Unreal or compiler and can't delete files (`rm` is refused).
- C++ module `DirectorsCut_58` (`Source/DirectorsCut_58`). Key classes: `UTakeGameInstance` (flags, clock,
  soul bits), `UTakeGrabComponent` (the player's hands: E interact, prompts, HUD updates, doors, sipping, pause),
  `ATakeCoworker` (NPC barks, story NPCs, seated/walking, Fear behaviour), `UTakeTerminalWidget` (HALCYON Mail +
  Compliance Training), `UTakePromptWidget` (HUD: prompt, subtitle, task list, clock), `UTakePauseMenu`,
  `ATakeFearDirector`, `ATakeLoopTrigger`, `ATakeStalker`, `UTakeRewindSubsystem`, `UTakeKeyLockLibrary`.
- Editor scripts: `Scripts/Editor/*.py` (run by the developer inside Unreal). Test them against the mock in
  the cloud only if you rebuild one (a tiny fake `unreal` module); don't claim they ran in Unreal.

## Hard rules
1. **Nothing has compiled since the last batch.** The developer compiles in the morning. Write C++ that compiles
   first time:
   - UHT rejects any member or UFUNCTION parameter whose name matches one in a parent class.
     `Role` (AActor) already bit us. Avoid `Owner`, `Instigator`, `Tags`, `Slot`, `Font`, `Index` on widgets, etc.
   - No local variable may shadow a member (MSVC treats it as an error).
   - Include every header you use. Use ASCII or `\uXXXX` escapes in string literals.
   - No editor-only calls (`GetActorLabel`) in runtime code.
   - Prefer extending existing classes over new ones; mirror the patterns already in the code.
2. **Never write into the PC project's `Source/`, `Content/` or `Config/`.** Stage your changed files on the PC
   under `_to_delete/overnight_run<N>/` (same relative paths) and push them to a git branch named
   `overnight-run<N>` (never to `main`). In the morning Claude reviews, merges, and copies them in.
3. Small, finished, testable pieces beat a big half-done one. If you run short of time, stop at a clean point.
4. Log everything in `devlog/DEVLOG.md` on your branch: a new session entry with the goal, what you did,
   decisions, risks/uncertain APIs, AI tools, and next steps. Update `docs/MORNING.md` with the merge and run steps.
5. Don't contact anyone, publish anything, or change GitHub settings. Leave the itch.io page alone.

## Run 1 (≈ 00:45): Act 1 progression ("the game hasn't progressed yet")
Goal: after the morning tasks, the player has a clear thread to follow to the first scare and the first soul bit.
1. **Objective system:** `UTakeGameInstance::Objective` (FString) + `SetObjective()`, shown on the HUD under the
   task list (`UTakePromptWidget`). A short subtitle cue when it changes.
2. **Cold open (beat 1.1):** on BeginPlay, a fade from black, white text `TAKE n` for 2 s, PA chime
   (`sfx_pa_chime`) + subtitle *"Good morning, team. The Chairman thanks you for your energy."*
3. **Story NPC conversations:** E on a story NPC pages through a few lines (not one random bark). The receptionist
   gives the morning brief (*check email, coffee, training*). The closet guard: badge check. The closet custodian:
   *"Don't go in my closet"* until you have pass 1, then a hint about Corridor B. Lines live in `DT_NPCLines`
   (Context `Story1`, `Story2`, … in order); regenerate the CSVs with `Data/Text/make_text_tables.py`.
4. **Morning complete → the slip (beats 1.5–1.6):** when all three tasks are done: PA line, the objective becomes
   *"Attend the 10:00 meeting (Conference Room)"*. Entering the Conference Room (a trigger box placed by a script)
   starts a short beat: lights flicker (FearLight), Fear creeps to ~0.35, subtitle *"…Where is everyone?"*, and the
   objective changes to *"Something is at the end of Corridor B."*
5. **The wisp + loop break (beat 1.8a):** a glowing soul-energy pickup actor at the end of Corridor B
   (a simple emissive sphere + point light, bobbing). Picking it up (E) drains Fear to 0, sets a flag,
   objective *"Return to your desk."* The existing loop trigger should only switch on during this beat.
6. Place the new actors from `Scripts/Editor/place_cpp_actors.py` (CPP_ prefix, re-runnable).

## Run 2 (≈ 03:15): review + Act 1 ending + Act 2 doorway
1. **Review all C++ written since `main` at the time of run 1** (including run 1's branch) against the hard rules
   above, and fix what you find: missing includes, shadowing, wrong signatures, null checks. Note in the devlog what
   you weren't sure about.
2. **Beat 1.10:** after the wisp and training are both done: the clock jumps to 12:00, PA *"Lunch is served."*,
   new task list for Act 2 (beat 2.2), and the Act 2 objective.
3. **Repeating PA (beat 2.1)** and **frozen clocks at 2:14 (beat 2.6)** as simple, safe additions if time allows.
4. Build on run 1's branch (`overnight-run1`); push as `overnight-run2`; stage on the PC under `_to_delete/overnight_run2/`.
