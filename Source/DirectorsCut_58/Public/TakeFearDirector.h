// TAKE 4471 - the single Fear value (0..1) and everything it drives.
// Drag one into the level. Set Fear in the Details panel to preview the rot in the editor (no Play needed).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TakeFearDirector.generated.h"

class APostProcessVolume;
class AExponentialHeightFog;
class UMaterialParameterCollection;
class ATakeLoopTrigger;
class ATakeStalker;
class ALight;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeFearStageChanged, int32, NewStage);

UCLASS()
class DIRECTORSCUT_58_API ATakeFearDirector : public AActor
{
	GENERATED_BODY()

public:
	ATakeFearDirector();

	/** 0 = bright HALCYON office, 1 = full Augmented Fear. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Interp, Category = "Fear", meta = (ClampMin = "0", ClampMax = "1", UIMin = "0", UIMax = "1"))
	float Fear = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fear", meta = (ClampMin = "0", ClampMax = "1"))
	float TargetFear = 0.f;

	/** Fear units per second while creeping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear")
	float CreepSpeed = 0.05f;

	/** Stage thresholds: off by a little / draining / watched / stalked. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fear")
	TArray<float> StageThresholds = { 0.25f, 0.5f, 0.75f, 0.9f };

	UPROPERTY(BlueprintReadOnly, Category = "Fear") int32 Stage = 0;

	/** Outside a scripted sequence (Corridor B, the ward, the wrap party), Fear fades back by itself: it starts
	 *  DecayDelay seconds after the last thing that scared you and takes about 1/DecayPerSecond seconds from full. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear") float DecayDelay = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fear") float DecayPerSecond = 1.f / 90.f;
	double LastScareTime = -100.0;

	/** 0..1, how far the cinema bars have closed (starts at Fear 0.75). Read it from the letterbox widget. */
	UPROPERTY(BlueprintReadOnly, Category = "Fear") float LetterboxAmount = 0.f;

	UPROPERTY(BlueprintAssignable, Category = "Fear") FTakeFearStageChanged OnFearStageChanged;

	// ---- What Fear drives (all optional) ----
	UPROPERTY(EditAnywhere, Category = "Fear|Targets") TObjectPtr<UMaterialParameterCollection> FearCollection;   // MPC_Fear
	UPROPERTY(EditAnywhere, Category = "Fear|Targets") TObjectPtr<APostProcessVolume> FearVolume;                // GB_PPV_Fear
	UPROPERTY(EditAnywhere, Category = "Fear|Targets") TObjectPtr<AExponentialHeightFog> Fog;
	/** Loop triggers switch on at LoopFear (fog hides the teleport by then). */
	UPROPERTY(EditAnywhere, Category = "Fear|Targets") TArray<TObjectPtr<ATakeLoopTrigger>> LoopTriggers;
	UPROPERTY(EditAnywhere, Category = "Fear|Targets") float LoopFear = 0.5f;
	/** The loop only switches on during the Corridor B beat (ETakeBeat::Corridor); during that beat it needs this much Fear. */
	UPROPERTY(EditAnywhere, Category = "Fear|Targets") float BeatLoopFear = 0.3f;
	/** Fade in from black at Play (the cold open's "TAKE n" slate sits on top of it). */
	UPROPERTY(EditAnywhere, Category = "Fear|Targets") bool bColdOpenFade = true;
	/** The stalker activates at the last stage. */
	UPROPERTY(EditAnywhere, Category = "Fear|Targets") TObjectPtr<ATakeStalker> Stalker;
	/** Find MPC_Fear, the Fear post-process volume, fog, loop triggers, the stalker and FearLight-tagged lights automatically. */
	UPROPERTY(EditAnywhere, Category = "Fear|Targets") bool bAutoWire = true;
	/** Add a Take Grab component to the player pawn at Play if it has none. */
	UPROPERTY(EditAnywhere, Category = "Fear|Targets") bool bGivePlayerGrab = true;
	/** Lights that dim and flicker as Fear rises. */
	UPROPERTY(EditAnywhere, Category = "Fear|Targets") TArray<TObjectPtr<ALight>> FearLights;

	// ---- Look ----
	UPROPERTY(EditAnywhere, Category = "Fear|Look") float MinSaturation = 0.3f;
	UPROPERTY(EditAnywhere, Category = "Fear|Look") float MaxVignette = 1.2f;
	UPROPERTY(EditAnywhere, Category = "Fear|Look") float MaxGrain = 0.6f;
	UPROPERTY(EditAnywhere, Category = "Fear|Look") float FogDensityCalm = 0.02f;
	UPROPERTY(EditAnywhere, Category = "Fear|Look") float FogDensityFear = 0.25f;
	UPROPERTY(EditAnywhere, Category = "Fear|Look") FLinearColor FearTint = FLinearColor(0.85f, 0.95f, 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Fear") void StartCreep(float Target, float Seconds);
	UFUNCTION(BlueprintCallable, Category = "Fear") void Calm(float Seconds = 3.f) { StartCreep(0.f, Seconds); }
	UFUNCTION(BlueprintCallable, Category = "Fear") void SetFearImmediate(float Value);
	UFUNCTION(BlueprintCallable, Category = "Fear") void ApplyFear();
	/** Make the FearLight lights flicker hard for a moment, whatever the Fear level (the 10:00 meeting slip). */
	UFUNCTION(BlueprintCallable, Category = "Fear") void FlickerBurst(float Seconds = 2.5f);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }   // live preview in the editor
	virtual void OnConstruction(const FTransform& Transform) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& Event) override;
#endif

private:
	TMap<TObjectPtr<ALight>, float> BaseIntensity;
	TMap<TObjectPtr<ALight>, FLinearColor> BaseColor;
	float RedAlert = 0.f;   // 0..1: the lights go red while the stalker is out
public:
	bool bForceRed = false;   // the refusal ending: red without a stalker
	float LightScale = 1.f;   // house lights (the wrap party dims them)
	FBox DimBox = FBox(ForceInit);   // lights inside this box get DimScale on top (the open office after hours)
	float DimScale = 1.f;
private:
	float FlickerClock = 0.f;
	float BurstLeft = 0.f;
	bool LoopAllowed(float& OutNeed) const;
	void UpdateStage();
	void AutoWire();
	void EnsurePlayerCanGrab();
};
