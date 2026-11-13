// TAKE 4471 - run state + console commands
#include "TakeGameInstance.h"
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
