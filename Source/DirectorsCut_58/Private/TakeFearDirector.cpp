// TAKE 4471 - Fear Director
#include "TakeFearDirector.h"
#include "DirectorsCut_58.h"
#include "TakeLoopTrigger.h"
#include "TakeStalker.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/Light.h"
#include "Components/LightComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TakeGrabComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "TakeGameInstance.h"
#include "Camera/PlayerCameraManager.h"

ATakeFearDirector::ATakeFearDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ATakeFearDirector::AutoWire()
{
	if (!bAutoWire || !GetWorld()) { return; }
	if (!FearCollection)
	{
		FearCollection = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/Fear/MPC_Fear.MPC_Fear"));
	}
	if (!FearVolume)
	{
		for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It) { if (It->bUnbound) { FearVolume = *It; break; } }
	}
	if (!Fog)
	{
		for (TActorIterator<AExponentialHeightFog> It(GetWorld()); It; ++It) { Fog = *It; break; }
	}
	if (LoopTriggers.Num() == 0)
	{
		for (TActorIterator<ATakeLoopTrigger> It(GetWorld()); It; ++It) { LoopTriggers.Add(*It); }
	}
	if (!Stalker)
	{
		for (TActorIterator<ATakeStalker> It(GetWorld()); It; ++It) { Stalker = *It; break; }
	}
	if (FearLights.Num() == 0)
	{
		for (TActorIterator<ALight> It(GetWorld()); It; ++It) { if (It->ActorHasTag(TEXT("FearLight"))) { FearLights.Add(*It); } }
	}
}

void ATakeFearDirector::EnsurePlayerCanGrab()
{
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Pawn) { return; }
	if (!Pawn->FindComponentByClass<UTakeGrabComponent>())
	{
		UTakeGrabComponent* Grab = NewObject<UTakeGrabComponent>(Pawn, TEXT("TakeGrab"));
		Grab->RegisterComponent();
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Added Take Grab to %s"), *Pawn->GetName());
	}
}

void ATakeFearDirector::BeginPlay()
{
	Super::BeginPlay();
	AutoWire();
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Fear Director: MPC %s, volume %s, fog %s, %d loop(s), stalker %s, %d light(s)"),
	       FearCollection ? TEXT("ok") : TEXT("MISSING"), FearVolume ? TEXT("ok") : TEXT("MISSING"), Fog ? TEXT("ok") : TEXT("none"),
	       LoopTriggers.Num(), Stalker ? TEXT("ok") : TEXT("none"), FearLights.Num());
	if (bGivePlayerGrab)
	{
		FTimerHandle T;   // the player pawn may not be spawned yet
		GetWorld()->GetTimerManager().SetTimer(T, this, &ATakeFearDirector::EnsurePlayerCanGrab, 0.3f, false);
	}
	SetFearImmediate(Fear);
	if (bColdOpenFade)
	{
		if (APlayerCameraManager* PCM = UGameplayStatics::GetPlayerCameraManager(this, 0))
		{
			PCM->StartCameraFade(1.f, 0.f, 3.f, FLinearColor::Black, false, false);
		}
	}
}

void ATakeFearDirector::FlickerBurst(float Seconds)
{
	BurstLeft = FMath::Max(BurstLeft, Seconds);
}

bool ATakeFearDirector::LoopAllowed(float& OutNeed) const
{
	OutNeed = LoopFear;
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI) { return true; }   // no story state (editor preview / other game instance): old behaviour
	if (GI->bDevForceLoop) { OutNeed = 0.f; return true; }
	OutNeed = BeatLoopFear;
	return GI->Beat == ETakeBeat::Corridor;
}

void ATakeFearDirector::StartCreep(float Target, float Seconds)
{
	TargetFear = FMath::Clamp(Target, 0.f, 1.f);
	CreepSpeed = FMath::Abs(TargetFear - Fear) / FMath::Max(Seconds, 0.01f);
}

void ATakeFearDirector::SetFearImmediate(float Value)
{
	Fear = TargetFear = FMath::Clamp(Value, 0.f, 1.f);
	ApplyFear();
	UpdateStage();
}

void ATakeFearDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const UWorld* World = GetWorld();
	const bool bPlaying = World && World->IsGameWorld();
	if (bPlaying)
	{
		Fear = FMath::FInterpConstantTo(Fear, TargetFear, DeltaSeconds, CreepSpeed);
		BurstLeft = FMath::Max(0.f, BurstLeft - DeltaSeconds);
		UpdateStage();
	}
	FlickerClock += DeltaSeconds;
	ApplyFear();
}

void ATakeFearDirector::UpdateStage()
{
	int32 NewStage = 0;
	for (const float T : StageThresholds) { if (Fear >= T) { ++NewStage; } }
	if (NewStage != Stage)
	{
		const int32 Old = Stage;
		Stage = NewStage;
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Fear stage %d -> %d (Fear %.2f)"), Old, NewStage, Fear);
		OnFearStageChanged.Broadcast(Stage);
		if (Stalker && Old < StageThresholds.Num() && NewStage >= StageThresholds.Num()) { Stalker->Activate(); }
	}
	float Need = LoopFear;
	const bool bAllowed = LoopAllowed(Need);
	for (ATakeLoopTrigger* Loop : LoopTriggers) { if (Loop) { Loop->SetLoopEnabled(bAllowed && Fear >= Need); } }
}

void ATakeFearDirector::ApplyFear()
{
	const float F = FMath::Clamp(Fear, 0.f, 1.f);
	LetterboxAmount = FMath::GetMappedRangeValueClamped(FVector2D(0.75f, 1.f), FVector2D(0.f, 1.f), F);

	if (FearCollection && GetWorld())
	{
		UKismetMaterialLibrary::SetScalarParameterValue(this, FearCollection, TEXT("Fear"), F);
	}
	if (FearVolume)
	{
		FPostProcessSettings& S = FearVolume->Settings;
		const float Sat = FMath::Lerp(1.f, MinSaturation, F);
		S.bOverride_ColorSaturation = true;   S.ColorSaturation = FVector4(Sat, Sat, Sat, 1.f);
		S.bOverride_VignetteIntensity = true; S.VignetteIntensity = FMath::Lerp(0.4f, MaxVignette, F);
		S.bOverride_FilmGrainIntensity = true; S.FilmGrainIntensity = FMath::Lerp(0.f, MaxGrain, F);
		S.bOverride_SceneColorTint = true;    S.SceneColorTint = FMath::Lerp(FLinearColor::White, FearTint, F);
	}
	if (Fog && Fog->GetComponent())
	{
		Fog->GetComponent()->SetFogDensity(FMath::Lerp(FogDensityCalm, FogDensityFear, F));
	}
	// Lights only change during Play, so editor previews never save dimmed intensities into the level.
	const bool bGame = GetWorld() && GetWorld()->IsGameWorld();
	for (ALight* L : FearLights)
	{
		if (!bGame) { break; }
		if (!L || !L->GetLightComponent()) { continue; }
		float& Base = BaseIntensity.FindOrAdd(L, L->GetLightComponent()->Intensity);
		// dim with Fear; above 0.5, flicker irregularly
		float Scale = FMath::Lerp(1.f, 0.25f, F);
		const bool bBurst = BurstLeft > 0.f;
		if (F > 0.5f || bBurst)
		{
			const float N = FMath::Sin(FlickerClock * 23.f) * FMath::Sin(FlickerClock * 7.3f + 1.3f);
			const float Threshold = bBurst ? 0.15f : 0.85f - (F - 0.5f);
			if (N > Threshold) { Scale *= 0.15f; }
		}
		L->GetLightComponent()->SetIntensity(Base * Scale);
	}
}

void ATakeFearDirector::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (bAutoWire && !FearCollection) { FearCollection = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/Fear/MPC_Fear.MPC_Fear")); }
	if (bAutoWire && !FearVolume && GetWorld())
	{
		for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It) { if (It->bUnbound) { FearVolume = *It; break; } }
	}
	ApplyFear();
}

#if WITH_EDITOR
void ATakeFearDirector::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);
	TargetFear = Fear;
	ApplyFear();
}
#endif
