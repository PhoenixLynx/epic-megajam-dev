// TAKE 4471 - rewind on capture
#include "TakeRewindSubsystem.h"
#include "DirectorsCut_58.h"
#include "TakeGameInstance.h"
#include "TakeFearDirector.h"
#include "TakeStalker.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UTakeRewindSubsystem::Rewind()
{
	if (bRewinding) { return; }
	bRewinding = true;
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		if (APawn* Pawn = PC->GetPawn()) { Pawn->DisableInput(PC); }
		if (PC->PlayerCameraManager) { PC->PlayerCameraManager->StartCameraFade(0.f, 1.f, 0.15f, FLinearColor::Black, false, true); }
	}
	if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { GI->TakeNumber++; }

	FTimerHandle T;
	GetWorld()->GetTimerManager().SetTimer(T, this, &UTakeRewindSubsystem::DoReset, 0.2f, false);
}

void UTakeRewindSubsystem::DoReset()
{
	UWorld* World = GetWorld();
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	const int32 Take = GI ? GI->TakeNumber : 0;
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] CUT. AGAIN. -> Take %d"), Take);
	OnRewind.Broadcast(Take);   // show the slate now (screen is black)

	for (TActorIterator<ATakeFearDirector> It(World); It; ++It) { It->SetFearImmediate(0.f); }
	for (TActorIterator<ATakeStalker> It(World); It; ++It) { It->Deactivate(); }

	// Back to the desk
	APlayerStart* Start = nullptr;
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (!Start || It->PlayerStartTag == TEXT("Desk4471")) { Start = *It; }
	}
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC && PC->GetPawn() && Start)
	{
		PC->GetPawn()->SetActorLocation(Start->GetActorLocation(), false, nullptr, ETeleportType::ResetPhysics);
		PC->SetControlRotation(Start->GetActorRotation());
	}

	// Something changed: turn one tagged object (mug, chair...) a little
	TArray<AActor*> Changeable;
	UGameplayStatics::GetAllActorsWithTag(this, TEXT("ChangeOnRewind"), Changeable);
	if (Changeable.Num() > 0)
	{
		AActor* A = Changeable[FMath::RandRange(0, Changeable.Num() - 1)];
		A->AddActorWorldRotation(FRotator(0.f, 90.f + FMath::FRandRange(-20.f, 20.f), 0.f), false, nullptr, ETeleportType::ResetPhysics);
	}

	FTimerHandle T;
	World->GetTimerManager().SetTimer(T, this, &UTakeRewindSubsystem::FinishRewind, SlateSeconds, false);
}

void UTakeRewindSubsystem::FinishRewind()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (PC->PlayerCameraManager) { PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, 0.6f, FLinearColor::Black, false, false); }
		if (APawn* Pawn = PC->GetPawn()) { Pawn->EnableInput(PC); }
	}
	bRewinding = false;
}
