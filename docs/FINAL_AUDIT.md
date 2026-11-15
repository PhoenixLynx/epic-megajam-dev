# Final notes audit: Sun Nov 15, 12:45 MST

All 322 playtest notes (Sat 14:44 → Sun 12:34) were read and checked against the code.
- **Fixed / built:** almost all of them.
- **Liked as they are:** about 40 were praise ("nice slow turn", "they chanted my number") and stay as they are.
- **Still open:** what's left is below.

## Critical: verify before packaging
| # | Note | Status |
|---|------|--------|
| 295, 317 | Stalker foreshadowing never seen | **Fixed 12:45.** It did fire at 12:31:59, but it started in the office while I was down Corridor B, and I trust-fell 7 s later. It now starts 15 s into overtime, at a walkable spot 8–16 m from me (preferably behind me), wherever I am. |
| 321 | Can fall off the penthouse | **Fixed 12:45:** an invisible wall above the glass rail, plus a safety net that puts me back by the elevator. |
| 001, 093, 103, 200 | Intro bleed-through before the title | Comes and goes in the editor. **Check it in the packaged .exe**, the real test. |
| 307 | Stale blank emails on the next take | **Not reproduced yet.** A screenshot of the inbox on take 2 would help. |

## Fixed in the 12:45 batch (needs Rebuild + scripts)
- 308, 315: Reception monitor sits on the desk, in reach, facing the receptionist, with a keyboard.
- 314: The visitor sign-in sheet is turned to the visitor.
- 309: The office clock moved clear of the "SUNRISE No. 4471" print.
- 310: The window gets a deep casing (head, jambs, sill) and half-drawn blinds, so it reads as a window.
- 311: Fridge top shelf. `SM_FridgeBody` was re-exported with a short freezer lip, and a soda and milk are back on the top shelf.
- 318: The Collector gets a slice of cake too (he never eats it).
- 319: The Collector is grey-skinned like the stalker.
- 320: Coffee mugs on the penthouse table (F to sip).
- 313: The pause menu page is now "To-do list" (N still opens it).
- 119: The conference phone is bolted down.

## Open: nice to have (only if time allows; otherwise post-jam)
- 280: Something to do during the conference call.
- 316: The security monitors show a live camera of you (a scene capture) instead of a still.
- 169: Corridor B extra long. `extend_corridor_b.py` is written (dry run by default).
- 063: More ward props (IV bags, nurses, doctors).
- 073, 087: An actors' lounge, and actors on break outside the window.
- 192: "Do something with this wall" (lobby).
- 302: "Notes about charts in the kitchen": unclear, needs a look together.
- 322: "If you do this cut to credits???" (asked right after falling off the penthouse). Falling is blocked now, so there's nothing to cut from.
- Title background: my own design, later today.
