// TAKE 4471 - text placeholders
#include "TakeTypes.h"
#include "TakeGameInstance.h"
#include "TakeKeyLockLibrary.h"
#include "Kismet/GameplayStatics.h"

FString TakeText::Fill(const UObject* WorldContext, const FString& In)
{
	FString Out = In;
	const UTakeGameInstance* GI = WorldContext ? Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(WorldContext)) : nullptr;
	Out.ReplaceInline(TEXT("{EMP}"), TEXT("4471"));
	Out.ReplaceInline(TEXT("{TAKE}"), *FString::FromInt(GI ? GI->TakeNumber : 1));
	Out.ReplaceInline(TEXT("{HITS}"), *FString::FromInt(GI ? FMath::Max(1, GI->CoworkersHit) : 1));
	// Glyph clues: pieces of this run's Key 2 / Key 3 signatures, drawn as filled/empty marks
	const int32 Seed = GI ? GI->RunSeed : 4471;
	const FTakeKeyLock K2 = UTakeKeyLockLibrary::GenerateKeyLock(Seed, 2, 0);
	const FTakeKeyLock K3 = UTakeKeyLockLibrary::GenerateKeyLock(Seed, 3, 0);
	auto Marks = [](int32 Value, int32 From, int32 To)
	{
		FString S;
		for (int32 B = From; B >= To; --B) { S += ((Value >> B) & 1) ? TEXT("\u25A0 ") : TEXT("\u25A1 "); }
		return S.TrimEnd();
	};
	Out.ReplaceInline(TEXT("{GLYPHS_A}"), *Marks(K2.Target, 7, 4));
	Out.ReplaceInline(TEXT("{GLYPHS_B}"), *Marks(K2.Target, 3, 0));
	Out.ReplaceInline(TEXT("{GLYPHS_C}"), *Marks(K3.Target, 7, 0));
	return Out;
}

int32 TakeSettings::DialogueMode = 0;
