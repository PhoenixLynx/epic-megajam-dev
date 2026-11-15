# Submission Checklist: 2026 Epic AI MegaJam

**Deadline:** Sun Nov 15, 2026, 6:59 PM Phoenix time (MST, UTC-7)
**Target:** submit by 5:00 PM MST to leave a buffer for upload problems.

## Required

- [ ] Windows development build packaged (zipped, tested by launching it on a clean run)
  - Fri Nov 13, 21:47: first test package OK (`Builds/FridayNight`, 1.1 GB, Development, exit code 0). Still to do: the final build Sunday, zip it, test on a clean run.
- [ ] itch.io game page set to **public**
- [ ] Devlog linked from the itch page (GitHub repo starts **private**: switch it to **public** before linking, or post the devlog on itch instead)
- [ ] Only the Unreal Engine logo used (no other engine/brand logos in the game or on the page)
- [ ] Submitted to the jam before the deadline (official: Nov 15, 5:59 PM PST = 6:59 PM Phoenix)
- [ ] Any pre-jam material (prompts, scripts, assets) declared; any sourced assets listed with licenses
- [ ] Submitted by me (a human), not an agent
- [ ] Rated Teen or below; no third-party IP or trademarks other than the UE logo

## Glass Box category (dev log prize)

- [ ] Dev log submitted **with the entry, during the contest period** (this repo's DEVLOG.md, linked or posted as an itch devlog)

## Nice to have

- [ ] Cover image + screenshots on the itch page
- [ ] **REQUIRED:** gameplay video, 30-60 s, 1920x1080 or higher (rules sections 1 and 7(c))
- [ ] Controls listed on the page
- [ ] AI tool usage summarized on the page (pulled from the devlog ledger)
- [ ] "Tools you used" field filled in at submission (some partner prizes, e.g. Hyper3D Rodin's pool, depend on it)

## Status log

| When (MST) | Update |
|------------|--------|
| 2026-11-12 19:17 | Checklist created. Nothing ticked yet. |
| 2026-11-12 19:40 | Added rule items found while checking the asset rules: pre-jam declaration, human submits, rating/IP, Glass Box dev log. |

- Note (Sat 12:10): rebuild with `Rebuild.bat` (project folder) after C++ changes; package again after this batch compiles and at end of Saturday.

## Sunday Nov 15: the day's plan (from the 8:45 AM debrief)

| Time (MST) | Step |
|---|---|
| on waking | Open the editor (the 3:55 AM build is current), run `import_memorial.py` (9 photos picked from the playtest screenshots), playtest pass |
| by 12:00 | **Feature freeze.** Title background v02 approve/redo, last notes |
| 12:00-13:00 | **Package:** Platforms > Windows > Package Project (Development) into `DirectorsCut_58/Builds/Sunday`; launch the .exe on a clean run (title, morning, one ending) |
| 13:00-14:00 | **Gameplay video** 30-60 s, 1920x1080 (OBS): morning office, coffee splash, Corridor B loop, the ward, the party, the director's chair |
| 14:00-15:30 | **itch.io page:** cover, screenshots, controls, AI-tool summary, devlog link (make the repo public or post the devlog on itch), zip upload |
| by 17:00 | **Submit** (deadline 18:59) |

Cook settings checked 8:45 AM: `DirectoriesToAlwaysCook` now also has `/Game/Ending`, `/Game/Memorial` and `/Engine/BasicShapes` (all loaded by path at runtime; without them a packaged build would lose Sol's signs, the memorial and the party shapes). Backup: `Config/DefaultGame.ini.bak-2026-11-15`.

