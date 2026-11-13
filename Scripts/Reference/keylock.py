"""
keylock.py: reference implementation of the XOR Key Lock (TAKE 4471).

This is the design-truth version of the puzzle logic, written in plain Python
so it can be tested thoroughly. The in-game version is a straight port to
Blueprint (or C++); see docs/KEYLOCK_SPEC.md.

The puzzle
----------
- A TARGET signature: N glyph-bits (an integer whose low N bits matter).
- F key FRAGMENTS: each is also N glyph-bits.
- The player toggles fragments on/off. The lock XORs the active fragments
  together. If the result equals the target, and no malware DECOY fragment is
  active, the key is solved.

XOR in plain terms: for each glyph position, count how many active fragments
have that glyph lit. Odd count -> the result glyph is lit. Even -> unlit.

Why it's always solvable
------------------------
We build it backwards: make the fragments first, secretly pick a subset of
the real (non-decoy) ones, and XOR them together. That XOR *is* the target.
So at least one correct answer always exists.
"""

from __future__ import annotations

import random
from dataclasses import dataclass, field

# Difficulty per key (1 = tutorial in Compliance Training, 2-3 = subtle).
DIFFICULTY = {
    #     bits  real frags  decoys  answer size (min, max)
    1: dict(bits=6, real=4, decoys=0, pick=(2, 2)),
    2: dict(bits=8, real=5, decoys=1, pick=(2, 3)),
    3: dict(bits=8, real=6, decoys=2, pick=(3, 4)),
}

# Fraction of fragment glyphs shown as "?" at each Soul Sight level.
# Holding Soul Sight on the lock reveals them (and costs Sight).
HIDDEN_FRACTION = {0: 0.35, 1: 0.20, 2: 0.10, 3: 0.0}

# Decoys are recognisable as malware only at this Soul Sight level or above.
DECOY_REVEAL_SIGHT = 2


@dataclass
class KeyLock:
    seed: int
    key_index: int
    bits: int
    target: int
    fragments: list[int]
    is_decoy: list[bool]
    answer: list[int]                       # indices of one valid solution
    hidden: list[list[int]] = field(default_factory=list)  # per fragment: hidden glyph positions

    @property
    def mask(self) -> int:
        return (1 << self.bits) - 1


def _popcount(x: int) -> int:
    return bin(x).count("1")


def generate(seed: int, key_index: int, sight_level: int = 0) -> KeyLock:
    """Build a solvable Key Lock. Same seed + key_index -> same puzzle."""
    cfg = DIFFICULTY[key_index]
    rng = random.Random(seed * 1000 + key_index)
    bits, mask = cfg["bits"], (1 << cfg["bits"]) - 1
    total = cfg["real"] + cfg["decoys"]

    while True:  # retry until the puzzle isn't trivial (rarely loops more than once)
        # Distinct fragments, each with 2..bits-2 glyphs lit (no empty/full/near-empty rows).
        frags: list[int] = []
        while len(frags) < total:
            f = rng.randint(1, mask)
            if f not in frags and 2 <= _popcount(f) <= bits - 2:
                frags.append(f)

        order = list(range(total))
        rng.shuffle(order)
        decoy_ids = set(order[: cfg["decoys"]])
        real_ids = [i for i in range(total) if i not in decoy_ids]

        k = rng.randint(*cfg["pick"])
        answer = sorted(rng.sample(real_ids, k))
        target = 0
        for i in answer:
            target ^= frags[i]

        # Reject trivial targets: empty, a single fragment, or too few/many glyphs lit.
        if target == 0 or target in frags or not (2 <= _popcount(target) <= bits - 1):
            continue
        break

    lock = KeyLock(seed=seed, key_index=key_index, bits=bits, target=target,
                   fragments=frags, is_decoy=[i in decoy_ids for i in range(total)],
                   answer=answer)
    lock.hidden = hidden_glyphs(lock, sight_level, rng)
    return lock


def hidden_glyphs(lock: KeyLock, sight_level: int, rng: random.Random) -> list[list[int]]:
    """Which glyph positions show as '?' per fragment. Key 1 (tutorial) hides nothing."""
    if lock.key_index == 1:
        return [[] for _ in lock.fragments]
    frac = HIDDEN_FRACTION[min(sight_level, 3)]
    n_hide = round(lock.bits * frac)
    return [sorted(rng.sample(range(lock.bits), n_hide)) for _ in lock.fragments]


def evaluate(lock: KeyLock, active: list[int]) -> str:
    """What the lock does when the player submits.
    Returns 'solved', 'corrupted' (a decoy was active), or 'wrong'."""
    if any(lock.is_decoy[i] for i in active):
        return "corrupted"
    result = 0
    for i in active:
        result ^= lock.fragments[i]
    return "solved" if result == lock.target else "wrong"


def sight_check(lock: KeyLock, active: list[int]) -> list[bool]:
    """Soul Sight check (key 2+): which glyphs of the player's current result
    already match the target. Costs Sight in game; never shows the target itself."""
    result = 0
    for i in active:
        result ^= lock.fragments[i]
    return [((result >> b) & 1) == ((lock.target >> b) & 1) for b in range(lock.bits)]


def glyph_row(value: int, bits: int, hidden: list[int] = ()) -> str:
    """Text preview: lit glyph = #, unlit = ., hidden = ?"""
    return "".join("?" if b in hidden else ("#" if (value >> b) & 1 else ".")
                   for b in reversed(range(bits)))


def describe(lock: KeyLock, sight_level: int = 0) -> str:
    lines = [f"Key {lock.key_index}  seed {lock.seed}  ({lock.bits} glyphs)",
             f"  TARGET   {glyph_row(lock.target, lock.bits)}"]
    for i, f in enumerate(lock.fragments):
        tag = "  <- malware" if lock.is_decoy[i] and sight_level >= DECOY_REVEAL_SIGHT else ""
        lines.append(f"  frag {i}   {glyph_row(f, lock.bits, lock.hidden[i])}{tag}")
    lines.append(f"  (one answer: fragments {lock.answer})")
    return "\n".join(lines)


if __name__ == "__main__":
    for key, sight in [(1, 0), (2, 1), (3, 2)]:
        print(describe(generate(seed=4471, key_index=key, sight_level=sight), sight), "\n")
