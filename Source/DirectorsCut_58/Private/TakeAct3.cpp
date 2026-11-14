// TAKE 4471 - Act 3 (overtime), the ending (the wrap party) and the title screen. Part of UTakeGrabComponent.
//   3.0  Act 2 done + the clocks stopped -> SCENE 4 · OVERTIME: "Mandatory team building in the Wellness Center."
//   3.1  pass 2 opens the Wellness Center; coworkers stand in a circle; E on the X = fall backward
//   3.2  nobody catches you -> SCENE 5 · THE WARD (the hospital below); Fear stays high; capture rewinds to the ward door
//   3.3  charts are script pages; two carry Key 3's squares, one is a decoy (4470's, different handwriting)
//   3.5  the nurses' station terminal (Key Lock 3, signature hidden) -> pass 3
//   3.6  the service elevator -> SCENE 6 · C: you're back in the elevator; C is lit
//   E    press C -> WRAP PARTY: everyone in the lobby, applause, paper confetti, the Collector, YES / NO, an end card, credits
#include "TakeGrabComponent.h"
#include "DirectorsCut_58.h"
#include "TakeGameInstance.h"
#include "TakeFearDirector.h"
#include "TakeStalker.h"
#include "TakeCoworker.h"
#include "TakeWidgets.h"
#include "TakeAct3Widgets.h"
#include "TakePauseMenu.h"
#include "TakeSaveGame.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
	const FVector WardLanding(1400.f, 6400.f, -1400.f);    // HospitalWard marker (floor at -1500)
	const FVector WardStalkerStart(2800.f, 7200.f, -1380.f);
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
	ShowTimeCard(TEXT("2:14 PM"), TEXT("SCENE 4  ·  OVERTIME"));
	PlaySfx2D(TEXT("sfx_pa_chime"), 0.7f);
	QueueSubtitle(TEXT("PA"), TEXT("Mandatory team building in the Wellness Center. Attendance is mandatory."));
	QueueSubtitle(TEXT("PA"), TEXT("Attendance has always been mandatory."));
	if (ATakeFearDirector* FearDir = FindFearDirector()) { FearDir->StartCreep(FMath::Max(FearDir->Fear, 0.2f), 4.f); }
	GI->SetObjective(GI->ObjectiveForBeat());
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Beat -> Overtime (Act 3)"));
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
	GI->SetObjective(GI->ObjectiveForBeat());
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Beat -> Ward"));
}

void UTakeGrabComponent::ExitWard()
{
	if (ExitT >= 0.f) { return; }
	ExitT = 0.f;
	PlaySfx2D(TEXT("sfx_badge_ok"), 0.7f);
	ShowTimeCard(TEXT("2:14 PM"), TEXT("SCENE 6  ·  C"));
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

	// 3.0: the afternoon is done and the clocks have stopped -> overtime (or right away for a dev jump)
	if (GI->Beat == ETakeBeat::Lunch && GI->bClocksFrozen && GI->Act2TasksDone())
	{
		if (GI->bDevAct3Now) { StartOvertime(); }
		else if (OvertimeIn < 0.f) { OvertimeIn = 20.f; }
		else if ((OvertimeIn -= DeltaTime) <= 0.f) { StartOvertime(); }
	}

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
		if (FallT >= 1.0f && FallT - DeltaTime < 1.0f) { ShowTimeCard(TEXT("2:14 PM"), TEXT("SCENE 5  ·  THE WARD")); }
		if (FallT >= 1.4f) { FallT = -1.f; EnterWard(); }
	}

	// 3.3-3.5: the ward keeps you afraid, and something walks between the beds
	if (GI->Beat == ETakeBeat::Ward)
	{
		if (ATakeFearDirector* FearDir = FindFearDirector())
		{
			if (FearDir->TargetFear < 0.5f && FearDir->Fear < 0.5f) { FearDir->StartCreep(0.55f, 3.f); }
		}
		if (StalkerIn > 0.f && (StalkerIn -= DeltaTime) <= 0.f)
		{
			for (TActorIterator<ATakeStalker> It(GetWorld()); It; ++It)
			{
				It->SetActorLocation(WardStalkerStart, false, nullptr, ETeleportType::TeleportPhysics);
				It->Activate();
				break;
			}
			QueueSubtitle(TEXT(""), TEXT("(Something is walking between the beds. Throw things at it.)"));
		}
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
	switch (EndStep)
	{
	case 0:   // under the card: everyone gathers in the lobby
		if (EndT >= 0.6f)
		{
			EndStep = 1;
			TeleportPlayer(LobbyParty, -90.f);   // facing the elevator
			for (TActorIterator<ATakeFearDirector> It(GetWorld()); It; ++It) { It->SetFearImmediate(0.f); }
			for (TActorIterator<ATakeStalker> It(GetWorld()); It; ++It) { It->Deactivate(); }
			int32 N = 0;
			for (TActorIterator<ATakeCoworker> It(GetWorld()); It; ++It) { ++N; }
			int32 I = 0;
			for (TActorIterator<ATakeCoworker> It(GetWorld()); It; ++It, ++I)
			{
				// a half ring in front of you, two rows deep
				const float A = FMath::DegreesToRadians(-160.f + 140.f * (N > 1 ? float(I) / float(N - 1) : 0.5f));
				const float R = (I % 2 == 0) ? 330.f : 450.f;
				const FVector P = LobbyParty + FVector(FMath::Cos(A) * R, FMath::Sin(A) * R, -8.f);
				It->JoinParty(P, LobbyParty);
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
				const FVector P = LobbyParty + FVector(FMath::FRandRange(-260.f, 260.f), FMath::FRandRange(-380.f, -60.f), FMath::FRandRange(160.f, 260.f));
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
			QueueSubtitle(TEXT("The Collector"), TEXT("The astral realm is open now. Build yourself any world you want. A palace. An afterlife. No more corridors."));
			QueueSubtitle(TEXT("The Collector"), TEXT("You just have to hand over your keys."));
		}
		break;
	case 2:   // when the speeches are over: the choice
		if (SubQueue.Num() == 0 && SubtitleLeft <= 0.f && EndT > 4.f)
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
				: FString(TEXT("SOUL SIGHT\nTHE COLLECTOR IS \u2588\u2588\u2588\u2588\u2588\u2588"));
			Prompt->SetSlate(FMath::Clamp(EndCardT / 0.4f, 0.f, 1.f), Card, EndCardT < 4.f ? FMath::Clamp(EndCardT / 0.6f, 0.f, 1.f)
				: FMath::Clamp(1.f - (EndCardT - 7.f) / 0.3f, 0.f, 1.f));
		}
		if (EndCardT > 7.4f)
		{
			EndStep = 6;
			APawn* Pawn = Cast<APawn>(GetOwner());
			APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
			CreditsW = PC ? CreateWidget<UTakeCreditsWidget>(PC, UTakeCreditsWidget::StaticClass()) : nullptr;
			if (CreditsW)
			{
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
		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController())) { PC->SetIgnoreMoveInput(true); }   // look around, but stay for the speeches
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
	if (bEndingYes)
	{
		QueueSubtitle(TEXT("The Collector"), TEXT("Wonderful. Your world is being built as we speak. You'll love it. Everyone does."));
		QueueSubtitle(TEXT(""), TEXT("(A bright palace. Perfect lawns. A warm sun that never moves. Everyone you meet is smiling, a beat too long.)"));
	}
	else
	{
		QueueSubtitle(TEXT("The Collector"), TEXT("Of course. Of course. Take your time."));
		QueueSubtitle(TEXT(""), TEXT("(The passes are no longer on your badge. You never felt them go.)"));
		QueueSubtitle(TEXT(""), TEXT("(For three seconds: the uncut footage. The real office. Empty desks. A long line of badges on hooks, each with a number.)"));
		if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { GI->SetSoulBits(0); }
	}
	EndStep = 4;
}

void UTakeGrabComponent::OnCreditsDone(int32 Unused)
{
	// Back to the title. A new run starts from the morning (Continue still loads your save).
	if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { GI->bTitleDone = false; }
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this, true)));
}

// ------------------------------------------------------------------ title
void UTakeGrabComponent::ShowTitle()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC) { StartSlate(false); return; }
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
		if (Choice == 0 && (GI->Beat != ETakeBeat::Morning || GI->bEndingSeen)) { GI->ResetRun(); }   // a fresh take after an ending
	}
	if (TitleW) { TitleW->RemoveFromParent(); TitleW = nullptr; }
	UGameplayStatics::SetGamePaused(this, false);
	if (PC) { PC->SetInputMode(FInputModeGameOnly()); PC->SetShowMouseCursor(false); }
	if (Choice == 1 && UTakePauseMenu::LoadProgress(this))
	{
		if (Prompt) { Prompt->SetSlate(0.f, FString(), 0.f); }
		ShowSubtitle(TEXT(""), TEXT("Welcome back."), 2.f);
		return;
	}
	StartSlate(false);   // TAKE 1 / 9:00 AM / SCENE 1 · MORNING
}
