// TAKE 4471 - run state + console commands
#include "TakeGameInstance.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "TakeGrabComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "TakeCoworker.h"
#include "EngineUtils.h"
#include "DirectorsCut_58.h"
#include "TakeFearDirector.h"
#include "TakeStalker.h"
#include "TakeLoopTrigger.h"
#include "TakeRewindSubsystem.h"
#include "TakeKeyLockLibrary.h"
#include "EngineUtils.h"
#include "Misc/DateTime.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformTime.h"

void UTakeGameInstance::Init()
{
	Super::Init();
	// Before the title screen, don't draw the world at all (the camera fade only starts at BeginPlay, so the first
	// frames could show the office through the "loading" black). The title choice turns it back on.
	FWorldDelegates::OnPostWorldInitialization.AddWeakLambda(this, [this](UWorld* World, const UWorld::InitializationValues)
	{
		if (World && World->IsGameWorld() && !bTitleDone) { HideWorldUntilTitle(true); }
	});
	if (DevSeedOverride >= 0)
	{
		RunSeed = DevSeedOverride;
	}
	else
	{
		const uint64 Mixed = static_cast<uint64>(FDateTime::Now().GetTicks()) ^ (FPlatformTime::Cycles64() << 17);
		RunSeed = static_cast<int32>((Mixed ^ (Mixed >> 31)) & 0x7FFFFFFF);
	}
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Run started. Take %d."), TakeNumber);   // seed deliberately not logged
}

void UTakeGameInstance::HideWorldUntilTitle(bool bHide)
{
	if (UGameViewportClient* VC = GetGameViewportClient()) { VC->bDisableWorldRendering = bHide; }
}

void UTakeGameInstance::SetSoulBits(int32 NewValue)
{
	SoulBits = FMath::Clamp(NewValue, 0, 3);
	OnSoulBitsChanged.Broadcast(SoulBits);
}

void UTakeGameInstance::TakeFear(float Value)
{
	for (TActorIterator<ATakeFearDirector> It(GetWorld()); It; ++It) { It->SetFearImmediate(Value); }
}

void UTakeGameInstance::TakeCreep(float Target, float Seconds)
{
	for (TActorIterator<ATakeFearDirector> It(GetWorld()); It; ++It) { It->bGateOverride = true; It->StartCreep(Target, Seconds); It->bGateOverride = false; }
}

void UTakeGameInstance::TakeEnding(int32 Choice)
{
	if (APawn* Pawn = GetFirstLocalPlayerController() ? GetFirstLocalPlayerController()->GetPawn() : nullptr)
	{
		if (UTakeGrabComponent* Grab = Pawn->FindComponentByClass<UTakeGrabComponent>()) { Grab->DevEnding(Choice); }
	}
}

void UTakeGameInstance::TakeSoulBits(int32 Value) { SetSoulBits(Value); }

void UTakeGameInstance::TakeStalker()
{
	for (TActorIterator<ATakeStalker> It(GetWorld()); It; ++It) { It->Activate(); }
}

void UTakeGameInstance::TakeRewind()
{
	if (UTakeRewindSubsystem* R = GetWorld() ? GetWorld()->GetSubsystem<UTakeRewindSubsystem>() : nullptr) { R->Rewind(); }
}

void UTakeGameInstance::TakeLoop(bool bEnable)
{
	bDevForceLoop = bEnable;   // the Fear Director otherwise only allows the loop during the Corridor B beat
	for (TActorIterator<ATakeLoopTrigger> It(GetWorld()); It; ++It) { It->SetLoopEnabled(bEnable); }
}

void UTakeGameInstance::TakeSeed()
{
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] RunSeed=%d"), RunSeed);
	for (int32 Key = 1; Key <= 3; ++Key)
	{
		const FTakeKeyLock L = UTakeKeyLockLibrary::GenerateKeyLock(RunSeed, Key, 0);
		UE_LOG(LogTake, Log, TEXT("  Key %d target %s"), Key, *UTakeKeyLockLibrary::GlyphRowString(L.Target, L.Bits));
		for (int32 I = 0; I < L.Fragments.Num(); ++I)
		{
			UE_LOG(LogTake, Log, TEXT("    frag %d %s%s"), I, *UTakeKeyLockLibrary::GlyphRowString(L.Fragments[I], L.Bits, L.HiddenMasks[I]),
			       L.IsDecoy[I] ? TEXT("  <- malware") : TEXT(""));
		}
	}
}

void UTakeGameInstance::TakeNPCYaw(float Yaw)
{
	if (UWorld* World = GetWorld())
	{
		int32 N = 0;
		for (TActorIterator<ATakeCoworker> It(World); It; ++It) { It->SetMeshYaw(Yaw); ++N; }
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] NPC mesh yaw = %.0f on %d coworker(s)"), Yaw, N);
	}
}

void UTakeGameInstance::SetObjective(const FString& NewObjective)
{
	if (Objective == NewObjective) { return; }
	Objective = NewObjective;
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Objective: %s"), *Objective);
	OnObjectiveChanged.Broadcast(Objective);
}

FString UTakeGameInstance::ObjectiveForBeat() const
{
	switch (Beat)
	{
	case ETakeBeat::Morning:      return bMetReceptionist ? FString(TEXT("Finish your badge tasks before noon")) : FString(TEXT("Check in at Reception (Lobby)"));
	case ETakeBeat::Meeting:      return FString(TEXT("Attend the 10:00 meeting (Conference Room)"));
	case ETakeBeat::Corridor:     return FString(TEXT("Something is at the end of Corridor B."));
	case ETakeBeat::ReturnToDesk: return FString(TEXT("Return to your desk."));
	case ETakeBeat::Lunch:
		if (!bJoinedCall)            { return FString(TEXT("Join the 12:30 call (Conference Room)")); }
		if (!bAttendedMeeting)       { return FString(TEXT("Attend the 1:00 meeting (Conference Room)")); }
		if (CoworkersCheckedIn < 3)  { return FString(TEXT("Check in with Reception, Security and the Custodian (look for the !)")); }
		if (!bRepliedChairman)       { return FString(TEXT("Reply to the Chairman (your terminal)")); }
		return FString(TEXT("Wait for further direction."));
	case ETakeBeat::Overtime:
		if (SoulBits < 2) { return FString(TEXT("Team building needs access pass 2 (the Supply Closet terminal)")); }
		return FString(TEXT("Mandatory team building: Wellness Center (end of Corridor B)"));
	case ETakeBeat::Ward:
		if (SoulBits < 3) { return FString(TEXT("Find Key 3 in the charts. The nurses' station terminal.")); }
		return FString(TEXT("The service elevator (east end of the ward)"));
	case ETakeBeat::Wrap:         return FString(TEXT("Press C."));
	default:                      return FString();
	}
}

void UTakeGameInstance::TakeBeat(int32 NewBeat)
{
	if (NewBeat < 0)
	{
		const TCHAR* Help = TEXT("TakeBeat N:  0 morning | 1 meeting | 2 corridor | 3 back to desk | 4 lunch (Act 2) | 5 overtime (Act 3) | 6 the ward | 7 wrap party");
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] %s"), Help);
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Cyan, Help); }
		return;
	}
	const int32 Clamped = FMath::Clamp(NewBeat, 0, static_cast<int32>(ETakeBeat::Count) - 1);
	if (Clamped == static_cast<int32>(ETakeBeat::Morning))
	{
		bReadEmail = false;   // otherwise the morning would complete again on the next frame
		bGotCoffee = false;
	}
	else
	{
		bReadEmail = bGotCoffee = bTrainingComplete = bMetReceptionist = true;
		if (SoulBits < 1) { SetSoulBits(1); }
	}
	// Act 2 state only exists from the lunch beat on
	bJoinedCall = bAttendedMeeting = bRepliedChairman = bClocksFrozen = false;
	CoworkersCheckedIn = 0;
	bDevLunchNow = false;
	if (Clamped >= static_cast<int32>(ETakeBeat::Overtime))
	{
		// Act 3 test: Act 2 done, clocks stopped; the story code starts Overtime on the next frame
		Beat = ETakeBeat::Lunch;
		bJoinedCall = bAttendedMeeting = bRepliedChairman = bClocksFrozen = true;
		CoworkersCheckedIn = 3;
		WorkdayMinutes = 14.f * 60.f + 14.f;
		bDevAct3Now = true;
		DevAct3Target = static_cast<ETakeBeat>(Clamped);   // Overtime, Ward or Wrap: the story code goes on from there
		SetObjective(ObjectiveForBeat());
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] TakeBeat %d: jumping to Act 3%s"), Clamped,
			Clamped == static_cast<int32>(ETakeBeat::Ward) ? TEXT(", then the ward") : Clamped == static_cast<int32>(ETakeBeat::Wrap) ? TEXT(", then the wrap party") : TEXT(""));
		return;
	}
	if (Clamped >= static_cast<int32>(ETakeBeat::Lunch))
	{
		// Stop just before beat 1.10 and let the story code play it (12:00, "Lunch is served.", Act 2 tasks)
		Beat = ETakeBeat::ReturnToDesk;
		bDevLunchNow = true;
	}
	else
	{
		Beat = static_cast<ETakeBeat>(Clamped);
		if (WorkdayMinutes > 12.f * 60.f) { WorkdayMinutes = 11.f * 60.f + 30.f; }
	}
	bGotWisp = Beat == ETakeBeat::ReturnToDesk;
	SetObjective(ObjectiveForBeat());
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Beat -> %d%s"), Clamped, bDevLunchNow ? TEXT(" (beat 1.10 plays now)") : TEXT(""));
}

void UTakeGameInstance::TakeTime(int32 HHMM)
{
	const int32 Hours = FMath::Clamp(HHMM / 100, 0, 23);
	const int32 Minutes = FMath::Clamp(HHMM % 100, 0, 59);
	WorkdayMinutes = static_cast<float>(Hours * 60 + Minutes);
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Clock -> %02d:%02d"), Hours, Minutes);
}

FString UTakeGameInstance::TaskListText() const
{
	auto Box = [](bool bDone) { return bDone ? TEXT("[x]") : TEXT("[  ]"); };
	if (CurrentAct() >= 3)
	{
		return FString::Printf(TEXT("%s  Access pass 2\n%s  Team building (Wellness Center)\n%s  Access pass 3\n%s  Press C"),
		                       Box(SoulBits >= 2), Box(bTrustFell), Box(SoulBits >= 3), Box(bEndingSeen));
	}
	if (CurrentAct() >= 2)
	{
		const int32 Met = FMath::Clamp(CoworkersCheckedIn, 0, 3);
		return FString::Printf(TEXT("%s  Join 12:30 call\n%s  Attend 1:00 meeting\n%s  Check in: Reception, Security, Custodian  (%d/3)\n%s  Reply to the Chairman"),
		                       Box(bJoinedCall), Box(bAttendedMeeting), Box(Met >= 3), Met, Box(bRepliedChairman));
	}
	return FString::Printf(TEXT("%s  Read your email (inbox zero)\n%s  Get coffee\n%s  Complete Compliance Training"),
	                       Box(bReadEmail), Box(bGotCoffee), Box(bTrainingComplete));
}

int32 UTakeGameInstance::TaskSignature() const
{
	if (CurrentAct() >= 3)
	{
		return 0x400 | (SoulBits >= 2 ? 1 : 0) | (bTrustFell ? 2 : 0) | (SoulBits >= 3 ? 4 : 0) | (bEndingSeen ? 8 : 0) | (static_cast<int32>(Beat) << 4);
	}
	if (CurrentAct() >= 2)
	{
		return 0x100 | (bJoinedCall ? 1 : 0) | (bAttendedMeeting ? 2 : 0) | (bRepliedChairman ? 4 : 0) | (FMath::Clamp(CoworkersCheckedIn, 0, 3) << 4);
	}
	return (bReadEmail ? 1 : 0) | (bGotCoffee ? 2 : 0) | (bTrainingComplete ? 4 : 0);
}

FString UTakeGameInstance::GlyphString(int32 Value, int32 Bits)
{
	FString Out;
	for (int32 B = 0; B < Bits; ++B) { Out += ((Value >> B) & 1) ? TEXT("\u25A0") : TEXT("\u25A1"); }
	return Out;
}

bool UTakeGameInstance::AddGlyphNote(int32 Key, const FString& Source, int32 Value, int32 Bits)
{
	const FString Prefix = FString::Printf(TEXT("Key %d  \u00B7  %s"), Key, *Source);
	for (const FString& N : GlyphNotes) { if (N.StartsWith(Prefix)) { return false; } }
	GlyphNotes.Add(FString::Printf(TEXT("%s  \u00B7  %s"), *Prefix, *GlyphString(Value, Bits)));
	return true;
}

void UTakeGameInstance::TakeSkip()
{
	if (CurrentAct() < 2)
	{
		if (!MorningTasksDone())
		{
			bReadEmail = bGotCoffee = bTrainingComplete = bMetReceptionist = true;
			if (SoulBits < 1) { SetSoulBits(1); }
			SetObjective(ObjectiveForBeat());
			return;
		}
		TakeBeat(4);   // morning done: straight to lunch (beat 1.10)
		return;
	}
	if (CurrentAct() >= 3)
	{
		if (const APawn* P = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			if (UTakeGrabComponent* Hands = P->FindComponentByClass<UTakeGrabComponent>()) { Hands->SkipAct3Step(); }
		}
		return;
	}
	if (bJoinedCall && bAttendedMeeting && bRepliedChairman && CoworkersCheckedIn >= 3) { bDevAct3Now = true; return; }   // already done: on to Act 3
	bJoinedCall = bAttendedMeeting = bRepliedChairman = true;
	CoworkersCheckedIn = 3;
	WorkdayMinutes = FMath::Max(WorkdayMinutes, 14.f * 60.f + 10.f);   // four minutes to 2:14
	SetObjective(ObjectiveForBeat());
}

void UTakeGameInstance::ResetRun()
{
	Beat = ETakeBeat::Morning;
	SetSoulBits(0);
	TrainingSlide = HRSlide = HRModulesDone = 0;
	bTrainingComplete = bReadEmail = bGotCoffee = bMetReceptionist = bGotWisp = false;
	bJoinedCall = bAttendedMeeting = bRepliedChairman = bClocksFrozen = bRepliedPhish = false;
	bTrustFell = bEndingSeen = bDevLunchNow = bDevAct3Now = bHotBevModuleDone = false;
	CoworkersCheckedIn = CoworkersHit = HotBeverageIncidents = GuardComplaints = CoffeesPoured = Baskets = BestTossStreak = 0;
	WorkdayMinutes = 540.f;
	Energy = 100.f;
	RewindPoint = FVector::ZeroVector;
	ReadMail.Reset();
	MailReplies.Reset();
	GlyphNotes.Reset();
	TakeNumber += 1;   // it's a new take, after all
	SetObjective(ObjectiveForBeat());
}
