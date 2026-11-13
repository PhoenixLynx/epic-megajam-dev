# Scripts

## Tonight: build the grey-box campus (about 2 minutes)

1. Open `DirectorsCut_58` in Unreal 5.8.
2. **Edit → Plugins** → search **Python Editor Script Plugin** → enable → restart if asked.
3. **Tools → Execute Python Script…** → pick `Scripts/Editor/build_greybox_campus.py`.
4. It creates and saves **`/Game/Maps/L_Campus_Greybox`**: 8 rooms, doorways, desk #4471, cubicles, lights, Corridor B markers, a Fear post-process volume, and a PlayerStart behind your desk.
5. In **World Settings**, set **GameMode Override** to the First Person template's game mode so you spawn as the first-person player, then press **Play**.

Re-running is safe: it deletes and rebuilds everything labelled `GB_`. Layout lives in the `ROOMS`, `PROPS` and `MARKERS` lists at the top of the script, so tweak the numbers there.

Preview of the layout: `devlog/screenshots/2026-11-13_greybox_preview.png`.

## Reference code (not used in the game directly)

- `Reference/keylock.py`: the XOR Key Lock generator, design-truth version. Port spec: `docs/KEYLOCK_SPEC.md`.
- `Reference/test_keylock.py`: tests. Run `python Scripts/Reference/test_keylock.py`.

## Game text

`Data/Text/*.csv` → import into Unreal as DataTables (drag the CSV into the Content Browser, pick the matching struct). Edit lines in `Data/Text/make_text_tables.py` and re-run it.
