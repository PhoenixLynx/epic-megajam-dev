// TAKE 4471 - run state + console commands
#include "TakeGameInstance.h"
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
#include "HAL/PlatformTime.h"

void UTakeGameInstance::Init()
{
	Super::Init();
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
	for (TActorIterator<ATakeFearDirector> It(GetWorld()); It; ++It) { It->StartCreep(Target, Seconds); }
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
	default:                      return FString();
	}
}

void UTakeGameInstance::TakeBeat(int32 NewBeat)
{
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
	Beat = static_cast<ETakeBeat>(Clamped);
	bGotWisp = Beat == ETakeBeat::ReturnToDesk;
	SetObjective(ObjectiveForBeat());
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Beat -> %d"), Clamped);
}
