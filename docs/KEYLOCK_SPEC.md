# XOR Key Lock: spec + Blueprint port

Reference code: `Scripts/Reference/keylock.py` (tested: `test_keylock.py`, 5 tests × 5,000 seeds per key).
Port the **algorithm**, not the random sequence: Unreal's `Random Stream` produces different numbers than Python, which is fine.

## Difficulty

| Key | Where | Glyphs | Real fragments | Malware decoys | Answer size | Hidden glyphs |
|-----|-------|--------|----------------|----------------|-------------|---------------|
| 1 | Compliance Training (tutorial) | 6 | 4 | 0 | 2 | none; target + running result shown |
| 2 | Supply Closet | 8 | 5 | 1 | 2–3 | by Sight level; no running result |
| 3 | Hospital | 8 | 6 | 2 | 3–4 | by Sight level; target scattered |

Hidden-glyph fraction by Soul Sight level: 0 → 35%, 1 → 20%, 2 → 10%, 3 → 0%. Decoys are tagged as malware at Sight 2+.

## Data (Blueprint struct `S_KeyLock`)

| Field | Type | Notes |
|-------|------|-------|
| Bits | Integer | 6 or 8 |
| Target | Integer | low `Bits` bits used |
| Fragments | Integer array | one integer per fragment |
| IsDecoy | Boolean array | same length as Fragments |
| Answer | Integer array | fragment indices of one solution (for debugging only) |
| Hidden | Integer array per fragment | glyph positions shown as `?` |

Glyph `b` of a value is lit when `(Value >> b) & 1 == 1`. In Blueprint: `Value & (1 << b) != 0` (use the integer **Bitwise AND** and **Shift Left** nodes).

## Generate (function `GenerateKeyLock(Seed, KeyIndex, SightLevel)`)

1. `Make Random Stream` from `Seed * 1000 + KeyIndex`.
2. **Fragments:** repeat until you have `Real + Decoys` of them: pick a random integer 1…(2^Bits − 1); keep it if it's not already in the list and has between 2 and Bits−2 glyphs lit.
3. **Decoys:** shuffle the indices; the first `Decoys` are malware.
4. **Answer:** pick `AnswerSize` random non-decoy indices.
5. **Target** = XOR of the chosen fragments (integer **XOR** node, in a loop).
6. **Reject and retry** (back to step 2) if the target is 0, equals any single fragment, or has fewer than 2 / all glyphs lit.
7. **Hidden glyphs:** key 1 → none. Otherwise, per fragment, pick `round(Bits × HiddenFraction[Sight])` random positions.

## Evaluate (on Submit)

- Any active fragment is a decoy → **Corrupted** (raise Fear sharply; the lock flickers red).
- XOR of active fragments == Target → **Solved** → mint the access pass.
- Otherwise → **Wrong** (raise Fear; key 2+ gives no hint how close).

## Soul Sight check (keys 2–3)

Holding Soul Sight on the lock drains Sight and briefly marks each glyph of the player's *current result* as right or wrong (compare bit by bit with the target). It never shows the target itself.

## Run seed

`RunSeed` = a value from the high-resolution clock at game start (e.g. `Now` → ticks, truncated to an integer), stored on the Game Instance and never shown. Each key uses `RunSeed` + its key index. Dev builds read an optional override seed from a console variable to reproduce bugs.
