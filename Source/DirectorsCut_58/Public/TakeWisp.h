// TAKE 4471 - a glowing wisp of soul energy (beat 1.8a: the end of Corridor B).
// An emissive sphere + a point light, bobbing and pulsing. Only there during its story beat.
// E picks it up (UTakeGrabComponent handles the story side: Fear drains, objective changes).
// place_cpp_actors.py puts one on GB_SoulEnergy_Act1.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TakeGameInstance.h"
#include "TakeWisp.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class UMaterialInterface;

UCLASS()
class DIRECTORSCUT_58_API ATakeWisp : public AActor
{
	GENERATED_BODY()

public:
	ATakeWisp();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wisp") TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wisp") TObjectPtr<UStaticMeshComponent> Orb;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wisp") TObjectPtr<UPointLightComponent> Glow;

	/** Glowing material for the orb (place_cpp_actors.py makes /Game/Fear/M_Wisp). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wisp") TObjectPtr<UMaterialInterface> OrbMaterial;
	/** The story beat during which the wisp is there. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wisp") ETakeBeat VisibleDuring = ETakeBeat::Corridor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wisp") float OrbScale = 0.28f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wisp") float BobHeight = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wisp") float BobSpeed = 1.7f;
	/** Point light brightness in lumens. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wisp") float GlowLumens = 900.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wisp") FLinearColor GlowColor = FLinearColor(0.50f, 0.95f, 0.85f);

	/** Here and not picked up yet (during its beat). */
	UFUNCTION(BlueprintPure, Category = "Wisp") bool IsAvailable() const;
	/** Hide it for good (the grab component calls this when you pick it up). */
	UFUNCTION(BlueprintCallable, Category = "Wisp") void Collect();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	bool bCollected = false;
	bool bShown = true;
	float Clock = 0.f;
	void ApplyShown(bool bNewShown);
};
