// TAKE 4471 - XOR Key Lock for Blueprints
#include "TakeKeyLockLibrary.h"
#include "TakeKeyLockCore.h"
#include "TakeGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RandomStream.h"

namespace
{
	struct FUnrealRng
	{
		FRandomStream Stream;
		explicit FUnrealRng(int32 Seed) : Stream(Seed) {}
		int RandRange(int Min, int Max) { return Stream.RandRange(Min, Max); }   // inclusive
	};

	int32 ToMask(const TArray<int32>& Active)
	{
		int32 Mask = 0;
		for (const int32 I : Active) { if (I >= 0 && I < TakeKeyLockCore::MaxFragments) { Mask |= (1 << I); } }
		return Mask;
	}

	TakeKeyLockCore::FLock ToCore(const FTakeKeyLock& In)
	{
		TakeKeyLockCore::FLock L;
		L.KeyIndex = In.KeyIndex;
		L.Bits = In.Bits;
		L.Target = In.Target;
		L.NumFragments = FMath::Min(In.Fragments.Num(), TakeKeyLockCore::MaxFragments);
		for (int32 I = 0; I < L.NumFragments; ++I)
		{
			L.Fragments[I] = In.Fragments[I];
			L.IsDecoy[I] = In.IsDecoy.IsValidIndex(I) ? In.IsDecoy[I] : false;
		}
		return L;
	}
}

FTakeKeyLock UTakeKeyLockLibrary::GenerateKeyLock(int32 Seed, int32 KeyIndex, int32 SightLevel)
{
	FUnrealRng Rng(Seed * 1000 + KeyIndex);
	const TakeKeyLockCore::FLock L = TakeKeyLockCore::Generate(KeyIndex, SightLevel, Rng);

	FTakeKeyLock Out;
	Out.KeyIndex = L.KeyIndex;
	Out.Bits = L.Bits;
	Out.Target = L.Target;
	for (int32 I = 0; I < L.NumFragments; ++I)
	{
		Out.Fragments.Add(L.Fragments[I]);
		Out.IsDecoy.Add(L.IsDecoy[I]);
		Out.HiddenMasks.Add(L.HiddenMask[I]);
	}
	for (int32 I = 0; I < L.NumAnswer; ++I) { Out.Answer.Add(L.Answer[I]); }
	return Out;
}

FTakeKeyLock UTakeKeyLockLibrary::GenerateKeyLockForRun(const UObject* WorldContextObject, int32 KeyIndex, int32 SightLevel)
{
	int32 Seed = 4471;
	if (const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject)))
	{
		Seed = GI->RunSeed;
	}
	return GenerateKeyLock(Seed, KeyIndex, SightLevel);
}

ETakeLockResult UTakeKeyLockLibrary::EvaluateKeyLock(const FTakeKeyLock& Lock, const TArray<int32>& ActiveFragments)
{
	switch (TakeKeyLockCore::Evaluate(ToCore(Lock), ToMask(ActiveFragments)))
	{
	case TakeKeyLockCore::EResult::Solved:    return ETakeLockResult::Solved;
	case TakeKeyLockCore::EResult::Corrupted: return ETakeLockResult::Corrupted;
	default:                                  return ETakeLockResult::Wrong;
	}
}

int32 UTakeKeyLockLibrary::CurrentResult(const FTakeKeyLock& Lock, const TArray<int32>& ActiveFragments)
{
	return TakeKeyLockCore::XorOf(ToCore(Lock), ToMask(ActiveFragments));
}

TArray<bool> UTakeKeyLockLibrary::SightCheck(const FTakeKeyLock& Lock, const TArray<int32>& ActiveFragments)
{
	const int32 Match = TakeKeyLockCore::SightCheckMask(ToCore(Lock), ToMask(ActiveFragments));
	TArray<bool> Out;
	for (int32 B = 0; B < Lock.Bits; ++B) { Out.Add(((Match >> B) & 1) != 0); }
	return Out;
}

bool UTakeKeyLockLibrary::IsGlyphHidden(const FTakeKeyLock& Lock, int32 FragmentIndex, int32 Glyph)
{
	return Lock.HiddenMasks.IsValidIndex(FragmentIndex) && ((Lock.HiddenMasks[FragmentIndex] >> Glyph) & 1) != 0;
}

FString UTakeKeyLockLibrary::GlyphRowString(int32 Value, int32 Bits, int32 HiddenMask)
{
	FString S;
	for (int32 B = Bits - 1; B >= 0; --B)
	{
		S.AppendChar(((HiddenMask >> B) & 1) ? TEXT('?') : (((Value >> B) & 1) ? TEXT('#') : TEXT('.')));
	}
	return S;
}
