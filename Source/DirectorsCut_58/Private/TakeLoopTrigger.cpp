// TAKE 4471 - Corridor B loop
#include "TakeLoopTrigger.h"
#include "DirectorsCut_58.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"

ATakeLoopTrigger::ATakeLoopTrigger()
{
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetBoxExtent(FVector(190.f, 20.f, 150.f));
	Box->SetCollisionProfileName(TEXT("Trigger"));
	RootComponent = Box;
}

void ATakeLoopTrigger::BeginPlay()
{
	Super::BeginPlay();
	Box->OnComponentBeginOverlap.AddDynamic(this, &ATakeLoopTrigger::OnBoxOverlap);

	// Auto-setup: posters tagged LoopPoster (the grey-box builder tags GB_Poster_0..5), sorted by name
	if (Posters.Num() == 0)
	{
		TArray<AActor*> Found;
		UGameplayStatics::GetAllActorsWithTag(this, TEXT("LoopPoster"), Found);
		Found.Sort([](const AActor& A, const AActor& B) { return A.GetName() < B.GetName(); });
		for (AActor* A : Found) { if (AStaticMeshActor* SMA = Cast<AStaticMeshActor>(A)) { Posters.Add(SMA); } }
	}
	// Auto-setup: each poster's changed twin = same material path + "_Changed"
	if (ChangedMaterials.Num() == 0)
	{
		for (AStaticMeshActor* P : Posters)
		{
			UMaterialInterface* Changed = nullptr;
			if (P && P->GetStaticMeshComponent() && P->GetStaticMeshComponent()->GetMaterial(0))
			{
				const FString Path = P->GetStaticMeshComponent()->GetMaterial(0)->GetPathName();   // /Game/.../MI_Poster_X.MI_Poster_X
				FString Package, Object;
				if (Path.Split(TEXT("."), &Package, &Object))
				{
					Changed = LoadObject<UMaterialInterface>(nullptr, *(Package + TEXT("_Changed.") + Object + TEXT("_Changed")));
				}
			}
			ChangedMaterials.Add(Changed);
		}
	}
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Loop trigger: %d posters"), Posters.Num());
	OriginalMaterials.Reset();
	for (AStaticMeshActor* P : Posters)
	{
		OriginalMaterials.Add(P && P->GetStaticMeshComponent() ? P->GetStaticMeshComponent()->GetMaterial(0) : nullptr);
	}
}

void ATakeLoopTrigger::SetLoopEnabled(bool bEnable)
{
	if (bBroken) { bEnable = false; }
	if (bLoopEnabled != bEnable)
	{
		bLoopEnabled = bEnable;
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Corridor loop %s"), bEnable ? TEXT("ON") : TEXT("OFF"));
	}
}

void ATakeLoopTrigger::OnBoxOverlap(UPrimitiveComponent*, AActor* Other, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!bLoopEnabled || bBroken) { return; }
	APawn* Pawn = Cast<APawn>(Other);
	if (!Pawn || !Pawn->IsPlayerControlled()) { return; }

	// Teleport keeps velocity and view direction, so the jump can't be felt.
	Pawn->SetActorLocation(Pawn->GetActorLocation() + LoopOffset, false, nullptr, ETeleportType::TeleportPhysics);
	++LoopCount;
	ChangeOnePoster();
	OnLooped.Broadcast(LoopCount);
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Looped (%d)"), LoopCount);
}

void ATakeLoopTrigger::RestorePosters()
{
	for (int32 I = 0; I < Posters.Num(); ++I)
	{
		if (Posters[I] && OriginalMaterials.IsValidIndex(I) && OriginalMaterials[I])
		{
			Posters[I]->GetStaticMeshComponent()->SetMaterial(0, OriginalMaterials[I]);
		}
	}
}

void ATakeLoopTrigger::ChangeOnePoster()
{
	RestorePosters();
	CurrentChangedPoster = nullptr;
	if (Posters.Num() == 0) { return; }
	const int32 Index = (LoopCount * 7 + 3) % Posters.Num();   // jumps around, never the same twice in a row
	if (Posters[Index] && ChangedMaterials.IsValidIndex(Index) && ChangedMaterials[Index])
	{
		Posters[Index]->GetStaticMeshComponent()->SetMaterial(0, ChangedMaterials[Index]);
		CurrentChangedPoster = Posters[Index];
	}
}

bool ATakeLoopTrigger::TryBreakLoop(AActor* InteractedPoster)
{
	if (bBroken || !CurrentChangedPoster || InteractedPoster != CurrentChangedPoster) { return false; }
	bBroken = true;
	bLoopEnabled = false;
	RestorePosters();
	OnLoopBroken.Broadcast();
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Loop broken after %d loops"), LoopCount);
	return true;
}

void ATakeLoopTrigger::ResetLoop()
{
	bBroken = false;
	LoopCount = 0;
	CurrentChangedPoster = nullptr;
	RestorePosters();
}
