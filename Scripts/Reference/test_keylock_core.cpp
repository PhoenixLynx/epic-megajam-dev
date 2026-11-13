// Standalone tests for Source_Pending/.../TakeKeyLockCore.h (no Unreal needed).
// Build: g++ -std=c++17 -O2 -I Source_Pending/DirectorsCut_58/Public Scripts/Reference/test_keylock_core.cpp -o /tmp/t && /tmp/t
#include <cassert>
#include <cstdio>
#include <random>
#include "TakeKeyLockCore.h"

using namespace TakeKeyLockCore;

struct FStdRng   // stands in for FRandomStream: RandRange is inclusive
{
	std::mt19937 G;
	explicit FStdRng(unsigned Seed) : G(Seed) {}
	int RandRange(int Min, int Max) { return std::uniform_int_distribution<int>(Min, Max)(G); }
};

int CountSolutions(const FLock& L)
{
	int N = 0;
	for (int M = 1; M < (1 << L.NumFragments); ++M) { if (Evaluate(L, M) == EResult::Solved) { ++N; } }
	return N;
}

int main()
{
	for (int Key = 1; Key <= 3; ++Key)
	{
		const FDifficulty D = GetDifficulty(Key);
		int Unique = 0, MaxSol = 0;
		for (unsigned Seed = 0; Seed < 20000; ++Seed)
		{
			FStdRng Rng(Seed * 1000 + Key);
			const FLock L = Generate(Key, 0, Rng);
			int AnswerMask = 0; for (int I = 0; I < L.NumAnswer; ++I) { AnswerMask |= 1 << L.Answer[I]; }
			assert(Evaluate(L, AnswerMask) == EResult::Solved);
			assert(L.Target != 0 && L.Target <= L.Mask());
			int Decoys = 0;
			for (int I = 0; I < L.NumFragments; ++I)
			{
				assert(L.Fragments[I] != L.Target);
				for (int J = I + 1; J < L.NumFragments; ++J) { assert(L.Fragments[I] != L.Fragments[J]); }
				if (L.IsDecoy[I]) { ++Decoys; assert(!(AnswerMask & (1 << I))); assert(Evaluate(L, AnswerMask | (1 << I)) == EResult::Corrupted); }
				if (Key == 1) { assert(L.HiddenMask[I] == 0); }
				else { assert(PopCount(L.HiddenMask[I]) == HiddenCount(L.Bits, 0)); }
			}
			assert(Decoys == D.Decoys);
			assert(SightCheckMask(L, AnswerMask) == L.Mask());
			const int S = CountSolutions(L); if (S == 1) { ++Unique; } if (S > MaxSol) { MaxSol = S; }
		}
		std::printf("key %d: 20000 seeds OK, unique-answer %.0f%%, max answers %d\n", Key, Unique / 200.0, MaxSol);
	}
	// determinism
	FStdRng A(42), B(42); const FLock LA = Generate(2, 1, A), LB = Generate(2, 1, B);
	assert(LA.Target == LB.Target && LA.Fragments[0] == LB.Fragments[0]);
	assert(HiddenCount(8, 0) == 3 && HiddenCount(8, 1) == 2 && HiddenCount(8, 2) == 1 && HiddenCount(8, 3) == 0);
	std::printf("ALL PASS\n");
	return 0;
}
