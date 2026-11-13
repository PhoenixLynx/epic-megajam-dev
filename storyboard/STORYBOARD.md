# Storyboard v0.1: TAKE 4471 (working title)

Theme: **Director's Cut**. Spine: your life is an orchestrated movie, and your soul bits are the scenes cut from it.
Legend: **[MVP]** must ship · **[STRETCH]** if time allows · 📸 capture a screenshot or clip for the devlog and itch page.

---

## 0. Campus map (one floor, built once)

```
                 ┌──────────────┐
                 │  ELEVATOR    │  buttons: 1, 2, 3 ... "C" (never lights)
                 └──────┬───────┘
┌──────────────┐ ┌──────┴───────┐ ┌──────────────┐
│ CONFERENCE   │ │  RECEPTION   │ │ BREAK ROOM   │
│ ROOM         │─│  LOBBY       │─│ coffee,      │
│ calls,       │ │  Chairman    │ │ water cooler │
│ meetings     │ │  portrait    │ └──────────────┘
└──────────────┘ └──────┬───────┘
                 ┌──────┴────────────────────────┐
                 │  OPEN OFFICE FLOOR            │
                 │  cubicles · YOUR DESK #4471   │
                 │  bulletin board (call sheet)  │
                 └──────┬─────────────────┬──────┘
          ┌─────────────┴──┐      ┌───────┴────────┐
          │  CORRIDOR B    │      │ SUPPLY CLOSET  │
          │  (the loop)    │      │ Key Lock 2     │
          └─────────┬──────┘      └────────────────┘
          ┌─────────┴──────────────┐
          │  WELLNESS CENTER       │  → becomes the HOSPITAL in Act 3
          └────────────────────────┘
```

**[MVP]** all rooms above. Reused by every act: the same rooms get stranger as the day goes on.

---

## ACT 1: MORNING (9:00 → 12:00) · "Take 1"

**Teaches:** look, interact, grab/throw, Soul Sight flicker, Fear, rewind, the Key Lock.

| # | Beat | What the player sees and does | Systems |
|---|------|-------------------------------|---------|
| 1.1 | **Cold open** | Black. A clapperboard snaps. White text: `TAKE 1`. Fade in at your desk. Wall clock: 8:59 → 9:00. PA chime: *"Good morning, team. The Chairman thanks you for your energy."* 📸 | Clapper UI, PA audio |
| 1.2 | **Task list** | Badge HUD pops in: ☐ Check email ☐ Get coffee ☐ Complete Compliance Training | Task list **[MVP]** |
| 1.3 | **Email** | Desk monitor, 3 emails: *Welcome back!* · Chairman memo: *"Places, everyone. Big day."* · IT notice with a row of odd glyphs in the footer (first planted glyph, foreshadowing). | Email inbox **[MVP]** |
| 1.4 | **Coffee run** | Walk past the Chairman's portrait in the lobby (face cropped out of frame). Break room: pick up a mug (teaches grab). A coworker by the cooler, small talk, smiles a beat too long. 📸 | Grab, NPC dialogue |
| 1.5 | **First flicker** | Walking back, Soul Sight flickers **involuntarily** for one second: glyph-light bleeds through the walls, and a glowing wisp of soul energy hangs at the end of **Corridor B**. Then it's gone. 📸 | Soul Sight bleed |
| 1.6 | **The creep** | You follow it. Corridor B: a light buzzes (Fear 1)… colors drain, footsteps echo (2)… the corridor is longer than it was; walk to the end and you're back at the start, one poster changed (3). | Fear value, loop corridor **[MVP]** |
| 1.7 | **First stalker** | Edge of screen: something tall in a suit. Letterbox bars slide in. It walks, then **runs**. 📸 | Stalker AI **[MVP]**, letterbox |
| 1.8a | **Escape** | Spot the one detail different in this loop (the poster), follow it → loop breaks → grab the energy wisp → Fear drains away, back in a normal hallway. | |
| 1.8b | **Caught** | Grab → black → *"Cut. Again."* → `TAKE 2` → desk, 9:00, hurt. One thing changed (the mug is already on your desk). Try again. | Rewind on capture **[MVP]** |
| 1.9 | **Compliance Training** | Back at desk with the energy. *"Data Integrity Module 1"*: a cheerful slideshow that teaches the **XOR Key Lock**. Target and running result are **visible** (tutorial). Last quiz question: *"Do you consent to your next take?"* Both buttons say **Yes**. 📸 | Key Lock **[MVP]** |
| 1.10 | **Soul bit 1** | Lock solves → a key glyph burns into your badge → **telekinesis unlocked**. Clock jumps: 12:00. PA: *"Lunch is served."* | Soul bit, telekinesis **[MVP]** |

---

## ACT 2: MIDDAY (12:00 → 3:00) · Fear arrives faster

**Teaches:** hidden targets, false clues, fighting back, mannequins, frozen time.

| # | Beat | What the player sees and does | Systems |
|---|------|-------------------------------|---------|
| 2.1 | **Lunch, again** | PA: *"Lunch is served."*, then exactly the same words again. And again. | Repeating PA **[MVP]** |
| 2.2 | **New tasks** | ☐ Join 12:30 call ☐ Attend 1:00 meeting ☐ Check in with 3 coworkers ☐ Reply to the Chairman | |
| 2.3 | **The video call** | Conference room screen: a grid of coworker tiles. One tile is frozen mid-laugh. One camera shows the room **behind you**. The Chairman's tile: camera off, just **"C"**. **Live captions** for a muted coworker scroll glyphs instead of words → clue fragment. 📸 | Video call **[MVP]** (or meeting) |
| 2.4 | **The meeting** | Slides and a whiteboard. Whiteboard glyphs = clue fragment. Agenda: *"Item 4: Retake."* The meeting starts over, word for word. | Meeting **[STRETCH]** if the call is built |
| 2.5 | **Coworkers** | Three people with clues. One is a **malware agent**: same smile, gives a **false** fragment. (A higher Sight level might catch their code flicker.) | NPC agents **[MVP]** |
| 2.6 | **Frozen time** | Every clock on the floor stops at **2:14**. Fear begins faster. Mannequin staff turn to face you; they move only when you're not looking. 📸 | Frozen clocks **[MVP]**, mannequins **[MVP]** |
| 2.7 | **Fight back** | The stalker comes for you in the open office. Throw a keyboard, splash hot coffee, empty the fire extinguisher, knock a filing cabinet into its path, telekinesis-yank a chair. It staggers, but it keeps coming. 📸 | Improvised combat **[MVP]** |
| 2.8 | **Energy + Key 2** | The energy is in the conference room's Astral layer. Key Lock 2 is in the **Supply Closet**: **target hidden** (assembled from the email footer + captions + whiteboard), **no running result**, wrong guess raises Fear. Hold Soul Sight on the lock to check, at a cost. | Subtle lock **[MVP]** |
| 2.9 | **Soul bit 2** | → **float + floating objects** unlocked. The bulletin board now shows a **call sheet** listing *EMPLOYEE 4471* as cast. The director's chair is standing in your cubicle. 📸 | Float **[STRETCH]** |

---

## ACT 3: OVERTIME (4:00 → ?) · Fear barely lifts

**Teaches:** everything at once, under pressure.

| # | Beat | What the player sees and does | Systems |
|---|------|-------------------------------|---------|
| 3.1 | **Team building** | PA: *"Mandatory team building in the Wellness Center."* Coworkers stand in a circle. Trust exercise: you fall backward… | |
| 3.2 | **Nobody catches you** | …and you fall **through the floor** into the **Hospital**. Fear is near-max and stays there. 📸 | **[STRETCH]** separate hospital; **[MVP] fallback:** Wellness Center transforms in place |
| 3.3 | **The ward** | Beds arranged in cubicle rows. Patient charts are **script pages**: stage directions about you, some describing what you do seconds before you do it. Boom-mic shadow on the wall. A red REC light in a doorway. | Script pages **[MVP]** |
| 3.4 | **Scattered key** | Key 3's target is split across charts and monitors, **with decoys**. Nested loops: doors lead back to the ward. Capture = rewind to the ward entrance. Maybe two stalkers. | Decoys **[MVP]**, 2nd stalker **[STRETCH]** |
| 3.5 | **Soul bit 3** | Full Soul Sight: you can see the interface **and** the source. Heavy telekinesis. The stalkers' code is readable; spend a bit to **Dissolve** or **Patch** one. | Dissolve/Patch **[STRETCH]** |
| 3.6 | **"C"** | Back at the elevator. Button **C** is lit. 📸 | |

---

## ENDING: THE WRAP PARTY

| # | Beat | What the player sees and does |
|---|------|-------------------------------|
| E.1 | Doors open. Lights up, confetti, applause: **"That's a wrap on Employee 4471!"** Crew shirts. 📸 |
| E.2 | Everyone's there and kind: the coworkers, the agents, the stalker with its mask off, holding a coffee. It was all a production. |
| E.3 | Something's off: one clock still frozen at 2:14, the PA repeating one line under the music, one smile held too long. |
| E.4 | The **director's chair**, turned away. The Chairman is never seen. |
| E.5 | **The Collector**, speaking for the Chairman: *"You've earned your director's cut. The astral realm is open now. Build yourself any world you want. Just hand over your keys."* |
| E.6 | **Yes** → the **theatrical cut**: a bright, perfect palace. Too perfect. · **No** → the reels are taken anyway; a few seconds of the darker, uncut version. |
| E.7 | Soul Sight flickers one last time on the Collector… **cut to black** before you can read its code. |
| E.8 | Credits roll. The "Directed by" line is left blank. |

---

## Scope fallback, if we're short on time

1. Drop the meeting (keep the video call).
2. Hospital = Wellness Center transforming in place.
3. Float, Dissolve/Patch, and the 2nd stalker cut.
4. Ending = one room, static NPCs, the choice, cut to black.
