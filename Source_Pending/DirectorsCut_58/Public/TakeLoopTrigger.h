// TAKE 4471 - Corridor B loop. Walk past the box -> silently moved back by LoopOffset into an
// identical stretch of corridor. Each loop, one poster swaps to its "_Changed" material.
// Interacting with the changed poster (call TryBreakLoop from your interact) breaks the loop.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TakeLoopTrigger.generated.h"

class UBoxComponent;
class AStaticMeshActor;
class UMaterialInterface;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeLooped, int32, LoopCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTakeLoopBroken);

UCLASS()
class DIRECTORSCUT_58_API ATakeLoopTrigger : public AActor
{
	GENERATED_BODY()

public:
	ATakeLoopTrigger();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loop") TObjectPtr<UBoxComponent> Box;

	/** Where the player is moved, relative to where they are (one corridor section back). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loop") FVector LoopOffset = FVector(0.f, -1000.f, 0.f);

	/** Off until the Fear Director turns it on (Fear >= 0.5) so fog hides the jump. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loop") bool bLoopEnabled = false;

	UPROPERTY(BlueprintReadOnly, Category = "Loop") int32 LoopCount = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Loop") bool bBroken = false;

	/** Posters in the looping stretch (GB_Poster_0..5). */
	UPROPERTY(EditAnywhere, Category = "Loop") TArray<TObjectPtr<AStaticMeshActor>> Posters;
	/** For each poster, the material with its one changed detail (T_Poster_*_Changed). */
	UPROPERTY(EditAnywhere, Category = "Loop") TArray<TObjectPtr<UMaterialInterface>> ChangedMaterials;

	UPROPERTY(BlueprintReadOnly, Category = "Loop") TObjectPtr<AStaticMeshActor> CurrentChangedPoster;

	UPROPERTY(BlueprintAssignable, Category = "Loop") FTakeLooped OnLooped;
	UPROPERTY(BlueprintAssignable, Category = "Loop") FTakeLoopBroken OnLoopBroken;

	UFUNCTION(BlueprintCallable, Category = "Loop") void SetLoopEnabled(bool bEnable);
	/** Call when the player interacts with a poster. Returns true if it was the changed one (loop broken). */
	UFUNCTION(BlueprintCallable, Category = "Loop") bool TryBreakLoop(AActor* InteractedPoster);
	UFUNCTION(BlueprintCallable, Category = "Loop") void ResetLoop();

	virtual void BeginPlay() override;

private:
	UFUNCTION() void OnBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp,
	                              int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);
	void ChangeOnePoster();
	void RestorePosters();
	UPROPERTY() TArray<TObjectPtr<UMaterialInterface>> OriginalMaterials;
};
