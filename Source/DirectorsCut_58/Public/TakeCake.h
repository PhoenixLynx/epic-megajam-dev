// TAKE 4471 - the cake. A real round cake made of wedges (a procedural mesh), so it can actually be cut.
//
//   Each wedge has three parts: the top (Sol's ART-011 "THAT'S A WRAP, 4471", mapped flat across the whole cake),
//   the frosted outside, and the two cut faces (sponge). Cutting a slice hides the next wedge: the cake really gets
//   smaller, the sponge shows in the gap, and the text on top is cut through.
//   A single slice on a plate is the same class with one wedge (MakeSlice).
//
// Spawned from code (UTakeGrabComponent::SpawnCake): the wrap party table, the director's floor, the kitchen in overtime.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TakeCake.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;

UCLASS()
class DIRECTORSCUT_58_API ATakeCake : public AActor
{
	GENERATED_BODY()

public:
	ATakeCake();

	UPROPERTY(VisibleAnywhere, Category = "Cake") TObjectPtr<UProceduralMeshComponent> Cake;

	/** Build the whole cake: Slices wedges of radius Radius and height Height (cm), with Missing wedges already gone. */
	void BuildCake(float InRadius, float InHeight, int32 InSlices, int32 Missing);
	/** Build one wedge (index Wedge of Slices), moved so its middle sits on this actor's origin: a slice on a plate. */
	void BuildSlice(float InRadius, float InHeight, int32 InSlices, int32 Wedge);

	/** Take the next wedge out. Returns false if there's nothing left. OutWedge = which one, OutDir = which way it faces (world). */
	bool CutSlice(int32& OutWedge, FVector& OutDir);
	int32 SlicesLeft() const;

	float Radius = 28.f;
	float Height = 16.f;
	int32 Slices = 8;
	int32 StartMissing = 0;
	TWeakObjectPtr<AActor> ServePlate;    // the one plate slices from this cake go on
	TWeakObjectPtr<AActor> ServedSlice;   // the slice on it you haven't eaten yet
	/** How many slices have been cut from this cake so far (not counting the ones already gone when it was built). */
	int32 CutsSoFar() const { return Slices - SlicesLeft() - StartMissing; }
	/** Until the first cut, keep the lettering turned toward the player while they're more than ~4 m away, so it reads
	 *  from whichever side they walk up to (playtest: the party and penthouse cakes faced the wrong way). */
	void AimAtPlayer();
	virtual void Tick(float DeltaSeconds) override;

private:
	TArray<bool> Present;
	void AddWedge(int32 Wedge, const FVector& Offset, int32 SectionBase);
	UMaterialInterface* TopMat() const;
	UMaterialInterface* SideMat() const;
	UMaterialInterface* SpongeMat() const;
	UPROPERTY() TObjectPtr<UMaterialInterface> CachedSide;
};
