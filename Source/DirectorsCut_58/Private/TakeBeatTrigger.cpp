// TAKE 4471 - story beat trigger boxes
#include "TakeBeatTrigger.h"
#include "DirectorsCut_58.h"
#include "TakeGrabComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"

ATakeBeatTrigger::ATakeBeatTrigger()
{
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetBoxExtent(BoxHalfSize);
	Box->SetCollisionProfileName(TEXT("Trigger"));
	RootComponent = Box;
}

void ATakeBeatTrigger::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (Box) { Box->SetBoxExtent(BoxHalfSize); }
}

void ATakeBeatTrigger::BeginPlay()
{
	Super::BeginPlay();
	Box->SetBoxExtent(BoxHalfSize);
	Box->OnComponentBeginOverlap.AddDynamic(this, &ATakeBeatTrigger::OnBoxOverlap);
}

void ATakeBeatTrigger::OnBoxOverlap(UPrimitiveComponent*, AActor* Other, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	APawn* Pawn = Cast<APawn>(Other);
	if (!Pawn || !Pawn->IsPlayerControlled()) { return; }
	if (UTakeGrabComponent* Hands = Pawn->FindComponentByClass<UTakeGrabComponent>())
	{
		Hands->OnBeatTrigger(BeatName);
	}
}
