// TAKE 4471 - an invisible box that tells the player's Take Grab component "you walked into <BeatName>".
// The grab component decides what that means for the story (e.g. ConferenceRoom during the 10:00 meeting
// beat = the slip; CorridorB during the corridor beat = the PA and the creep). place_cpp_actors.py places them.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TakeBeatTrigger.generated.h"

class UBoxComponent;

UCLASS()
class DIRECTORSCUT_58_API ATakeBeatTrigger : public AActor
{
	GENERATED_BODY()

public:
	ATakeBeatTrigger();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beat") TObjectPtr<UBoxComponent> Box;
	/** ConferenceRoom, CorridorB, ... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beat") FName BeatName;
	/** Half size of the box (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beat") FVector BoxHalfSize = FVector(200.f, 200.f, 150.f);

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

private:
	UFUNCTION() void OnBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp,
	                              int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);
};
