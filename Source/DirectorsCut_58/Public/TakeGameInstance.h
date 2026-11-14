// TAKE 4471 - run state that survives level loads: seed, take number, soul bits.
// Also hosts console commands for testing (press ` in Play, type e.g. TakeFear 0.8).
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TakeGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeSoulBitsChanged, int32, NewSoulBits);

UCLASS()
class DIRECTORSCUT_58_API UTakeGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	/** Unique per run, never shown. Drives puzzles, NPC agents, clue placement. */
	UPROPERTY(BlueprintReadOnly, Category = "Take") int32 RunSeed = 0;

	/** Dev only: if >= 0, used instead of the clock so bugs can be reproduced. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Take|Dev") int32 DevSeedOverride = -1;

	/** Goes up every time the stalker catches you ("CUT. AGAIN."). */
	UPROPERTY(BlueprintReadWrite, Category = "Take") int32 TakeNumber = 1;

	/** Recovered soul bits (0..3). Each one = an access pass, stronger Soul Sight, more astral logic. */
	UPROPERTY(BlueprintReadOnly, Category = "Take") int32 SoulBits = 0;

	UPROPERTY(BlueprintAssignable, Category = "Take") FTakeSoulBitsChanged OnSoulBitsChanged;

	UFUNCTION(BlueprintCallable, Category = "Take") void SetSoulBits(int32 NewValue);
	UFUNCTION(BlueprintCallable, Category = "Take") void AddSoulBit() { SetSoulBits(SoulBits + 1); }

	/** Compliance Training: current slide, and whether Module 1 (the Key 1 tutorial) is done. */
	UPROPERTY(BlueprintReadWrite, Category = "Take") int32 TrainingSlide = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Take") bool bTrainingComplete = false;

	/** Access pass N (1..3) is held once you have N soul bits. */
	UFUNCTION(BlueprintPure, Category = "Take") bool HasPass(int32 PassNumber) const { return SoulBits >= PassNumber; }

	// ---- Console commands (testing) ----
	UFUNCTION(Exec) void TakeFear(float Value);          // set Fear instantly (0..1)
	UFUNCTION(Exec) void TakeCreep(float Target, float Seconds = 10.f);
	UFUNCTION(Exec) void TakeSoulBits(int32 Value);
	UFUNCTION(Exec) void TakeStalker();                  // activate the stalker now
	UFUNCTION(Exec) void TakeRewind();                   // "CUT. AGAIN."
	UFUNCTION(Exec) void TakeLoop(bool bEnable);         // force the Corridor B loop on/off
	UFUNCTION(Exec) void TakeSeed();                     // print the run seed + Key 1..3 previews to the log
};
