// TAKE 4471 - XOR Key Lock for Blueprints (wraps TakeKeyLockCore.h). Spec: docs/KEYLOCK_SPEC.md
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TakeKeyLockLibrary.generated.h"

UENUM(BlueprintType)
enum class ETakeLockResult : uint8
{
	Wrong,
	Solved,
	Corrupted   // a malware decoy fragment was active
};

USTRUCT(BlueprintType)
struct FTakeKeyLock
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "KeyLock") int32 KeyIndex = 1;
	UPROPERTY(BlueprintReadOnly, Category = "KeyLock") int32 Bits = 6;
	/** The signature to match. Glyph b is lit when (Target >> b) & 1. */
	UPROPERTY(BlueprintReadOnly, Category = "KeyLock") int32 Target = 0;
	UPROPERTY(BlueprintReadOnly, Category = "KeyLock") TArray<int32> Fragments;
	UPROPERTY(BlueprintReadOnly, Category = "KeyLock") TArray<bool> IsDecoy;
	/** One valid solution (debug only, never show it). */
	UPROPERTY(BlueprintReadOnly, Category = "KeyLock") TArray<int32> Answer;
	/** Per fragment: bit b set = glyph b shows as '?'. */
	UPROPERTY(BlueprintReadOnly, Category = "KeyLock") TArray<int32> HiddenMasks;
};

UCLASS()
class DIRECTORSCUT_58_API UTakeKeyLockLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Build a solvable lock. KeyIndex 1 = tutorial, 2 = Supply Closet, 3 = Hospital. Same seed + key = same lock. */
	UFUNCTION(BlueprintCallable, Category = "Take|KeyLock")
	static FTakeKeyLock GenerateKeyLock(int32 Seed, int32 KeyIndex, int32 SightLevel);

	/** Generate using the run seed from the game instance (unique every run). */
	UFUNCTION(BlueprintCallable, Category = "Take|KeyLock", meta = (WorldContext = "WorldContextObject"))
	static FTakeKeyLock GenerateKeyLockForRun(const UObject* WorldContextObject, int32 KeyIndex, int32 SightLevel);

	UFUNCTION(BlueprintPure, Category = "Take|KeyLock")
	static ETakeLockResult EvaluateKeyLock(const FTakeKeyLock& Lock, const TArray<int32>& ActiveFragments);

	/** XOR of the active fragments (the 'running result' - only show it on the tutorial key). */
	UFUNCTION(BlueprintPure, Category = "Take|KeyLock")
	static int32 CurrentResult(const FTakeKeyLock& Lock, const TArray<int32>& ActiveFragments);

	/** Soul Sight check: per glyph, does the current result match the target? (costs Sight in game) */
	UFUNCTION(BlueprintPure, Category = "Take|KeyLock")
	static TArray<bool> SightCheck(const FTakeKeyLock& Lock, const TArray<int32>& ActiveFragments);

	UFUNCTION(BlueprintPure, Category = "Take|KeyLock")
	static bool IsGlyphLit(int32 Value, int32 Glyph) { return ((Value >> Glyph) & 1) != 0; }

	UFUNCTION(BlueprintPure, Category = "Take|KeyLock")
	static bool IsGlyphHidden(const FTakeKeyLock& Lock, int32 FragmentIndex, int32 Glyph);

	/** Text preview like "#.##..#." (lit #, unlit ., hidden ?) for debugging. */
	UFUNCTION(BlueprintPure, Category = "Take|KeyLock")
	static FString GlyphRowString(int32 Value, int32 Bits, int32 HiddenMask = 0);
};
