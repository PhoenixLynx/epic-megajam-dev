// TAKE 4471 - Act 3 (overtime), the ending (the wrap party) and the title screen. Part of UTakeGrabComponent.
//   3.0  Act 2 done + the clocks stopped -> SCENE 4 - OVERTIME: "Mandatory team building in the Wellness Center."
//   3.1  pass 2 opens the Wellness Center; coworkers stand in a circle; E on the X = fall backward
//   3.2  nobody catches you -> SCENE 5 - THE WARD (the hospital below); Fear stays high; capture rewinds to the ward door
//   3.3  charts are script pages; two carry Key 3's squares, one is a decoy (4470's, different handwriting)
//   3.5  the nurses' station terminal (Key Lock 3, signature hidden) -> pass 3
//   3.6  the service elevator -> SCENE 6 - C: you're back in the elevator; C is lit
//   E    press C -> WRAP PARTY: everyone in the lobby, applause, paper confetti, the Collector, YES / NO, an end card, credits
#include "TakeGrabComponent.h"
#include "DirectorsCut_58.h"
#include "TakeGameInstance.h"
#include "TakeFearDirector.h"
#include "TakeStalker.h"
#include "TakeTypes.h"
#include "TakeCoworker.h"
#include "TakeWidgets.h"
#include "TakeAct3Widgets.h"
#include "TakePauseMenu.h"
#include "TakeSaveGame.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
	const FVector WardLanding(1400.f, 6400.f, -1400.f);    // HospitalWard marker (floor at -1500)

	// Things to fight back with in the ward (the stalker is stunned by anything tagged Throwable).
	// Floor items sit in the aisles between bed rows (y 5900-6350 and 6550-7000); "on a bed" ones drop onto the mattress.
	struct FWardProp { const TCHAR* Mesh; FVector Where; };
	const FVector ElevatorCar(2100.f, 250.f, 100.f);
	const FVector LobbyParty(2100.f, 1050.f, 100.f);
}

void UTakeGrabComponent::TeleportPlayer(const FVector& Where, float Yaw)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) { return; }
	Drop();
	Pawn->SetActorLocation(Where, false, nullptr, ETeleportType::ResetPhysics);
	if (AController* C = Pawn->GetController()) { C->SetControlRotation(FRotator(0.f, Yaw, 0.f)); }
}

// ------------------------------------------------------------------ Act 3
void UTakeGrabComponent::StartOvertime()
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI) { return; }
	GI->Beat = ETakeBeat::Overtime;
	GI->bDevAct3Now = false;
	OvertimeIn = -1.f;
	if (!bForeshadowDone) { ForeshadowIn = 35.f; }   // a while into overtime, something walks through the office
	ShowTimeCard(TEXT("2:14 PM"), TEXT("SCENE 4  \u00B7  OVERTIME"));
	PlaySfx2D(TEXT("sfx_pa_chime"), 0.7f);
	QueueSubtitle(TEXT("PA"), TEXT("Mandatory team building in the Wellness Center. Attendance is mandatory."));
	QueueSubtitle(TEXT("PA"), TEXT("Attendance has always been mandatory."));
	if (ATakeFearDirector* FearDir = FindFearDirector()) { FearDir->StartCreep(FMath::Max(FearDir->Fear, 0.2f), 4.f); }
	GI->SetObjective(GI->ObjectiveForBeat());
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Beat -> Overtime (Act 3)"));
}

void UTakeGrabComponent::TickForeshadow(float DeltaTime)
{
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI || bForeshadowDone) { return; }
	if (GI->Beat != ETakeBeat::Overtime)   // the trust fall (or a dev jump) came first: call it off
	{
		if (ForeshadowT >= 0.f) { EndForeshadow(false); }
		ForeshadowIn = -1.f;
		return;
	}
	if (bSeated || Inspecting || PauseMenu || LockPanel || UGameplayStatics::IsGamePaused(this)) { return; }
	ATakeStalker* Stalker = nullptr;
	for (TActorIterator<ATakeStalker> It(GetWorld()); It; ++It) { Stalker = *It; break; }
	if (!Stalker) { return; }
	const FVector Me = GetOwner()->GetActorLocation();

	if (ForeshadowT < 0.f)
	{
		if (ForeshadowIn < 0.f || (ForeshadowIn -= DeltaTime) > 0.f) { return; }
		// Somewhere open, out of the way, far enough to hear before you see it (the spots coworkers walk to)
		static const FVector2D Spots[] = { FVector2D(2000.f, 2600.f), FVector2D(1730.f, 3400.f), FVector2D(1760.f, 1540.f),
			FVector2D(3300.f, 1760.f), FVector2D(3140.f, 650.f), FVector2D(3620.f, 1250.f) };
		FVector Start(Spots[0].X, Spots[0].Y, Me.Z);
		float Best = -1.f;
		for (const FVector2D& S : Spots)
		{
			const float D = FVector::Dist2D(FVector(S.X, S.Y, 0.f), Me);
			if (D >= 900.f && D <= 2600.f && D > Best) { Best = D; Start = FVector(S.X, S.Y, Me.Z); }
		}
		GetWorld()->FindTeleportSpot(Stalker, Start, Stalker->GetActorRotation());
		Stalker->SetActorLocation(Start, false, nullptr, ETeleportType::TeleportPhysics);
		Stalker->bSoftCatch = true;
		Stalker->bSoftCaught = false;
		Stalker->Activate();
		ForeshadowT = 0.f;
		if (ATakeFearDirector* FearDir = FindFearDirector()) { FearDir->FlickerBurst(2.5f); FearDir->StartCreep(FMath::Max(FearDir->Fear, 0.8f), 3.f); }
		PlaySfx2D(TEXT("sfx_fear_hit"), 0.9f);
		bTensionFirstDone = true;
		PlayMusicCue(TEXT("the first footsteps"), TEXT("mus_halcyon_tension"), true);
		ShowSubtitle(TEXT(""), TEXT("(Footsteps. Heavy. Not yours. Coming this way.)"), 3.5f);
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Foreshadow: the stalker walks the office (from %s)"), *Start.ToCompactString());
		return;
	}
	ForeshadowT += DeltaTime;
	if (Stalker->bSoftCaught) { EndForeshadow(true); return; }
	if (ForeshadowT > 22.f) { EndForeshadow(false); }   // you got away (this time)
}

void UTakeGrabComponent::EndForeshadow(bool bCaught)
{
	bForeshadowDone = true;
	ForeshadowT = -1.f;
	ForeshadowIn = -1.f;
	for (TActorIterator<ATakeStalker> It(GetWorld()); It; ++It)
	{
		It->bSoftCatch = false;
		It->bSoftCaught = false;
		It->Deactivate();
	}
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	const float Calm = GI && GI->bClocksFrozen ? 0.15f : 0.f;
	if (bCaught)
	{
		// A hard cut: black, a thud, and the office is just the office again
		PlaySfx2D(TEXT("sfx_impact_heavy"), 1.f);
		if (APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
			{
				if (PC->PlayerCameraManager) { PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, 1.6f, FLinearColor::Black, false, false); }
			}
		}
		if (ATakeFearDirector* FearDir = FindFearDirector()) { FearDir->SetFearImmediate(Calm); }
		ShowSubtitle(TEXT(""), TEXT("...What was that?"), 3.f);
		QueueSubtitle(TEXT(""), TEXT("Nobody looks up. The office hums like nothing happened."));
	}
	else
	{
		if (ATakeFearDirector* FearDir = FindFearDirector()) { FearDir->StartCreep(Calm, 5.f); }
		if (GI && GI->Beat == ETakeBeat::Overtime)
		{
			ShowSubtitle(TEXT(""), TEXT("(The footsteps stop. Just like that.)"), 3.f);
			QueueSubtitle(TEXT(""), TEXT("...What was that?"));
		}
	}
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Foreshadow over (%s)"), bCaught ? TEXT("caught") : TEXT("got away"));
}

void UTakeGrabComponent::StartTrustFall()
{
	if (FallT >= 0.f) { return; }
	FallT = 0.f;
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController())) { PC->SetIgnoreMoveInput(true); PC->SetIgnoreLookInput(true); }
	}
	ShowSubtitle(TEXT("Coworkers"), TEXT("Fall back, 4471. We'll catch you. We always catch you."), 2.5f);
}

void UTakeGrabComponent::EnterWard()
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI) { return; }
	GI->Beat = ETakeBeat::Ward;
	GI->bTrustFell = true;
	GI->RewindPoint = WardLanding;
	TeleportPlayer(WardLanding, 90.f);
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController())) { PC->SetIgnoreMoveInput(false); PC->SetIgnoreLookInput(false); }
	}
	PlaySfx2D(TEXT("sfx_impact_heavy"), 0.9f);
	if (ATakeFearDirector* FearDir = FindFearDirector()) { FearDir->StartCreep(0.65f, 3.f); }
	QueueSubtitle(TEXT(""), TEXT("...Nobody caught you."));
	QueueSubtitle(TEXT(""), TEXT("A hospital ward. Beds in rows, like cubicles. Every bed has a chart."));
	StalkerIn = 25.f;
	// Weapons in the ward, once per play (a rewind to the ward door keeps whatever is lying around)
	bool bHaveWardProps = false;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It) { if (It->ActorHasTag(TEXT("WardProp"))) { bHaveWardProps = true; break; } }
	if (!bHaveWardProps)
	{
		// (inside the function, not at file level: Live Coding doesn't update file-level constants)
		const FWardProp WardProps[] = {
			{ TEXT("SM_FireExtinguisher"), FVector(1550.f, 6250.f, -1450.f) },   // right by the landing
			{ TEXT("SM_TrashBin"),         FVector( 650.f, 6450.f, -1460.f) },
			{ TEXT("SM_MeetingChair"),     FVector(1650.f, 6780.f, -1450.f) },
			{ TEXT("SM_WetFloorSign"),     FVector(2050.f, 6800.f, -1450.f) },
			{ TEXT("SM_ReamOfPaper"),      FVector(1300.f, 5800.f, -1410.f) },   // on a bed
			{ TEXT("SM_Binder"),           FVector(1800.f, 6450.f, -1410.f) },   // on a bed
			{ TEXT("SM_WaterBottle"),      FVector(2300.f, 7100.f, -1410.f) },   // on a bed
			{ TEXT("SM_Mug"),              FVector(1100.f, 7020.f, -1450.f) },
			{ TEXT("SM_FireExtinguisher"), FVector(2750.f, 6250.f, -1450.f) },   // a second one, far side
			// things to drag into its path (chairs roll; signs and buckets are light)
			{ TEXT("SM_OfficeChair"),      FVector(1050.f, 6250.f, -1450.f) },
			{ TEXT("SM_OfficeChair"),      FVector(2550.f, 6780.f, -1450.f) },
			{ TEXT("SM_WetFloorSign"),     FVector(1300.f, 7350.f, -1450.f) },   // CAUTION
			{ TEXT("SM_MopBucket"),        FVector(2900.f, 7420.f, -1460.f) },
			// the north aisle, between the ward's north wall (y 5300) and the first bed row (y 5700-5900)
			{ TEXT("SM_Keyboard"),         FVector(1740.f, 5560.f, -1450.f) },
			{ TEXT("SM_FireExtinguisher"), FVector(1500.f, 5500.f, -1450.f) },
			{ TEXT("SM_MeetingChair"),     FVector( 950.f, 5560.f, -1450.f) },
			{ TEXT("SM_TrashBin"),         FVector(2250.f, 5480.f, -1460.f) },
			{ TEXT("SM_ReamOfPaper"),      FVector(2600.f, 5560.f, -1450.f) },
			{ TEXT("SM_Binder"),           FVector(2000.f, 5620.f, -1450.f) },
			{ TEXT("SM_Mug"),              FVector(2800.f, 5800.f, -1410.f) },   // on a bed
		};
		int32 Made = 0;
		for (const FWardProp& P : WardProps)
		{
			if (AActor* A = SpawnProp(P.Mesh, P.Where, FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f)))
			{
				A->Tags.Add(TEXT("WardProp"));
				Spawned.Remove(A);   // not part of the snack/mug recycling: these stay for the whole ward
				++Made;
				const UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(A->GetRootComponent());
				if (!Root || !Root->IsSimulatingPhysics())   // can't be picked up: its mesh still has complex-only collision
				{
					UE_LOG(LogTake, Warning, TEXT("[TAKE 4471] Ward: %s has no physics (run fix_prop_collision.py)"), P.Mesh);
				}
			}
		}
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Ward: %d throwable(s) placed"), Made);
		// The nurses' station gets a phone (calls in the ward ring here, not upstairs)
		if (UStaticMesh* PhoneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Props/Meshes/SM_DeskPhone.SM_DeskPhone")))
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			// On the nurses' station desk itself (it was floating beside it): the nearest desk to the station, its top
			// surface found by tracing down, near the corner closest to where the phone used to hang in the air
			FVector PhoneAt(2300.f, 7505.f, -1424.f);
			AActor* Desk = nullptr;
			float BestD = 600.f;
			for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
			{
				const UStaticMeshComponent* DSM = It->GetStaticMeshComponent();
				if (!DSM || !DSM->GetStaticMesh() || !DSM->GetStaticMesh()->GetName().Contains(TEXT("Desk")) || DSM->GetStaticMesh()->GetName().Contains(TEXT("Phone"))) { continue; }
				const float D = FVector::Dist(It->GetActorLocation(), PhoneAt);
				if (D < BestD) { BestD = D; Desk = *It; }
			}
			if (Desk)
			{
				FVector O, E;
				Desk->GetActorBounds(true, O, E);
				const FVector Corner(FMath::Clamp(PhoneAt.X, O.X - E.X + 20.f, O.X + E.X - 20.f), FMath::Clamp(PhoneAt.Y, O.Y - E.Y + 20.f, O.Y + E.Y - 20.f), O.Z + E.Z + 40.f);
				FHitResult Top;
				FCollisionQueryParams DQ(SCENE_QUERY_STAT(TakeWardPhone), false);
				PhoneAt = GetWorld()->LineTraceSingleByChannel(Top, Corner, Corner - FVector(0.f, 0.f, 200.f), ECC_Visibility, DQ) && Top.GetActor() == Desk
					? Top.ImpactPoint + FVector(0.f, 0.f, 0.5f) : FVector(Corner.X, Corner.Y, O.Z + E.Z + 0.5f);
				UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Ward phone on %s at %s"), *Desk->GetName(), *PhoneAt.ToCompactString());
			}
			if (AStaticMeshActor* Phone = GetWorld()->SpawnActor<AStaticMeshActor>(PhoneAt, FRotator(0.f, 180.f, 0.f), Params))
			{
				Phone->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
				Phone->GetStaticMeshComponent()->SetStaticMesh(PhoneMesh);
				Phone->Tags.Add(TEXT("WardPhone"));
				Phone->Tags.Add(TEXT("WardProp"));
				Phones.Add(Phone);
				WardRingIn = 8.f;   // its first call comes soon after you land
			}
		}
	}
	GI->SetObjective(GI->ObjectiveForBeat());
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Beat -> Ward"));
}

void UTakeGrabComponent::ExitWard()
{
	if (ExitT >= 0.f) { return; }
	ExitT = 0.f;
	PlaySfx2D(TEXT("sfx_badge_ok"), 0.7f);
	ShowTimeCard(TEXT("2:14 PM"), TEXT("SCENE 6  \u00B7  C"));
}

void UTakeGrabComponent::SkipAct3Step()
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI) { return; }
	switch (GI->Beat)
	{
	case ETakeBeat::Overtime: if (GI->SoulBits < 2) { GI->SetSoulBits(2); GI->SetObjective(GI->ObjectiveForBeat()); } else { StartTrustFall(); } break;
	case ETakeBeat::Ward:     if (GI->SoulBits < 3) { GI->SetSoulBits(3); GI->SetObjective(GI->ObjectiveForBeat()); } else { ExitWard(); } break;
	case ETakeBeat::Wrap:     StartEnding(); break;
	default: break;
	}
}

void UTakeGrabComponent::TickAct3(float DeltaTime)
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI || PauseMenu) { return; }

	// Checkpoints, so CONTINUE picks up at the start of the act you were in (playtest: "continue should reload the
	// latest save checkpoint, and it should be after each act for now"). One save each time the story arrives at an
	// act start: the morning (Act 1, right after the title), lunch (Act 2), overtime (Act 3) and the ward. Nothing
	// periodic, and nothing during the ending (Wrap isn't a checkpoint; the party and the card are never saved).
	// (Statics instead of members so this stays Live Coding friendly; reset whenever a new play session starts.)
	{
		static TWeakObjectPtr<UWorld> SaveWorld;
		static ETakeBeat SavedBeat = ETakeBeat::Count;
		if (SaveWorld.Get() != GetWorld()) { SaveWorld = GetWorld(); SavedBeat = ETakeBeat::Count; }   // the first beat of a session counts too
		const bool bPlaying = GI->bTitleDone && !GI->bEndingSeen && FallT < 0.f && ExitT < 0.f && EndStep == 0
			&& !UGameplayStatics::IsGamePaused(this);
		if (bPlaying && GI->Beat != SavedBeat)
		{
			SavedBeat = GI->Beat;
			const bool bActStart = SavedBeat == ETakeBeat::Morning || SavedBeat == ETakeBeat::Lunch
				|| SavedBeat == ETakeBeat::Overtime || SavedBeat == ETakeBeat::Ward;
			if (bActStart && UTakePauseMenu::SaveProgress(this))
			{
				UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Checkpoint saved (beat %d)"), static_cast<int32>(SavedBeat));
			}
		}
	}

	// 3.0: the afternoon is done and the clocks have stopped -> overtime (or right away for a dev jump)
	if (GI->Beat == ETakeBeat::Lunch && GI->bClocksFrozen && GI->Act2TasksDone())
	{
		if (GI->bDevAct3Now)
		{
			const ETakeBeat Target = GI->DevAct3Target;
			GI->DevAct3Target = ETakeBeat::Overtime;
			StartOvertime();
			if (Target == ETakeBeat::Ward || Target == ETakeBeat::Wrap)   // TakeBeat 6 / 7: skip the walk and the fall
			{
				GI->SetSoulBits(Target == ETakeBeat::Wrap ? 3 : 2);
				EnterWard();
				if (Target == ETakeBeat::Wrap) { ExitWard(); }
			}
		}
		else if (OvertimeIn < 0.f) { OvertimeIn = 60.f; }   // a full minute of 2:14 before overtime (20 s felt like a jump cut)
		else if ((OvertimeIn -= DeltaTime) <= 0.f) { StartOvertime(); }
	}

	TickForeshadow(DeltaTime);

	// 3.1 -> 3.2: falling backward (the view tips up), then the floor isn't there
	if (FallT >= 0.f)
	{
		FallT += DeltaTime;
		if (APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			if (AController* C = Pawn->GetController())
			{
				const FRotator R = C->GetControlRotation();
				C->SetControlRotation(FRotator(FMath::Min(70.f, 70.f * FallT / 1.2f), R.Yaw, 0.f));
			}
		}
		if (FallT >= 1.0f && FallT - DeltaTime < 1.0f) { ShowTimeCard(TEXT("2:14 PM"), TEXT("SCENE 5  \u00B7  THE WARD")); }
		if (FallT >= 1.4f) { FallT = -1.f; EnterWard(); }
	}

	// 3.3-3.5: the ward keeps you afraid, and something walks between the beds
	if (GI->Beat == ETakeBeat::Ward)
	{
		if (ATakeFearDirector* FearDir = FindFearDirector())
		{
			if (FearDir->TargetFear < 0.5f && FearDir->Fear < 0.5f) { FearDir->StartCreep(0.55f, 3.f); }
		}
		// The warning, about 4 s before it comes: the lights stutter, the stinger, Fear climbs (also after a rewind)
		if (StalkerIn > 4.f && StalkerIn - DeltaTime <= 4.f)
		{
			if (ATakeFearDirector* FearDir = FindFearDirector())
			{
				FearDir->FlickerBurst(3.8f);
				FearDir->StartCreep(FMath::Max(FearDir->Fear, 0.82f), 3.5f);
			}
			PlaySfx2D(TEXT("sfx_fear_hit"), 0.9f);
			if (!bTensionWardDone) { bTensionWardDone = true; PlayMusicCue(TEXT("the ward: it's coming"), TEXT("mus_halcyon_tension"), true); }
			ShowSubtitle(TEXT(""), TEXT("(The lights stutter. Somewhere past the last bed, something heavy starts walking.)"), 3.5f);
		}
		if (StalkerIn > 0.f && (StalkerIn -= DeltaTime) <= 0.f)
		{
			PlaySfx2D(TEXT("sfx_impact_heavy"), 0.8f);   // it's here
			if (!bFearWardDone) { bFearWardDone = true; PlayMusicCue(TEXT("the stalker is in the ward"), TEXT("mus_halcyon_fear1"), true); }
			for (TActorIterator<ATakeStalker> It(GetWorld()); It; ++It)
			{
				// Never start inside furniture: nudge to the nearest spot where its capsule fits
				// The open aisle south of the last bed row (2800,7200 was inside bed 3-5). Kept here, not in a
				// file-level constant, because Live Coding doesn't update constants (it kept the old spot).
				// From a different place each time (never right next to you): the aisles between and around the bed rows
				static const FVector Spots[] = {
					FVector(2500.f, 7420.f, -1380.f), FVector(1300.f, 7420.f, -1380.f), FVector(700.f, 6450.f, -1380.f),
					FVector(2900.f, 6450.f, -1380.f), FVector(1500.f, 5520.f, -1380.f), FVector(2600.f, 5520.f, -1380.f) };
				TArray<FVector> Far;
				const FVector Me = GetOwner()->GetActorLocation();
				for (const FVector& S : Spots) { if (FVector::Dist2D(S, Me) > 900.f) { Far.Add(S); } }
				FVector Start = Far.Num() > 0 ? Far[FMath::RandRange(0, Far.Num() - 1)] : Spots[0];
				if (!GetWorld()->FindTeleportSpot(*It, Start, It->GetActorRotation()))
				{
					UE_LOG(LogTake, Warning, TEXT("[TAKE 4471] Ward: no free spot near the stalker start, using it anyway"));
				}
				It->SetActorLocation(Start, false, nullptr, ETeleportType::TeleportPhysics);
				UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Ward: stalker enters at %s"), *Start.ToCompactString());
				It->Activate();
				break;
			}
			QueueSubtitle(TEXT(""), TEXT("(Something is walking between the beds. Throw things at it.)"));
		}
	}

	// Working a lock panel in the ward: it waits (it's watching you work, not giving you a free win; the timer still runs)
	if (LockPanel && GI->Beat == ETakeBeat::Ward)
	{
		for (TActorIterator<ATakeStalker> It(GetWorld()); It; ++It) { It->Stun(0.75f); }
	}

	// 3.6: the service elevator -> back upstairs, in the elevator car
	if (ExitT >= 0.f)
	{
		ExitT += DeltaTime;
		if (ExitT >= 0.35f)
		{
			ExitT = -1.f;
			GI->Beat = ETakeBeat::Wrap;
			for (TActorIterator<ATakeStalker> It(GetWorld()); It; ++It) { It->Deactivate(); }
			if (ATakeFearDirector* FearDir = FindFearDirector()) { FearDir->Calm(2.f); }
			TeleportPlayer(ElevatorCar, 90.f);
			QueueSubtitle(TEXT(""), TEXT("The doors close. When they open, you're upstairs. In the elevator."));
			QueueSubtitle(TEXT(""), TEXT("The panel: C is lit."));
			GI->SetObjective(GI->ObjectiveForBeat());
		}
	}

	// The ending, step by step
	if (EndT < 0.f) { return; }
	EndT += DeltaTime;
	TickRefusal(DeltaTime);
	if (bPenthouseBuilt) { TickDailies(DeltaTime); }
	// The party stays calm... unless you spent the day throwing things at these people
	if (EndStep >= 1 && !bRefused)
	{
		if (ATakeFearDirector* PartyFear = FindFearDirector())
		{
			if (!bBadEmployee && PartyFear->Fear > 0.02f) { PartyFear->SetFearImmediate(0.f); }
			else if (bBadEmployee && EndT > 12.f && PartyFear->TargetFear < 0.3f)
			{
				PartyFear->StartCreep(0.45f, 8.f);
				ShowSubtitle(TEXT(""), TEXT("(They remember every stapler. Every mug. They're still clapping, but nobody is smiling at you.)"), 5.f);
			}
		}
	}
	// The wrap party: walk around among them, but stay at the party (about 3.5 m from the middle)
	if (EndStep >= 1 && EndStep != 12 && EndStep != 13 && !(EndStep == 4 && bPenthouseBuilt))
	{
		if (APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			// The lobby and the kitchen (refreshments); the office, the conference room and the corridors are closed
			const FVector P = Pawn->GetActorLocation();
			// The lobby + kitchen box, and (to the west) the conference room; clamp to whichever is nearer
			const FVector InLobby(FMath::Clamp(P.X, 1550.f, 3650.f), FMath::Clamp(P.Y, EndStep == 11 ? 0.f : 560.f, 1450.f), P.Z);
			const FVector InConf(FMath::Clamp(P.X, 500.f, 1600.f), FMath::Clamp(P.Y, 800.f, 1400.f), P.Z);
			const FVector InOffice(FMath::Clamp(P.X, 1600.f, 3200.f), FMath::Clamp(P.Y, 1400.f, 3500.f), P.Z);   // the open office (after hours)
			FVector Leash = FVector::DistSquared2D(InLobby, P) <= FVector::DistSquared2D(InConf, P) ? InLobby : InConf;
			if (FVector::DistSquared2D(InOffice, P) < FVector::DistSquared2D(Leash, P)) { Leash = InOffice; }
			if (!Leash.Equals(P, 1.f))
			{
				Pawn->SetActorLocation(Leash, false, nullptr, ETeleportType::None);
				if (P.Y < 600.f && EndStep < 10)
				{
					if (!bPartyLiftSaid) { bPartyLiftSaid = true; ShowSubtitle(TEXT(""), TEXT("(The elevator doesn't open for you. Not yet. Not until you answer.)"), 3.f); }
				}
				else if (!bPartyLeashSaid) { bPartyLeashSaid = true; ShowSubtitle(TEXT("PA"), TEXT("The party is in the lobby. Corridor B has gone home."), 3.f); }
			}
		}
	}
	switch (EndStep)
	{
	case 0:   // under the card: everyone gathers in the lobby
		if (EndT >= 0.6f)
		{
			EndStep = 1;
			TeleportPlayer(LobbyParty, 90.f);   // facing the party (the elevator is behind you, the way stays clear)
			BuildLobbyParty();
			if (ATakeFearDirector* PartyLights = FindFearDirector()) { PartyLights->LightScale = 0.5f; }   // house lights down: a little too dim for a party
			if (APawn* Pawn = Cast<APawn>(GetOwner()))   // you can mingle now (a few steps; see the leash below)
			{
				if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController())) { PC->SetIgnoreMoveInput(false); }
			}
			for (TActorIterator<ATakeFearDirector> It(GetWorld()); It; ++It) { It->SetFearImmediate(0.f); }
			for (TActorIterator<ATakeStalker> It(GetWorld()); It; ++It) { It->Deactivate(); }
			if (const UTakeGameInstance* PartyGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this)))
			{
				bBadEmployee = PartyGI->CoworkersHit >= 6 || PartyGI->HotBeverageIncidents >= 2;   // they remember
			}
			int32 N = 0;
			for (TActorIterator<ATakeCoworker> It(GetWorld()); It; ++It) { if (!It->ActorHasTag(TEXT("Collector"))) { ++N; } }
			int32 I = 0;
			for (TActorIterator<ATakeCoworker> It(GetWorld()); It; ++It)
			{
				if (It->ActorHasTag(TEXT("Collector"))) { continue; }   // he has his own spot, by the elevator
				// a half ring in front of you, two rows deep, on the far side from the elevator (so the way to it stays clear)
				const float A = FMath::DegreesToRadians(15.f + 150.f * (N > 1 ? float(I) / float(N - 1) : 0.5f));
				const float R = (I % 2 == 0) ? 300.f : 420.f;
				FVector P = LobbyParty + FVector(FMath::Cos(A) * R, FMath::Sin(A) * R * 0.42f, 2.f);   // y stays under ~1230 (the reception desk starts ~1300)
				GetWorld()->FindTeleportSpot(*It, P, It->GetActorRotation());   // never inside furniture
				It->JoinParty(P, LobbyParty);
				++I;
			}
		}
		break;
	case 1:   // applause, paper confetti, and the speeches
		if (EndT >= 1.0f)
		{
			EndStep = 2;
			PlaySfx2D(TEXT("sfx_applause"), 0.9f);
			for (int32 C = 0; C < 26; ++C)
			{
				const FVector P = LobbyParty + FVector(FMath::FRandRange(-260.f, 260.f), FMath::FRandRange(60.f, 380.f), FMath::FRandRange(160.f, 260.f));
				if (AActor* Bit = SpawnProp(C % 3 == 0 ? TEXT("SM_Brochure") : TEXT("SM_PaperBall"), P, FRotator(FMath::FRandRange(0.f, 360.f), FMath::FRandRange(0.f, 360.f), 0.f)))
				{
					if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Bit->GetRootComponent()))
					{
						Prim->AddImpulse(FVector(FMath::FRandRange(-80.f, 80.f), FMath::FRandRange(-80.f, 80.f), FMath::FRandRange(50.f, 180.f)), NAME_None, true);
					}
				}
			}
			QueueSubtitle(TEXT("PA"), TEXT("That's a wrap on Employee 4471! That's a wrap on Employee 4471!"));
			QueueSubtitle(TEXT("Coworker"), TEXT("You were incredible. We're so proud of you."));
			QueueSubtitle(TEXT(""), TEXT("(The clock over the elevator still reads 2:14.)"));
			QueueSubtitle(TEXT("The Collector"), TEXT("Congratulations, 4471. Really. Not many make it to the end of the take."));
			QueueSubtitle(TEXT("The Collector"), TEXT("The Chairman watched every second. He thinks you've earned your director's cut."));
			QueueSubtitle(TEXT("The Collector"), TEXT("Sign off on this one and you never do another take. Same desk. Same mug. The perfect day, forever. That's the theatrical cut."));
			QueueSubtitle(TEXT("The Collector"), TEXT("You just have to hand over your keys."));
		}
		break;
	case 2:   // when the speeches are over: mingle (the question comes later, from the man in the hat)
		if ((SubQueue.Num() == 0 && SubtitleLeft <= 0.f && EndT > 4.f) || EndT > 45.f)   // (never wait forever on a stray line)
		{
			StartMingle();
		}
		break;
	case 30:   // mingling: talk to people, have cake; when you've been sociable enough, the Collector wants a word
		MingleT += DeltaTime;
		if (!bCollectorReady && (PartyChats >= 3 || (bAteCake && PartyChats >= 1) || MingleT > 150.f))
		{
			bCollectorReady = true;
			if (ATakeCoworker* Col = Collector.Get())
			{
				Col->bPartyReady = true;
				Col->SayLine(TEXT("{EMP}. A word? Over here, by the elevator."));
			}
			if (UTakeGameInstance* ColGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { ColGI->SetObjective(TEXT("The man in the hat wants a word.")); }
		}
		break;
	case 3:
		break;
	}
	if (EndStep == 31)   // you talked to him: the question
	{
		{
			EndStep = 3;
			APawn* Pawn = Cast<APawn>(GetOwner());
			APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
			ChoiceW = PC ? CreateWidget<UTakeChoiceWidget>(PC, UTakeChoiceWidget::StaticClass()) : nullptr;
			if (ChoiceW)
			{
				ChoiceW->Question = TEXT("HAND OVER YOUR KEYS?");
				ChoiceW->OnChoice.AddDynamic(this, &UTakeGrabComponent::OnEndingChoice);
				ChoiceW->AddToViewport(40);
				FInputModeUIOnly Mode;
				Mode.SetWidgetToFocus(ChoiceW->TakeWidget());
				PC->SetInputMode(Mode);
				PC->SetShowMouseCursor(true);
			}
		}
	}
	switch (EndStep)
	{
	case 10:   // YES: when the lines are done (or a few seconds), the elevator dings (then you walk to it: UseThing -> GoToPenthouse)
		YesT += DeltaTime;
		if ((SubQueue.Num() == 0 && SubtitleLeft <= 0.f) || YesT > 6.f)
		{
			EndStep = 11;
			PlaySfx(TEXT("sfx_badge_ok"), FVector(2100.f, 500.f, 150.f), 1.f);
			ShowSubtitle(TEXT(""), TEXT("(Behind you, the elevator dings. C is lit. Nobody else moves.)"), 4.f);
			if (UTakeGameInstance* LiftGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { LiftGI->SetObjective(TEXT("Take the elevator. C is lit.")); }
		}
		break;
	case 4:   // the answer's lines, then the end card
		if (SubQueue.Num() == 0 && SubtitleLeft <= 0.f)
		{
			EndStep = 5;
			EndCardT = 0.f;
			PlaySfx2D(TEXT("sfx_clapper_snap"), 0.8f);
		}
		break;
	case 5:   // THE THEATRICAL CUT / THE DIRECTOR'S CUT, then Soul Sight flickers on the Collector... cut to black
		EndCardT += DeltaTime;
		if (Prompt)
		{
			const FString Card = EndCardT < 4.f ? (bEndingYes ? FString(TEXT("THE THEATRICAL CUT")) : FString(TEXT("THE DIRECTOR'S CUT")))
				: (bEndingYes ? TakeText::Fill(this, TEXT("DIRECTED BY\nEMPLOYEE {EMP}")) : FString(TEXT("RESET.\nFROM THE TOP.")));
			Prompt->SetSlate(FMath::Clamp(EndCardT / 0.4f, 0.f, 1.f), Card, EndCardT < 4.f ? FMath::Clamp(EndCardT / 0.6f, 0.f, 1.f)
				: FMath::Clamp(1.f - (EndCardT - 7.f) / 0.3f, 0.f, 1.f));
		}
		if (EndCardT > 7.4f)
		{
			EndStep = 6;
			PlayMusicCue(TEXT("end credits"), TEXT("mus_halcyon_calm"), true);
			APawn* Pawn = Cast<APawn>(GetOwner());
			APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
			CreditsW = PC ? CreateWidget<UTakeCreditsWidget>(PC, UTakeCreditsWidget::StaticClass()) : nullptr;
			if (CreditsW)
			{
				if (const UTakeGameInstance* NextGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { CreditsW->NextTake = NextGI->TakeNumber + 1; }
				CreditsW->DirectedBy = bEndingYes ? TakeText::Fill(this, TEXT("Employee {EMP}")) : FString(TEXT("________________"));
				CreditsW->SetIsFocusable(true);
				CreditsW->OnChoice.AddDynamic(this, &UTakeGrabComponent::OnCreditsDone);
				CreditsW->AddToViewport(60);
				FInputModeUIOnly Mode;
				Mode.SetWidgetToFocus(CreditsW->TakeWidget());
				PC->SetInputMode(Mode);
			}
		}
		break;
	default:
		break;
	}
}

void UTakeGrabComponent::StartEnding()
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI || EndT >= 0.f) { return; }
	GI->Beat = ETakeBeat::Wrap;
	GI->bEndingSeen = true;
	EndT = 0.f;
	EndStep = 0;
	ShowTimeCard(TEXT("??:??"), TEXT("WRAP PARTY"));
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController())) { PC->SetIgnoreMoveInput(true); }   // still for the card; you can walk around once you are at the party
	}
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] The ending (wrap party)"));
}

void UTakeGrabComponent::OnEndingChoice(int32 Choice)
{
	bEndingYes = Choice == 0;
	if (ChoiceW) { ChoiceW->RemoveFromParent(); ChoiceW = nullptr; }
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController())) { PC->SetInputMode(FInputModeGameOnly()); PC->SetShowMouseCursor(false); }
	}
	if (bEndingYes)   // the elevator, the director's floor (TakeDirector.cpp)
	{
		QueueSubtitle(TEXT("The Collector"), TEXT("Wonderful. I'll take those."));
		QueueSubtitle(TEXT(""), TEXT("(The keys are warm. They were never really yours.)"));
		EndStep = 10;
	}
	else              // the set comes down (TakeDirector.cpp)
	{
		QueueSubtitle(TEXT("The Collector"), TEXT("...No?"));
		StartRefusal();
	}
}

void UTakeGrabComponent::OnCreditsDone(int32 Choice)
{
	// 0 = PLAY AGAIN: a new take from the morning, straight into the slate. 1 = back to the title (Continue still loads your save).
	if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (Choice == 0) { GI->ResetRun(); GI->bTitleDone = true; GI->EmployeeNumber += 1; }   // you're the next one now
		else { GI->bTitleDone = false; }
	}
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController())) { PC->SetInputMode(FInputModeGameOnly()); PC->SetShowMouseCursor(false); }
	}
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)));
}

// ------------------------------------------------------------------ title
void UTakeGrabComponent::ShowTitle()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC)
	{
		if (UTakeGameInstance* NoTitleGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { NoTitleGI->HideWorldUntilTitle(false); }
		StartSlate(false);
		return;
	}
	TitleW = CreateWidget<UTakeTitleWidget>(PC, UTakeTitleWidget::StaticClass());
	if (!TitleW) { StartSlate(false); return; }
	TitleW->bHasSave = UGameplayStatics::DoesSaveGameExist(UTakeSaveGame::ProgressSlot, 0);
	TitleW->SetIsFocusable(true);
	TitleW->OnChoice.AddDynamic(this, &UTakeGrabComponent::OnTitleChoice);
	TitleW->AddToViewport(70);
	if (Prompt) { Prompt->SetSlate(1.f, FString(), 0.f); }
	UGameplayStatics::SetGamePaused(this, true);   // the office waits behind the menu
	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(TitleW->TakeWidget());
	PC->SetInputMode(Mode);
	PC->SetShowMouseCursor(true);
}

void UTakeGrabComponent::OnTitleChoice(int32 Choice)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (Choice == 2) { UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false); return; }
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI)
	{
		GI->bTitleDone = true;
		GI->HideWorldUntilTitle(false);
		if (Choice == 0 && (GI->Beat != ETakeBeat::Morning || GI->bEndingSeen)) { GI->ResetRun(); }   // a fresh take after an ending
		if (Choice == 1 && GI->bEndingSeen) { GI->ResetRun(); }   // CONTINUE after the credits: clear the ending first (the save fills in the rest)
	}
	if (TitleW) { TitleW->RemoveFromParent(); TitleW = nullptr; }
	UGameplayStatics::SetGamePaused(this, false);
	if (PC) { PC->SetInputMode(FInputModeGameOnly()); PC->SetShowMouseCursor(false); }
	if (Choice == 1 && UTakePauseMenu::LoadProgress(this))
	{
		if (Prompt) { Prompt->SetSlate(0.f, FString(), 0.f); }
		SetColdOpenFade(0.f);   // the screen stayed black behind the title
		ShowSubtitle(TEXT(""), TEXT("Welcome back."), 2.f);
		if (ATakeFearDirector* LoadFear = FindFearDirector()) { LoadFear->SetFearImmediate(GI && GI->bClocksFrozen ? 0.15f : 0.f); }   // you come back calm
		// Each checkpoint is the start of an act: put back what that act's start sets up (the rest comes from the save)
		const ETakeBeat LoadedBeat = GI ? GI->Beat : ETakeBeat::Morning;
		if (LoadedBeat == ETakeBeat::Morning)
		{
			StartSlate(false);   // Act 1 from the top: TAKE n / 9:00 AM / SCENE 1 - MORNING, same take number as the save
		}
		else if (LoadedBeat == ETakeBeat::Lunch)
		{
			ShowTimeCard(TEXT("12:00 PM"), TEXT("SCENE 2  \u00B7  LUNCH"));
			LunchPAPlayed = 0;
			LunchPALeft = 1 + FMath::Max(0, LunchPAExtraRepeats);   // "Lunch is served." x3, like the first time
			LunchPAIn = 4.2f;
		}
		else if (LoadedBeat == ETakeBeat::Overtime)
		{
			ShowTimeCard(TEXT("2:14 PM"), TEXT("SCENE 4  \u00B7  OVERTIME"));
			if (!bForeshadowDone) { ForeshadowIn = 35.f; }   // the footsteps through the office, as in StartOvertime
		}
		else if (LoadedBeat == ETakeBeat::Ward)
		{
			// Saved in the ward: set it up again (weapons, the stalker on its way); you pick up at the ward entrance
			ShowTimeCard(TEXT("2:14 PM"), TEXT("SCENE 5  \u00B7  THE WARD"));
			EnterWard();
		}
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] CONTINUE: checkpoint beat %d"), static_cast<int32>(LoadedBeat));
		return;
	}
	StartSlate(false);   // TAKE 1 / 9:00 AM / SCENE 1 - MORNING
}
