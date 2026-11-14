// TAKE 4471 - Act 1 progression (overnight run 1). Part of UTakeGrabComponent, kept in its own file:
//   cold open (beat 1.1)  : black, "TAKE n", clapper, PA chime + "Good morning, team."
//   objective             : GameInstance->Objective, drawn under the badge task list, "New objective" cue
//   morning complete      : -> "Attend the 10:00 meeting (Conference Room)"
//   ConferenceRoom box    : the slip (lights flicker, Fear ~0.35, "...Where is everyone?") -> Corridor B
//   CorridorB box         : PA "perfectly safe", Fear creeps; the loop is allowed only in this beat
//   loop hints            : after loop 1 / 2 / 4; no changed poster at all -> the loop lets go after 3
//   wisp (beat 1.8a)      : E -> Fear drains to 0, "Return to your desk."
//   rewind                : "CUT. AGAIN." / "TAKE n" slate over the rewind's black
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
	for (TActorIterator<ATakeFearDirector> It(GetWorld()); It; ++It) { return *It; }
	return nullptr;
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
	bSlateClap = false;
	bSlatePA = false;
	if (Prompt) { Prompt->SetSlate(1.f, FString(), 0.f); }   // black from the first frame
}

void UTakeGrabComponent::HandleRewind(int32 NewTakeNumber)
{
	StartSlate(true);
	SlateTake = NewTakeNumber;
}

void UTakeGrabComponent::TickSlate(float DeltaTime)
{
	if (SlateT < 0.f) { return; }
	SlateT += DeltaTime;
	const FString SlateLabel = FString::Printf(TEXT("TAKE %d"), SlateTake);
	if (!bSlateRewind)
	{
		// Cold open: black, a clapper snap, "TAKE n" for ~2 s, fade in at your desk, then the PA
		if (!bSlateClap && SlateT >= 0.4f) { bSlateClap = true; PlaySfx2D(TEXT("sfx_clapper_snap"), 0.8f); }
		const float Black = SlateT < 2.6f ? 1.f : FMath::Clamp(1.f - (SlateT - 2.6f) / 1.2f, 0.f, 1.f);
		if (Prompt) { Prompt->SetSlate(Black, SlateLabel, Envelope(SlateT, 0.3f, 0.4f, 2.3f, 0.4f)); }
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
	if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->bGotWisp = true;
		if (GI->Beat == ETakeBeat::Corridor) { GI->Beat = ETakeBeat::ReturnToDesk; }
		GI->SetObjective(GI->ObjectiveForBeat());
	}
	ShowSubtitle(TEXT(""), TEXT("Warm light runs up your arm. The hum stops. The colour comes back."), 4.5f);
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Wisp collected -> ReturnToDesk"));
}
