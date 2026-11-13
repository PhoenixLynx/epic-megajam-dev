"""Tests for keylock.py. Run: python Scripts/Reference/test_keylock.py"""
import itertools
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from keylock import DIFFICULTY, evaluate, generate, sight_check  # noqa: E402

SEEDS = range(5000)


def all_solutions(lock):
    """Brute force every subset of real fragments that hits the target."""
    real = [i for i, d in enumerate(lock.is_decoy) if not d]
    sols = []
    for r in range(1, len(real) + 1):
        for combo in itertools.combinations(real, r):
            x = 0
            for i in combo:
                x ^= lock.fragments[i]
            if x == lock.target:
                sols.append(list(combo))
    return sols


def test_always_solvable_and_nontrivial():
    for key in DIFFICULTY:
        for s in SEEDS:
            lock = generate(s, key)
            assert evaluate(lock, lock.answer) == "solved", (key, s)
            assert lock.target != 0
            assert lock.target not in lock.fragments          # no one-fragment answers
            assert len(set(lock.fragments)) == len(lock.fragments)
            assert not any(lock.is_decoy[i] for i in lock.answer)
            assert sum(lock.is_decoy) == DIFFICULTY[key]["decoys"]
            assert lock.target <= lock.mask


def test_deterministic():
    for key in DIFFICULTY:
        a, b = generate(123, key), generate(123, key)
        assert (a.target, a.fragments, a.is_decoy) == (b.target, b.fragments, b.is_decoy)
        assert generate(124, key).fragments != a.fragments


def test_decoy_corrupts_even_if_xor_matches():
    for s in SEEDS:
        lock = generate(s, 3)
        d = lock.is_decoy.index(True)
        assert evaluate(lock, lock.answer + [d]) == "corrupted"


def test_wrong_and_sight_check():
    lock = generate(7, 2)
    assert evaluate(lock, []) == "wrong"
    assert all(sight_check(lock, lock.answer))
    assert not all(sight_check(lock, []))


def test_hidden_glyphs():
    assert all(h == [] for h in generate(1, 1, sight_level=0).hidden)   # tutorial: nothing hidden
    low = sum(len(h) for h in generate(1, 2, sight_level=0).hidden)
    high = sum(len(h) for h in generate(1, 2, sight_level=3).hidden)
    assert low > high == 0


def report_solution_counts():
    for key in DIFFICULTY:
        counts = [len(all_solutions(generate(s, key))) for s in range(500)]
        print(f"  key {key}: answers per puzzle  min {min(counts)}  max {max(counts)}  "
              f"unique-answer puzzles {sum(c == 1 for c in counts) / len(counts):.0%}")


if __name__ == "__main__":
    for name, fn in list(globals().items()):
        if name.startswith("test_"):
            fn()
            print(f"PASS {name}")
    report_solution_counts()
