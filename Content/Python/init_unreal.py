"""
init_unreal.py: runs automatically every time the editor starts (Python Editor Script Plugin).
TAKE 4471: starts the Unreal MCP server so Claude can connect without any clicks.
"""
import unreal

try:
    unreal.SystemLibrary.execute_console_command(None, "ModelContextProtocol.StartServer")
    unreal.log("[TAKE 4471] Unreal MCP server start requested (http://127.0.0.1:8000/mcp)")
except Exception as e:  # noqa: BLE001
    unreal.log_warning(f"[TAKE 4471] Could not start Unreal MCP server: {e}")
