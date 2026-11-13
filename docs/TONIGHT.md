# Tonight's runbook: Fri Nov 13

Total: about 20 minutes before we're building. Claude can read your logs and reports from the cloud, so you don't need to copy errors by hand. Just say "check the log".

**Already done for you (while you were at work):**
- Plugins switched on in `DirectorsCut_58.uproject`: Python Editor Script, Editor Scripting Utilities, **Unreal MCP** (`ModelContextProtocol`), **All Toolsets**. Backup of the old file: `_to_delete/DirectorsCut_58.uproject.bak`.
- `Content/Python/init_unreal.py` starts the MCP server automatically every time the editor opens.
- The project's default game mode is already the First Person one, so no setting is needed.

---

## 1. Open the project (2 min)
Open `DirectorsCut_58` in UE 5.8. It may say plugins changed or ask to restart. Say yes.
**Expect:** the Output Log shows `[TAKE 4471] Unreal MCP server start requested`.
*If it complains a plugin is missing, tell Claude. It can edit the .uproject from the cloud.*

## 2. Preflight (1 min)
**Tools → Execute Python Script…** → `Scripts/Editor/preflight_check.py`
**Expect:** `0 FAIL` at the end. It also creates `.mcp.json` in the project folder.
→ Tell Claude "preflight done". It reads `Saved/Take4471_preflight.txt` and fixes any script that would fail.

## 3. Build the campus (1 min)
**Tools → Execute Python Script…** → `Scripts/Editor/build_greybox_campus.py`
**Expect:** a progress bar, then the level `L_Campus_Greybox` opens: 8 rooms, cubicles, orange throwables.
Press **Play**: you spawn behind desk #4471 facing your monitor. 📸 **Screenshot #1: first walk-through.**

## 4. Fear materials (1 min)
**Tools → Execute Python Script…** → `Scripts/Editor/build_fear_assets.py`
**Expect:** walls off-white, carpet blue-grey, props mint, throwables orange.

## 4b. Import Claude's assets (1 min)
**Tools → Execute Python Script…** → `Scripts/Editor/import_jam_assets.py`
**Expect:** `/Game/Props/Meshes` (16 office props), `/Game/UI/Glyphs`, `/Game/Props/Textures` (posters, portrait, call sheet, charts), `/Game/Audio/Placeholder`.
Then **Tools → Execute Python Script…** → `Scripts/Editor/dress_campus.py`: furnishes every room with the real props (208 placed, 111 throwable), turns the break room into a kitchen, places the mannequins.
**Check:** your desk's monitor should face your chair. If **all** props face the wrong way, open `dress_campus.py`, set `YAW_FIX = 180` (or 90 / -90) at the top, and run it again. If they're tiny or huge, change `SCALE_FIX`. 📸 **Screenshot: the dressed office.**

## 5. Connect Claude to the editor (5–10 min)
The MCP server runs inside the editor at `http://127.0.0.1:8000/mcp`. Two ways to connect:

**A. This chat (Claude desktop app)** so you can keep messaging from your phone:
1. Claude desktop app → **Settings → Developer → Edit Config** (opens `claude_desktop_config.json`).
2. Add this (or merge into your existing `mcpServers`):
   ```json
   {
     "mcpServers": {
       "unreal-mcp": {
         "command": "npx",
         "args": ["-y", "mcp-remote", "http://127.0.0.1:8000/mcp"]
       }
     }
   }
   ```
   (`mcp-remote` is a small bridge that turns the editor's web address into something the desktop app can launch. It needs Node.js. **Already installed** on this PC (`C:\Program Files\nodejs`). Check the version in a terminal with `node -v`: 18 or newer is fine; if older, update from nodejs.org, LTS.)
3. Fully quit and reopen the Claude desktop app, with the Unreal editor already open.
4. Tell Claude "check the Unreal connection". It checks whether `unreal-mcp` shows up.

**B. Claude Code on the PC** (Epic's documented route). Open a terminal in the project folder, run `claude`, approve the `unreal-mcp` server from `.mcp.json`, then `/mcp` to confirm it's connected.

Try A first. If A doesn't connect, B is the reliable fallback.

## 6. First real build (the fun part)
Fear Director, from `docs/BLUEPRINT_RECIPES.md` #1. With MCP connected, Claude builds it with you.
📸 **Screenshots #2–4: the same hallway at Fear 0, 0.5 and 1.**

## 7. Git on the PC (before bed, 5 min)
**Git for Windows is already installed** (`C:\Program Files\Git`, usually includes Git LFS; check with `git lfs version`). Install **GitHub Desktop** for easy pushing → File → Clone `PhoenixLynx/epic-megajam-dev` into a temp folder → Claude moves the `.git` folder into `DirectorsCut_58` → commit the Unreal files → **Push**. From then on, Unreal assets push from the PC (Git LFS included).
