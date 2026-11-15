# Submission Checklist: 2026 Epic AI MegaJam

**Deadline:** Sun Nov 15, 2026, 6:59 PM Phoenix time (MST, UTC-7)
**Target:** submit by 5:00 PM MST to leave a buffer for upload problems.

## Required

- [x] Windows development build packaged and tested (zip still to make)
  - Fri Nov 13, 21:47: first test package OK (`Builds/FridayNight`, 1.1 GB, Development, exit code 0). Still to do: the final build Sunday, zip it, test on a clean run.
  - Sun 14:45: feature freeze. All critical playtest fixes verified in the editor. Engine debug view-mode keys cleared in `Config/DefaultInput.ini` (F5 was shader complexity). Final package started.
  - Always-cook folders: `/Game/Ending`, `/Game/Memorial` and `/Engine/BasicShapes` were added at 8:45 AM. `/Game/FirstPerson` and `/Game/Input` are optional extras. See [PACKAGING.md](PACKAGING.md) for the steps and the .exe test.
- [x] Save/Continue checked in the .exe (Sun 15:12): checkpoints at the start of each act, so a save after noon continues at 12:00. Fine for the jam.
- [x] **Zip named `TeamName_GameName`** (Epic's final-day email), e.g. `PhoenixLynx_Take4471.zip`, uploaded to my itch account, then submitted to the jam
- [x] **Submission form:** team name, each member's name + Epic Games account email (I type this myself, it's not kept in this repo), game name, **30–60 s gameplay video LINK** (e.g. unlisted YouTube)
- [x] Optional fields: devlog link (Glass Box), "Tools you used" (Hyper3D pool), NVIDIA questions (RTX award), OpenAI question
- [x] **Gameplay video, 30–60 s, 1920×1080 or higher**: 57 s, 1080p, https://youtu.be/Yd4DKz2yuZ8 (@PhoenixLynxDev) (rules sections 1 and 7(c)): shot list in [VIDEO_SHOTLIST.md](VIDEO_SHOTLIST.md)
- [x] itch.io game page set to **public**
- [x] Devlog linked from the itch page (GitHub repo starts **private**: switch it to **public** before linking, or post the devlog on itch instead)
- [x] Only the Unreal Engine logo used (no other engine/brand logos in the game or on the page)
- [x] Submitted to the jam before the deadline (official: Nov 15, 5:59 PM PST = 6:59 PM Phoenix)
- [x] Any pre-jam material (prompts, scripts, assets) declared; any sourced assets listed with licenses
- [x] Submitted by me (a human), not an agent
- [x] Rated Teen or below; no third-party IP or trademarks other than the UE logo

## Glass Box category (dev log prize)

- [x] Dev log submitted **with the entry, during the contest period** (this repo's DEVLOG.md, linked or posted as an itch devlog)

## Nice to have

- [x] Cover image + screenshots on the itch page (page text drafted: [ITCH_PAGE.md](ITCH_PAGE.md))
- [x] Controls listed on the page (drafted in ITCH_PAGE.md, with the director's tools: F5/F6/F7/F8/Ctrl+1–4 stay in for judges, decided 14:04)
- [x] AI tool usage summarized on the page (pulled from the devlog ledger)
- [x] "Tools you used" field filled in at submission (some partner prizes, e.g. Hyper3D Rodin's pool, depend on it)

## Status log

| When (MST) | Update |
|------------|--------|
| 2026-11-12 19:17 | Checklist created. Nothing ticked yet. |
| 2026-11-12 19:40 | Added rule items found while checking the asset rules: pre-jam declaration, human submits, rating/IP, Glass Box dev log. |
| 2026-11-15 16:48 | **SUBMITTED.** Entry: https://itch.io/jam/2026-epic-ai-megajam/rate/5375494 · Game page: https://phoenixlynx999.itch.io/take-4471 · Video channel: https://www.youtube.com/@PhoenixLynxDev. Categories: Solo Driver, Glass Box. |
| 2026-11-15 16:32 | Video cut from my .exe captures (57 s, 1080p) and uploaded: https://youtu.be/Yd4DKz2yuZ8. Cover + YouTube thumbnail from the reception shot. itch page going public. |
| 2026-11-15 15:12 | FinalBuild packaged OK (1.1 GB, exit 0) and tested: both endings, save/continue by act. Epic's final-day email: zip as TeamName_GameName, video as a link, devlog link optional (Glass Box). Itch art made from in-game screenshots (Builds/ItchArt). |
| 2026-11-15 14:45 | Playtest verification done (critical: NO-ending fall, penthouse edge, YES-ending dialogue, blank inbox rows, F5 shader view): all fixed and checked. Feature freeze. Packaging. Next: .exe test, video, itch page, submit. |
| 2026-11-15 04:20 | Moved the gameplay video into Required. Packaging review → PACKAGING.md. itch page + video shot list drafted. Plan for today: playtest pass, my title background, feature freeze ~noon, package + .exe test, video, itch page, submit by ~5 PM. |

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

