# Concept (v0.8): working title "SOULHASH"

> **Theme: "Director's Cut"** (to confirm on the official jam page). Brainstorm: [THEME_BRAINSTORM.md](THEME_BRAINSTORM.md).
> **Spine:** the whole thing is an orchestrated movie. Your soul bits are the scenes cut from your life. A Director, the Chairman, is behind it all and is never fully seen.

**Genre:** first-person cryptographic horror, explore / stalk-and-chase / puzzle
**Status:** core direction locked. Waiting on jam theme.

## Pitch

You are a standardized citizen-employee at a bright, cheerful corporation. Under the surface, centralized entities have built an illusion overlay to trap people and divert their soul energy. You start to see through it. Every key you solve pulls you further out of the interface and into the truth, while the malware fights back with brainwashing and engineered fear.

## The layers

| Layer | Who made it | Feel | Role |
|-------|------------|------|------|
| **Interface: the Campus** | The corporation | Bright, clean, polite, too perfect | Where the game starts. Your normal workday. |
| **Astral Real-Time Zone** | The gatekeepers (hidden) | The truth: shining linework, glyph-code bleeding through surfaces | Seen through **Soul Sight**. Where your soul energy is held. |
| **Augmented Fear** | The malware | Visceral, rotting, stalked: the campus turned against you | The malware's attack state. Brainwashing + extreme fear. |
| **Source** | No one / you | Pure code | Final goal. |

**Soul Sight = truth. Augmented Fear = lies.** They are opposites, and that's the core tension.

## Real World: the Campus

- You're an employee: badge, ID number, standardized desk, wellness reminders.
- Bright corporate surface: posters thanking you for your energy, consent prompts on doors, smiling staff who are a little too still.
- Mood references (nothing copied): Magilumiere Co. Ltd.'s corporate cheer, Death Note's contracts and rules.
- **The Wellness Center** is the campus clinic. In Augmented Fear it becomes **the Hospital**.

## Soul Sight: you slip into it

Not a free toggle at the start. It grows with each soul bit:

| Soul bits | Soul Sight behavior |
|-----------|--------------------|
| 0 | Involuntary flickers: a second of glyph-light bleeding through a wall, then gone. |
| 1 | Hold to see, briefly; it drains fast. |
| 2 | Longer; reveals false bits and enemy code signatures. |
| 3 | Sees both interface and source. Path to Source opens. |

## Augmented Fear: the slip

The player shouldn't know exactly when it starts. No siren, no hard cut. It creeps in over ~30–90 seconds until you realize it's wrong and something is already following you.

**Creep stages (all driven by one 0→1 "Fear" value):**
1. **Off by a little:** a light buzzes, an ambient sound lags, one poster's text is different.
2. **Draining:** color desaturates, fog thickens, footsteps echo wrong, doors you passed are now closed.
3. **Watched:** staff mannequins have turned to face you, the corridor is longer than it was.
4. **Stalked:** the gatekeeper is visible at the edge of the screen, then it chases.

**Unreal tools:** a post-process blend weight lerped by Fear, a material parameter collection (rot/grime/desaturation on every surface), an audio low-pass + heartbeat layer, and actor swaps out of the player's view (check if it's off-screen before swapping).

**Soul Sight pierces Fear:** using it during Fear shows what's real (true exits, fake enemies, the stalker's code signature). But it drains fast, so it's a tense resource.

## Trapped in time and space

Fear zones should make you feel there's no way out, in both space and time.

**Space:**
- **Looping corridors:** walk to the end and you're back at the start, with one small detail changed. (Unreal: a trigger volume teleports the player by a fixed offset into an identical section, so the jump is invisible.)
- **Doors that lead back:** the exit opens into the room you just left.
- **Stretching halls:** a corridor gets longer each time you look away.

**Time:**
- **Frozen clocks:** every clock on campus stops at the same minute.
- **Repeating announcements:** the same PA message, word for word, again and again.
- **Rewind on capture:** if the stalker catches you, you wake back at your desk at the same minute, still hurt, and something is different. (Death costs you, but feels like being stuck in time instead of a game-over screen.)

**The way out is always real but always hidden:** a looped corridor has one detail that differs per loop, and following it breaks the loop. Soul Sight shows it more clearly.

## Escalation: Fear and illusions scale with progress

| Soul bits | Fear episodes | Traps | Stalker |
|-----------|--------------|-------|---------|
| 0 → 1 | Short, slow creep, one area | One looping corridor | Slow, stalks more than chases |
| 1 → 2 | Longer, faster creep | Frozen clocks, repeating announcements, doors that lead back | Faster; mannequins move when unseen |
| 2 → 3 | Hospital / Wellness Center; Fear barely lifts | Nested loops, rewind on capture, fake clues for the key | Fast, relentless, may be more than one |

Also scales with **distance to Source**: the closer you are, the less the campus pretends to be normal.

## The workday: where the puzzles hide

You're an employee with a job to do. Everyday office tasks are the objectives, and **the puzzles appear inside them.** The mundane is the cover; the conspiracy leaks through it.

**Daily Task List** (HR to-do on your badge/HUD) gives direction: check email, finish compliance training, join the 10:00 call, attend the meeting, talk to a coworker, team building at 4.

| Activity | Normal surface | Where the puzzle/horror leaks in |
|----------|---------------|----------------------------------|
| **Email inbox** (at your desk) | Cheerful company news, memos signed "The Chairman" | Key target glyphs hidden in signature lines and footers. **Phishing emails = malware agents' false bits.** Some emails describe things you haven't done yet. |
| **Mandatory Compliance Training** | Corporate slideshow + quiz | **Key 1 tutorial in disguise:** "Data Integrity Module 1" teaches the XOR rule. Later modules ask stranger questions ("Do you consent to your next take?"). |
| **Video calls** (Zoom-style grid) | Coworkers on camera, small talk | One tile is frozen or looping. One camera shows the room *behind you*. The Chairman's tile is always camera-off: just "C". A muted coworker mouths a glyph sequence. |
| **Meetings** (conference room) | Slides, whiteboard, agenda | Whiteboard glyphs = clues. The meeting repeats word for word (time loop). The agenda lists you as "Item 4: Retake." |
| **Talking to people** (walking the floor) | Friendly coworkers | Honest NPCs give real clues; agents give false ones. |
| **Team building** | Icebreakers, a trust exercise | Starts the deepest Fear episode. |

### The day = three takes = three keys

| Act | Time | Activities | Key | Fear |
|-----|------|-----------|-----|------|
| **1: Morning** | 9:00–12:00 | Onboarding emails, Compliance Training | **Key 1** (tutorial, shown) | First slip, single loop corridor |
| **2: Midday** | 12:00–3:00 | Video call, meeting, talking to people | **Key 2** (target hidden in emails/meeting) | Frozen clocks, mannequins move |
| **3: Overtime** | 4:00–? | Team building → Wellness Center / Hospital | **Key 3** (scattered, decoys) | Fear barely lifts; rewinds |

Then: elevator "C" → wrap party.

**Scope:** MVP = task list, email inbox, Compliance Training (Key 1), one meeting or video call. Stretch = team building as its own sequence, more modules.

## The Chairman (the Director)

Head of the corporation, and the director of your life's production. **Hinted at all game, never fully shown.**

**Foreshadowing, in rising order:**
1. **Lobby portrait:** the Chairman's face is always just cropped out of the frame.
2. **Memos** signed "The Chairman," written in film language: "Places, everyone." "Let's go again from the top."
3. **Call sheet** on the bulletin board, the daily schedule a film crew posts, listing *you* by employee number in the cast.
4. **PA announcements:** "The Chairman thanks you for your performance."
5. **The empty director's chair:** it appears in odd places, a little closer to you each take.
6. **In Fear:** a boom mic shadow, a camera dolly track on the floor, a red REC light in a dark doorway.
7. **Elevator button "C":** never lights. At the end, it does.

## Movie motifs throughout

- **"CUT!" on capture:** a clapperboard snaps the next take number, and you restart at your desk. Take count climbs all game.
- **Script pages** carry the key clues; stage directions describe you, sometimes before you act. Agents plant fakes.
- **NPCs are actors**; malware agents break character (missed marks, glances at a camera that isn't there).
- **Letterbox bars** close in as Fear rises, with film grain. Soul Sight pushes them back.
- **Astral logic = editing reality:** telekinesis moves props; float is wire work. Stretch: short rewind, freeze frame.

## NPCs: anyone could be malware

- The friendly coworkers, receptionist and wellness staff you meet can be **agents of the malware**.
- Which ones are agents is **chosen by the run seed**, so it's different every game.
- Agents look and talk exactly like everyone else. Under Soul Sight they *may* flicker a code signature, but only at higher Sight levels, and not reliably.
- **What they do:** honest NPCs give real clues for the hidden key target; agents plant **false clues** (ties into the false bits). Agents can also start a Fear episode early, lock a door behind you, or report you.
- You never know for sure who to trust. That's the point.

## Fighting back: improvised, not trained

You're an office worker, not a fighter. You fight with whatever the world has.

- **Hot coffee:** throw or splash it to **burn**, a short damage-over-time effect.
- **Grab and throw / swing:** staplers, mugs, monitors, keyboards, reams of printer paper, chairs, binders, small cabinets. Clumsy, heavy, satisfying. (Unreal: Physics Handle component to grab; on throw, apply an impulse; damage on hit = based on the object's speed.)
- **Fire extinguisher:** spray a cone that blinds and slows the stalker for a few seconds; limited charge. Iconic, readable, a great "oh no, use it now" moment.
- **Block their path:** knock over filing cabinets, chairs, carts, water coolers. Enemies slow down crossing debris. (Cheap version: a "debris" overlap that cuts enemy speed, instead of rebuilding the navigation mesh, which is expensive at runtime.)
- **Objects break or run out:** extinguishers empty, thrown things scatter. You can't hold the line forever.

### Astral logic: impossible physics, a little at a time

The deeper you are in the astral realm, the more its rules work for you. A certain amount of impossible physics, nothing too crazy.

| Soul bits | Astral logic unlocked |
|-----------|----------------------|
| 0 | None. Just a person with a stapler. |
| 1 | **Telekinesis (short range):** pull or push objects from a few meters away. |
| 2 | **Hang in the air:** objects you let go float for a moment before falling. **Short float:** hold jump for a brief glide/hover. |
| 3 | **Telekinetic throw** of heavy objects (cabinets), and slightly longer flight. |

**Unreal notes:**
- Telekinesis = the same Physics Handle as grabbing, just with a longer reach and a glow. Nearly free once grabbing works.
- Float/hover = briefly lower the character's gravity scale while jump is held, on a stamina timer.
- Hanging objects = briefly disable gravity on the released physics body.
- All of it drains **Soul Sight**, the same resource, so astral power is always a trade-off.

**Balance:** improvised hits **stagger and slow** gatekeepers; they buy you time to escape or reach a lock. Only a **soul bit (Dissolve / Patch)** truly ends one. You can fight back, but it's never safe to stay.

## Ending: the party

Inspired by the movie *The Game* (mood only, nothing copied).

1. **You reach Source with 3 soul bits.** Elevator button "C" lights. The doors open, the lights come up and confetti falls. It's a **wrap party**: "That's a wrap!" The whole thing was an orchestrated movie, and you were the star.
2. **Everyone's there:** the coworkers, the stalker (mask off, holding a coffee), the malware agents, all smiling, cheering, hugging you. They were your best friends all along. It was all a production.
   - The **director's chair** is there, turned away. The Chairman is never seen.
3. **Something still seems off:** one clock is still frozen, the PA repeats one line, a guest's smile holds too long.
4. **The Collector arrives,** speaking for the Chairman: a polished, gentle presence. Its offer (reels = your soul bits; working draft): *"The astral realm is open now. You can build yourself a world, a palace, a utopian afterlife. You just have to hand over your keys."*
5. **The choice: Yes / No.**
   - **Yes → the theatrical cut:** a bright, polished, happy ending. Too happy.
   - **No → the director's cut:** the reels are taken anyway; you glimpse a few seconds of the uncut, darker version.
6. **Final beat:** Soul Sight flickers one last time on the Collector… and **cut to black** before you can read its code. Were you rewarded, or robbed again?

Left deliberately unresolved. **Draft: refine during storyboard / as it unfolds.**

## Core loop

1. Work your day on the Campus. Small objectives, notice what's wrong.
2. Soul Sight flickers reveal hidden **soul energy** and **Key Locks**.
3. Going after energy triggers **Augmented Fear**: the campus turns, a gatekeeper stalks and chases. It deals real damage.
4. Survive, recover the energy, reach a **Key Lock**, solve it → **mint a soul bit**.
5. Soul bit = stronger Soul Sight, and can be spent to **Dissolve** or **Patch** a gatekeeper.
6. Each bit makes the next Fear episode harder and deeper (Hospital wing).
7. 3 bits → Source.

## Puzzle: XOR Key Lock (locked in, now subtle)

**Base rule:** target signature = a row of glyph-bits. Key fragments = rows of glyph-bits. Toggle fragments; active ones combine by XOR. Match the target. Generated backwards from a secret answer subset, so it's always solvable and never the same.

**Subtle, not easy to check:**
- **The target isn't shown at the lock.** It's hidden in the campus: glyphs on badges, poster borders, clock positions, room numbers. You find it and write it down yourself.
- **No running result.** The lock doesn't show your XOR total; you work it out.
- **No partial feedback.** A wrong submission doesn't say how close you were. It raises Fear.
- **False bits:** malware decoy fragments and fake target clues planted in the environment. Revealed only at Soul Sight 2+.
- **Soul Sight as the check:** holding Sight on the lock briefly shows which glyphs of your result are right. It costs Sight, so checking is never free.

**Difficulty ramp (so jam players don't bounce off):**
1. **Key 1 (tutorial-ish):** target shown at the lock, running result visible. Teaches the XOR rule.
2. **Key 2:** target hidden on campus, running result hidden, Sight check available.
3. **Key 3:** target scattered across Fear zones, with decoy clues.

## Music and audio: composed during the jam, generated at runtime

- **All music is composed fresh during the jam** by me (SkyYamaha). Pre-jam tracks would have to be declared as "sourced assets," which aren't scored, and would break this project's own made-during-the-jam rule.
- **Composed as stems and fragments, not finished songs:** short loops, drones, pads, rhythmic cells, melodic phrases, and stingers, each exportable as a .wav.
- **Assembled live in Unreal with MetaSounds** (Unreal's built-in node-based audio system, which can play, layer, pitch and randomize sounds with logic):
  - The run seed picks which fragments play and in what order, so the soundtrack never repeats exactly.
  - **The Fear value drives the mix:** campus = clean corporate muzak; creeping Fear = detune, low-pass, slowed tempo, layers drop out; chase = percussion and pulse stems slam in.
  - **Soul Sight** adds a shimmering, luminous layer on top.
  - **Time loops:** the music literally loops a few bars and stutters when you're trapped.
- **Ending party:** a cheerful corporate anthem built from the same fragments, with one stem left slightly out of tune.
- **Two palettes:** bright ambient (campus) and dark, eerie ambient (Fear/Astral), crossfaded by Fear. Full plan and recording list in [AUDIO_PLAN.md](AUDIO_PLAN.md).

## Every run is unique

- Seed from high-res clock ticks; never shown or saved.
- Drives: puzzles, energy and lock locations, gatekeeper spawns, which campus details are "wrong."
- Dev-only seed override for debugging.

## Enemies: Gatekeepers

- Malware entities that brainwash and inflict fear. They stalk first and chase second.
- **Difficulty = f(soul bits owned, distance to Source).**
- Code signature visible under Soul Sight; spend a soul bit to Dissolve or Patch.
- MVP: 1 stalker (chase + grab damage). Stretch: a ranged "Notary" variant.

## MVP scope (3 days, solo)

**Must:**
- Campus level (lobby, office floor, corridor, Wellness Center), built once
- Augmented Fear as a state on the same level (post-process + materials + audio + swaps)
- One looping corridor + frozen clocks (cheapest "trapped" effects)
- Rewind on capture
- Soul Sight bleed tied to soul bits
- XOR Key Lock, generated
- Workday: task list, email inbox, Compliance Training (Key 1 tutorial), one meeting or video call
- 1 stalker gatekeeper with chase + damage
- Improvised combat: grab/throw objects, hot coffee burn, fire extinguisher, knock-over debris that slows enemies
- Astral logic tier 1: short-range telekinesis (reuses grab)

**Stretch additions:** astral float/hover and floating objects (tier 2), heavy telekinetic throw (tier 3).
- 3 soul bits → party ending with the Collector (one room, NPCs reused, cut to black)
- 2–3 NPC types, some seeded as malware agents giving false clues
- Title screen, rewind on capture

**Stretch (in order):**
1. Hospital wing as its own deeper Fear area
2. Dissolve / Patch spend
3. Second enemy variant
4. Extra campus "wrongness" events

## Open questions

- [ ] Jam theme. A participant's devlog says it's **"Director's Cut"**. Needs confirming from the official page.
- [x] Old tracks: compose everything fresh; motifs from older work may be re-recorded and noted in the devlog.
- [x] Combat: yes, fight back with improvised objects; only soul bits truly end a gatekeeper.
- [x] "Antics" = the malware's tricks: loops, illusions, agents among the NPCs.
- [x] Ending: Yes/No choice; "No" → taken anyway. Details refined as it unfolds.
- [ ] Final title.
