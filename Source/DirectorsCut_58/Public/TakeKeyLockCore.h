// TAKE 4471 - XOR Key Lock core (engine-independent, header-only).
// Port of Scripts/Reference/keylock.py. Uses no Unreal types so it can be
// compiled and tested outside the engine (Scripts/Reference/test_keylock_core.cpp).
// The random generator is a template parameter: anything with
//   int RandRange(int Min, int Max)   // inclusive, like FRandomStream
#pragma once

namespace TakeKeyLockCore
{
	static constexpr int MaxFragments = 8;

	struct FDifficulty { int Bits, Real, Decoys, PickMin, PickMax; };

	inline FDifficulty GetDifficulty(int KeyIndex)
	{
		switch (KeyIndex)
		{
		case 1:  return { 6, 4, 0, 2, 2 };   // tutorial (Compliance Training)
		case 2:  return { 8, 5, 1, 2, 3 };   // Supply Closet
		default: return { 8, 6, 2, 3, 4 };   // Hospital
		}
	}

	// Glyphs shown as '?' per fragment, by Soul Sight level 0..3 (35%, 20%, 10%, 0%).
	inline int HiddenCount(int Bits, int SightLevel)
	{
		static const int Percent[4] = { 35, 20, 10, 0 };
		const int S = SightLevel < 0 ? 0 : (SightLevel > 3 ? 3 : SightLevel);
		return (Bits * Percent[S] + 50) / 100;   // round half up
	}

	struct FLock
	{
		int KeyIndex = 1;
		int Bits = 6;
		int Target = 0;
		int NumFragments = 0;
		int Fragments[MaxFragments] = {};
		bool IsDecoy[MaxFragments] = {};
		int NumAnswer = 0;
		int Answer[MaxFragments] = {};        // fragment indices of one valid solution
		int HiddenMask[MaxFragments] = {};    // bit b set = glyph b shown as '?'
		int Mask() const { return (1 << Bits) - 1; }
	};

	enum class EResult { Wrong = 0, Solved = 1, Corrupted = 2 };

	inline int PopCount(int X) { int C = 0; while (X) { C += X & 1; X >>= 1; } return C; }

	template <class TRng>
	void Shuffle(int* A, int N, TRng& Rng)
	{
		for (int I = N - 1; I > 0; --I)
		{
			const int J = Rng.RandRange(0, I);
			const int T = A[I]; A[I] = A[J]; A[J] = T;
		}
	}

	// Build a solvable lock backwards: fragments first, secret answer subset, target = XOR.
	template <class TRng>
	FLock Generate(int KeyIndex, int SightLevel, TRng& Rng)
	{
		const FDifficulty D = GetDifficulty(KeyIndex);
		const int Total = D.Real + D.Decoys;
		const int Mask = (1 << D.Bits) - 1;
		FLock L;
		L.KeyIndex = KeyIndex;
		L.Bits = D.Bits;
		L.NumFragments = Total;

		for (int Attempt = 0; Attempt < 10000; ++Attempt)
		{
			// Distinct fragments with 2..Bits-2 glyphs lit
			int Count = 0;
			while (Count < Total)
			{
				const int F = Rng.RandRange(1, Mask);
				bool Dup = false;
				for (int I = 0; I < Count; ++I) { if (L.Fragments[I] == F) { Dup = true; break; } }
				const int P = PopCount(F);
				if (!Dup && P >= 2 && P <= D.Bits - 2) { L.Fragments[Count++] = F; }
			}

			// Choose decoys
			int Order[MaxFragments];
			for (int I = 0; I < Total; ++I) { Order[I] = I; L.IsDecoy[I] = false; }
			Shuffle(Order, Total, Rng);
			for (int I = 0; I < D.Decoys; ++I) { L.IsDecoy[Order[I]] = true; }

			// Secret answer: K distinct real fragments
			int Real[MaxFragments]; int NumReal = 0;
			for (int I = 0; I < Total; ++I) { if (!L.IsDecoy[I]) { Real[NumReal++] = I; } }
			Shuffle(Real, NumReal, Rng);
			const int K = Rng.RandRange(D.PickMin, D.PickMax);
			L.NumAnswer = K;
			L.Target = 0;
			for (int I = 0; I < K; ++I) { L.Answer[I] = Real[I]; L.Target ^= L.Fragments[Real[I]]; }
			// sort answer for readability
			for (int I = 1; I < K; ++I) { int V = L.Answer[I], J = I - 1; while (J >= 0 && L.Answer[J] > V) { L.Answer[J + 1] = L.Answer[J]; --J; } L.Answer[J + 1] = V; }

			// Reject trivial targets
			bool Trivial = (L.Target == 0);
			for (int I = 0; I < Total && !Trivial; ++I) { if (L.Fragments[I] == L.Target) { Trivial = true; } }
			const int TP = PopCount(L.Target);
			if (TP < 2 || TP > D.Bits - 1) { Trivial = true; }
			if (!Trivial) { break; }
		}

		// Hidden glyphs (none on the tutorial key)
		const int NHide = (KeyIndex == 1) ? 0 : HiddenCount(D.Bits, SightLevel);
		for (int F = 0; F < Total; ++F)
		{
			int Pos[16]; for (int B = 0; B < D.Bits; ++B) { Pos[B] = B; }
			Shuffle(Pos, D.Bits, Rng);
			int H = 0; for (int I = 0; I < NHide; ++I) { H |= (1 << Pos[I]); }
			L.HiddenMask[F] = H;
		}
		return L;
	}

	// ActiveMask: bit i set = fragment i switched on.
	inline int XorOf(const FLock& L, int ActiveMask)
	{
		int R = 0;
		for (int I = 0; I < L.NumFragments; ++I) { if (ActiveMask & (1 << I)) { R ^= L.Fragments[I]; } }
		return R;
	}

	inline EResult Evaluate(const FLock& L, int ActiveMask)
	{
		for (int I = 0; I < L.NumFragments; ++I) { if ((ActiveMask & (1 << I)) && L.IsDecoy[I]) { return EResult::Corrupted; } }
		return XorOf(L, ActiveMask) == L.Target ? EResult::Solved : EResult::Wrong;
	}

	// Soul Sight check: bit b set = glyph b of the current result matches the target.
	inline int SightCheckMask(const FLock& L, int ActiveMask)
	{
		return ~(XorOf(L, ActiveMask) ^ L.Target) & L.Mask();
	}
}
