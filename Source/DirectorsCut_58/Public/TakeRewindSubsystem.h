// TAKE 4471 - "CUT. AGAIN." Fade out, take number +1, Fear reset, stalker back to dormant,
// player back at desk #4471 (PlayerStart tagged Desk4471), one object changed, fade in.
// Bind OnRewind in your HUD/slate widget to show "CUT. AGAIN. / TAKE N".
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TakeRewindSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeRewind, int32, NewTakeNumber);

UCLASS()
class DIRECTORSCUT_58_API UTakeRewindSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Take") FTakeRewind OnRewind;

	/** Seconds of black while the slate shows. */
	UPROPERTY(BlueprintReadWrite, Category = "Take") float SlateSeconds = 1.6f;

	UFUNCTION(BlueprintCallable, Category = "Take") void Rewind();
	UFUNCTION(BlueprintPure, Category = "Take") bool IsRewinding() const { return bRewinding; }

private:
	bool bRewinding = false;
	void DoReset();
	void FinishRewind();
};
