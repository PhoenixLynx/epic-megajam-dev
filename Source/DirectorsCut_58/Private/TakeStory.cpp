// TAKE 4471 - Act 1 progression (overnight run 1). Part of UTakeGrabComponent, kept in its own file:
//   cold open (beat 1.1)  : black, "TAKE n", clapper, PA chime + "Good morning, team."
//   objective             : GameInstance->Objective, drawn under the badge task list, "New objective" cue
//   morning complete      : -> "Attend the 10:00 meeting (Conference Room)"
//   ConferenceRoom box    : the slip (lights flicker, Fear ~0.35, "...Where is everyone?") -> Corridor B
//   CorridorB box         : PA "perfectly safe", Fear creeps; the loop is allowed only in this beat
//   loop hints            : after loop 1 / 2 / 4; no changed poster at all -> the loop lets go after 3
//   wisp (beat 1.8a)      : E -> Fear drains to 0, "Return to your desk."
//   rewind                : "CUT. AGAIN." / "TAKE n" slate over the rewind's black
// Act 2 doorway (overnight run 2):
//   back at desk #4471    : after the wisp + training -> beat 1.10: 12:00, PA "Lunch is served.", Act 2 tasks
//   repeating PA (2.1)    : "Lunch is served." word for word, twice more, then now and then all Act 2
//   check in (2.2)        : finishing a story NPC's "Lunch" conversation = one coworker checked in (3 needed)
//   frozen clocks (2.6)   : the clock reaches 2:14 and stops; PA, a shiver of Fear that never quite leaves
#include "TakeGrabComponent.h"
#include "DirectorsCut_58.h"
#include "TakeGameInstance.h"
#include "TakeFearDirector.h"
#include "TakeLoopTrigger.h"
#include "TakeRewindSubsystem.h"
#include "TakeWisp.h"
#include "TakeWidgets.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "TakePauseMenu.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

namespace
{
	/** 0 before In, ramps up over InTime, 1 until Out, ramps down over OutTime. */
	float Envelope(float T, float In, float InTime, float Out, float OutTime)
	{
		if (T < In) { return 0.f; }
		if (T < In + InTime) { return (T - In) / FMath::Max(InTime, 0.01f); }
		if (T < Out) { return 1.f; }
		return FMath::Clamp(1.f - (T - Out) / FMath::Max(OutTime, 0.01f), 0.f, 1.f);
	}
}

void UTakeGrabComponent::BindStoryEvents()
{
	UWorld* World = GetWorld();
	if (!World) { return; }
	if (UTakeRewindSubsystem* Rewind = World->GetSubsystem<UTakeRewindSubsystem>())
	{
		Rewind->OnRewind.AddUniqueDynamic(this, &UTakeGrabComponent::HandleRewind);
	}
	for (TActorIterator<ATakeLoopTrigger> It(World); It; ++It)
	{
		It->OnLooped.AddUniqueDynamic(this, &UTakeGrabComponent::HandleLooped);
	}
}

ATakeFearDirector* UTakeGrabComponent::FindFearDirector() const
{
	UWorld* World = GetWorld();
	if (!World) { return nullptr; }
	TActorIterator<ATakeFearDirector> It(World);
	return It ? *It : nullptr;
}

void UTakeGrabComponent::PlaySfx2D(const TCHAR* Name, float Volume)
{
	if (USoundBase* Snd = LoadObject<USoundBase>(nullptr, *FString::Printf(TEXT("/Game/Audio/Placeholder/%s.%s"), Name, Name)))
	{
		UGameplayStatics::PlaySound2D(this, Snd, Volume);
	}
}

void UTakeGrabComponent::QueueSubtitle(const FString& Who, const FString& Line)
{
	if (SubtitleLeft <= 0.f && SubQueue.Num() == 0)
	{
		ShowSubtitle(Who, Line, FMath::Max(2.5f, Line.Len() / 14.f));
		return;
	}
	SubQueue.Add(Line);
	while (SubQueueWho.Num() < SubQueue.Num() - 1) { SubQueueWho.Add(FString()); }   // keep the two lists in step
	SubQueueWho.Add(Who);
}

// ------------------------------------------------------------------ Slate (cold open + rewind)
void UTakeGrabComponent::StartSlate(bool bRewind)
{
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	SlateTake = GI ? GI->TakeNumber : 1;
	SlateT = 0.f;
	bSlateRewind = bRewind;
	bSlateTimeCard = false;
	bSlateClap = false;
	bSlatePA = false;
	if (Prompt) { Prompt->SetSlate(1.f, FString(), 0.f); }   // black from the first frame
	if (!bRewind) { SetColdOpenFade(1.f); }   // and the camera itself, in case the HUD isn't drawn yet
}

void UTakeGrabComponent::SetColdOpenFade(float Amount)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC || !PC->PlayerCameraManager) { return; }
	if (Amount > 0.f) { PC->PlayerCameraManager->SetManualCameraFade(Amount, FLinearColor::Black, false); }
	else { PC->PlayerCameraManager->StopCameraFade(); }
}

void UTakeGrabComponent::ShowTimeCard(const FString& Time, const FString& Scene)
{
	// Between times of day: quick black, a clapper, "12:00 PM / SCENE 2 · LUNCH", then straight back in.
	SlateT = 0.f;
	bSlateRewind = false;
	bSlateTimeCard = true;
	bSlateClap = false;
	bSlatePA = true;   // no cold-open PA line
	TimeCardText = FString::Printf(TEXT("%s\n%s"), *Time, *Scene);
	if (Prompt) { Prompt->SetSlate(0.f, FString(), 0.f); }
}

void UTakeGrabComponent::HandleRewind(int32 NewTakeNumber)
{
	StartSlate(true);
	SlateTake = NewTakeNumber;
}

void UTakeGrabComponent::TickSlate(float DeltaTime)
{
	if (SlateT < 0.f) { return; }
	// The first frames after loading the map can take seconds: don't let that skip the black
	SlateT += FMath::Min(DeltaTime, 1.f / 30.f);
	const FString SlateLabel = FString::Printf(TEXT("TAKE %d"), SlateTake);
	if (bSlateTimeCard)
	{
		// 0-0.25 s black in, clapper, card until 1.9 s, fade back to the scene by 2.5 s
		if (!bSlateClap && SlateT >= 0.3f) { bSlateClap = true; PlaySfx2D(TEXT("sfx_clapper_snap"), 0.7f); }
		const float Black = SlateT < 0.25f ? SlateT / 0.25f : (SlateT < 1.9f ? 1.f : FMath::Clamp(1.f - (SlateT - 1.9f) / 0.6f, 0.f, 1.f));
		if (Prompt) { Prompt->SetSlate(Black, TimeCardText, Envelope(SlateT, 0.3f, 0.25f, 1.7f, 0.3f)); }
		if (SlateT > 2.5f)
		{
			SlateT = -1.f;
			bSlateTimeCard = false;
			if (Prompt) { Prompt->SetSlate(0.f, FString(), 0.f); }
		}
		return;
	}
	if (!bSlateRewind)
	{
		// Cold open: black, a clapper snap, "TAKE n" for ~2 s, fade in at your desk, then the PA
		if (!bSlateClap && SlateT >= 0.4f) { bSlateClap = true; PlaySfx2D(TEXT("sfx_clapper_snap"), 0.8f); }
		const float Black = SlateT < 2.6f ? 1.f : FMath::Clamp(1.f - (SlateT - 2.6f) / 1.2f, 0.f, 1.f);
		const FString ColdLabel = SlateTake <= 1 ? FString::Printf(TEXT("%s\n9:00 AM  \u00B7  SCENE 1  \u00B7  MORNING"), *SlateLabel) : SlateLabel;
		if (Prompt) { Prompt->SetSlate(Black, ColdLabel, Envelope(SlateT, 0.3f, 0.4f, 2.3f, 0.4f)); }
		SetColdOpenFade(Black);   // the camera fades with the HUD's black (covers the first frames, before the HUD draws)
		if (!bSlatePA && SlateT >= 3.2f)
		{
			bSlatePA = true;
			PlaySfx2D(TEXT("sfx_pa_chime"), 0.7f);
			ShowSubtitle(TEXT("PA"), TEXT("Good morning, team. The Chairman thanks you for your energy."), 5.f);
			if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this)))
			{
				if (GI->Objective.IsEmpty()) { GI->SetObjective(GI->ObjectiveForBeat()); }   // the cue queues after the PA line
			}
		}
		if (SlateT > 4.f)
		{
			SlateT = -1.f;
			if (Prompt) { Prompt->SetSlate(0.f, FString(), 0.f); }
			SetColdOpenFade(0.f);
		}
		return;
	}
	// Rewind: the screen is already black. "CUT. AGAIN." then "TAKE n", then the camera fades back in (at ~1.6 s)
	const bool bFirstCard = SlateT < 0.8f;
	if (!bSlateClap && SlateT >= 0.8f) { bSlateClap = true; PlaySfx2D(TEXT("sfx_clapper_snap"), 0.8f); }
	const float Black = SlateT < 1.6f ? 1.f : FMath::Clamp(1.f - (SlateT - 1.6f) / 0.5f, 0.f, 1.f);
	const float TextAlpha = bFirstCard ? Envelope(SlateT, 0.f, 0.15f, 0.65f, 0.15f) : Envelope(SlateT, 0.8f, 0.1f, 1.5f, 0.3f);
	if (Prompt) { Prompt->SetSlate(Black, bFirstCard ? FString(TEXT("CUT.  AGAIN.")) : SlateLabel, TextAlpha); }
	if (!bSlatePA && SlateT >= 2.2f)
	{
		bSlatePA = true;
		PlaySfx2D(TEXT("sfx_pa_chime"), 0.7f);
		ShowSubtitle(TEXT("PA"), TEXT("Thank you for your patience. We are going again from the top."), 4.5f);
	}
	if (SlateT > 2.4f)
	{
		SlateT = -1.f;
		if (Prompt) { Prompt->SetSlate(0.f, FString(), 0.f); }
	}
}

// ------------------------------------------------------------------ Beats
void UTakeGrabComponent::TickStory()
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI) { return; }
	// Morning complete (beat 1.5): the PA line comes from UpdateTasks; here the story moves on.
	// Also covers loading a save where the tasks were already done.
	if (GI->Beat == ETakeBeat::Morning && GI->MorningTasksDone())
	{
		GI->Beat = ETakeBeat::Meeting;
		GI->SetObjective(GI->ObjectiveForBeat());
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Beat -> Meeting"));
	}
	// Beat 1.10: back at your desk with the wisp, training done -> lunch (Act 2). TakeBeat 4 skips the walk.
	// Not while you're logged in (the terminal covers the subtitles): it plays as you stand up.
	if (GI->Beat == ETakeBeat::ReturnToDesk && GI->bTrainingComplete && SlateT < 0.f && !bSeated && !Inspecting && (GI->bDevLunchNow || IsNearMyDesk()))
	{
		StartLunch();
	}
}

void UTakeGrabComponent::OnStoryTalkFinished(const FString& RoleName, const FString& SetName)
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI) { return; }
	if (RoleName.Equals(TEXT("Receptionist"), ESearchCase::IgnoreCase) && SetName.IsEmpty() && !GI->bMetReceptionist)
	{
		GI->bMetReceptionist = true;   // the morning brief
		if (GI->Beat == ETakeBeat::Morning) { GI->SetObjective(GI->ObjectiveForBeat()); }
	}
	// Beat 2.2: "Check in with 3 coworkers" = finish a lunchtime conversation with each story NPC
	if (SetName == TEXT("Lunch") && GI->CurrentAct() >= 2 && GI->CoworkersCheckedIn < 3)
	{
		++GI->CoworkersCheckedIn;
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Checked in with %s (%d/3)"), *RoleName, GI->CoworkersCheckedIn);
		GI->SetObjective(GI->ObjectiveForBeat());
	}
}

void UTakeGrabComponent::OnBeatTrigger(FName TriggerName)
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI) { return; }
	ATakeFearDirector* FearDir = FindFearDirector();

	// Beat 1.5-1.6: the 10:00 meeting. Nobody's there.
	if (TriggerName == FName(TEXT("ConferenceRoom")) && GI->Beat == ETakeBeat::Meeting)
	{
		GI->Beat = ETakeBeat::Corridor;
		if (FearDir)
		{
			FearDir->FlickerBurst(2.5f);
			FearDir->StartCreep(FMath::Max(FearDir->Fear, 0.35f), 4.f);
		}
		ShowSubtitle(TEXT(""), TEXT("...Where is everyone?"), 3.5f);
		GI->SetObjective(GI->ObjectiveForBeat());   // "Something is at the end of Corridor B." (cue follows the line)
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Beat -> Corridor (the slip)"));
		return;
	}

	// Act 2: the 12:30 call isn't built yet (beat 2.3). Say so in the fiction, once.
	if (TriggerName == FName(TEXT("ConferenceRoom")) && GI->CurrentAct() >= 2 && !GI->bJoinedCall && !bLunchRoomNoted)
	{
		bLunchRoomNoted = true;
		ShowSubtitle(TEXT(""), TEXT("The big screen: SYNC (12:30). The organizer has started the meeting. (E on the screen to join.)"), 4.5f);
		return;
	}

	// Beat 1.6: Corridor B. The loop is allowed from now on (Fear Director, BeatLoopFear).
	if (TriggerName == FName(TEXT("CorridorB")) && GI->Beat == ETakeBeat::Corridor && !bCorridorEntered)
	{
		bCorridorEntered = true;
		PlaySfx2D(TEXT("sfx_pa_chime"), 0.6f);
		ShowSubtitle(TEXT("PA"), TEXT("Corridor B is perfectly safe. Please continue walking."), 4.f);
		if (FearDir) { FearDir->StartCreep(FMath::Max(FearDir->Fear, 0.55f), 8.f); }
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Entered Corridor B during the corridor beat"));
	}
}

void UTakeGrabComponent::HandleLooped(int32 Count)
{
	if (Count == 1) { QueueSubtitle(TEXT(""), TEXT("...Wasn't I just here?")); }
	else if (Count == 2) { QueueSubtitle(TEXT(""), TEXT("The posters. One of them wasn't like that before.")); }
	else if (Count == 4) { QueueSubtitle(TEXT(""), TEXT("Find the one that's wrong. Look closer.")); }

	// Mercy: if no loop has a changed poster to find (no _Changed materials imported), let the player through
	if (Count >= 3)
	{
		bool bAnyChanged = false;
		for (TActorIterator<ATakeLoopTrigger> It(GetWorld()); It; ++It) { if (It->CurrentChangedPoster) { bAnyChanged = true; } }
		if (!bAnyChanged)
		{
			for (TActorIterator<ATakeLoopTrigger> It(GetWorld()); It; ++It) { It->ForceBreak(); }
			QueueSubtitle(TEXT(""), TEXT("The hum drops. The corridor lets you through."));
		}
	}
}

bool UTakeGrabComponent::TryBreakLoopAt(AActor* Poster)
{
	if (!Poster || !Poster->ActorHasTag(TEXT("LoopPoster"))) { return false; }
	bool bBroke = false;
	for (TActorIterator<ATakeLoopTrigger> It(GetWorld()); It; ++It)
	{
		if (It->bLoopEnabled && It->TryBreakLoop(Poster)) { bBroke = true; }
	}
	if (bBroke)
	{
		PlaySfx2D(TEXT("sfx_lock_solved"), 0.5f);
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Loop broken by looking closer at %s"), *Poster->GetName());
	}
	return bBroke;
}

// ------------------------------------------------------------------ The wisp (beat 1.8a)
ATakeWisp* UTakeGrabComponent::WispInView() const
{
	FVector Loc; FRotator Rot;
	if (!GetView(Loc, Rot)) { return nullptr; }
	for (TActorIterator<ATakeWisp> It(GetWorld()); It; ++It)
	{
		if (!It->IsAvailable()) { continue; }
		const FVector To = It->GetActorLocation() - Loc;
		if (To.Size() <= UseRange + 50.f && FVector::DotProduct(Rot.Vector(), To.GetSafeNormal()) > 0.8f) { return *It; }
	}
	return nullptr;
}

void UTakeGrabComponent::CollectWisp(ATakeWisp* Wisp)
{
	if (!Wisp || !Wisp->IsAvailable()) { return; }
	Wisp->Collect();
	PlaySfx2D(TEXT("sfx_lock_solved"), 0.8f);
	if (ATakeFearDirector* FearDir = FindFearDirector()) { FearDir->Calm(2.5f); }
	if (UTakeGameInstance* WispGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { WispGI->AddEnergy(100.f); }   // soul energy: full
	if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->bGotWisp = true;
		if (GI->Beat == ETakeBeat::Corridor) { GI->Beat = ETakeBeat::ReturnToDesk; }
		GI->SetObjective(GI->ObjectiveForBeat());
	}
	ShowSubtitle(TEXT(""), TEXT("Warm light runs up your arm. The hum stops. The colour comes back."), 4.5f);
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Wisp collected -> ReturnToDesk"));
}

// ------------------------------------------------------------------ Act 2 doorway (overnight run 2)
bool UTakeGrabComponent::IsNearMyDesk()
{
	const AActor* Me = GetOwner();
	if (!Me) { return false; }
	if (!MyDesk.IsValid())
	{
		AActor* Found = nullptr;
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			if (It->ActorHasTag(TEXT("Desk4471"))) { Found = *It; break; }
			if (!Found && It->ActorHasTag(TEXT("Monitor4471"))) { Found = *It; }
		}
		if (!Found)
		{
			for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
			{
				if (It->PlayerStartTag == FName(TEXT("Desk4471"))) { Found = *It; break; }
			}
		}
		if (!Found) { return false; }
		MyDesk = Found;
	}
	return FVector::Dist2D(Me->GetActorLocation(), MyDesk->GetActorLocation()) <= DeskArriveRadius;
}

void UTakeGrabComponent::StartLunch()
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI || GI->Beat == ETakeBeat::Lunch) { return; }
	GI->Beat = ETakeBeat::Lunch;
	GI->bDevLunchNow = false;
	GI->WorkdayMinutes = 12.f * 60.f;   // the clock jumps: 12:00
	GI->bClocksFrozen = false;
	if (ATakeFearDirector* FearDir = FindFearDirector()) { FearDir->Calm(2.f); }
	// The jump cut: a time card, like a scene slate ("12:00 PM / SCENE 2: LUNCH")
	ShowTimeCard(TEXT("12:00 PM"), TEXT("SCENE 2  \u00B7  LUNCH"));
	QueueSubtitle(TEXT(""), TEXT("...Twelve? It was eleven a second ago."));
	LunchPAPlayed = 0;
	LunchPALeft = 1 + FMath::Max(0, LunchPAExtraRepeats);   // the first announcement + the word-for-word repeats
	LunchPAIn = 4.2f;   // after the card
	GI->SetObjective(GI->ObjectiveForBeat());   // "Join the 12:30 call"; the cue queues behind the PA
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Beat -> Lunch (beat 1.10, Act 2 starts)"));
}

void UTakeGrabComponent::PlayLunchPA()
{
	++LunchPAPlayed;
	PlaySfx2D(TEXT("sfx_pa_chime"), 0.7f);
	// Word for word, every time (beat 2.1). Shown now, even over another line: the PA doesn't wait.
	ShowSubtitle(TEXT("PA"), TEXT("Lunch is served."), FMath::Min(3.f, LunchPARepeatSeconds - 0.5f));
	if (LunchPAPlayed == 3) { QueueSubtitle(TEXT(""), TEXT("...Didn't they just say that?")); }
	if (LunchPAPlayed == 6) { QueueSubtitle(TEXT(""), TEXT("Same voice. Same breath before \"served\". Every time.")); }
}

void UTakeGrabComponent::TickAct2(float DeltaTime)
{
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI || GI->CurrentAct() < 2) { return; }
	if (!bAct2Revealed) { RevealAct2(); }   // lunch is out (also after loading an Act 2 save)
	// Beat 2.6: after the clocks stop, Fear never quite leaves (checked about once a second)
	FloorCheckIn -= DeltaTime;
	if (GI->bClocksFrozen && FloorCheckIn <= 0.f)
	{
		FloorCheckIn = 1.f;
		ATakeFearDirector* FearDir = FindFearDirector();
		if (FearDir && FearDir->TargetFear < FrozenFearFloor) { FearDir->StartCreep(FrozenFearFloor, 5.f); }
	}
	if (bSeated || Inspecting || PauseMenu) { return; }   // the PA waits until you can hear it
	if (LunchPAIn < 0.f)   // nothing planned (e.g. after loading an Act 2 save): the next one comes later
	{
		LunchPAIn = FMath::FRandRange(LunchPAGapMin, LunchPAGapMax);
	}
	LunchPAIn -= DeltaTime;
	if (LunchPAIn > 0.f) { return; }
	PlayLunchPA();
	if (LunchPALeft > 0) { --LunchPALeft; }
	LunchPAIn = LunchPALeft > 0 ? LunchPARepeatSeconds : FMath::FRandRange(LunchPAGapMin, LunchPAGapMax);
	if (LunchPALeft > 0) { return; }
	// Between the word-for-word bursts, it comes back as one line, or as all three again
	if (FMath::FRand() < 0.35f) { LunchPALeft = 1 + FMath::Max(0, LunchPAExtraRepeats); }
}

void UTakeGrabComponent::FreezeClocks()
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI || GI->bClocksFrozen) { return; }
	GI->bClocksFrozen = true;
	GI->WorkdayMinutes = FrozenClockMinutes;
	if (ATakeFearDirector* FearDir = FindFearDirector())
	{
		FearDir->FlickerBurst(1.2f);
		FearDir->StartCreep(FMath::Max(FearDir->Fear, FrozenFearFloor + 0.1f), 6.f);
	}
	ShowTimeCard(TEXT("2:14 PM"), TEXT("SCENE 3  \u00B7  HOLD"));
	PlaySfx2D(TEXT("sfx_pa_chime"), 0.7f);
	ShowSubtitle(TEXT("PA"), TEXT("The time is two fourteen. The time is two fourteen."), 4.5f);
	QueueSubtitle(TEXT(""), TEXT("The second hand isn't moving. None of them are."));
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Clocks frozen at 2:14 (beat 2.6)"));
}
