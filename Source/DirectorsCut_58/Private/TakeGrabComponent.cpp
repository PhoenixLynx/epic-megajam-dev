// TAKE 4471 - grab / throw / telekinesis
#include "TakeGrabComponent.h"
#include "DirectorsCut_58.h"
#include "TakeGameInstance.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UTakeGrabComponent::UTakeGrabComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTakeGrabComponent::BeginPlay()
{
	Super::BeginPlay();
	AActor* Owner = GetOwner();
	Handle = Owner->FindComponentByClass<UPhysicsHandleComponent>();
	if (!Handle)
	{
		Handle = NewObject<UPhysicsHandleComponent>(Owner, TEXT("TakePhysicsHandle"));
		Handle->RegisterComponent();
	}
	if (bAutoBindKeys)
	{
		// Input component exists once the pawn is possessed; try now and shortly after.
		BindKeys();
		FTimerHandle T;
		GetWorld()->GetTimerManager().SetTimer(T, this, &UTakeGrabComponent::BindKeys, 0.5f, false);
	}
}

void UTakeGrabComponent::BindKeys()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn || !Pawn->InputComponent) { return; }
	static const FName Bound(TEXT("TakeGrabBound"));
	if (Pawn->Tags.Contains(Bound)) { return; }
	Pawn->Tags.Add(Bound);
	Pawn->InputComponent->BindKey(EKeys::E, IE_Pressed, this, &UTakeGrabComponent::TryGrabOrDropInput);
	Pawn->InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &UTakeGrabComponent::Throw);
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Grab keys bound: E = grab/drop, Right Mouse = throw"));
}

bool UTakeGrabComponent::GetView(FVector& OutLoc, FRotator& OutRot) const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) { return false; }
	Pawn->GetActorEyesViewPoint(OutLoc, OutRot);
	return true;
}

int32 UTakeGrabComponent::GetSoulBits() const
{
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	return GI ? GI->SoulBits : 0;
}

float UTakeGrabComponent::CurrentRange() const
{
	return GetSoulBits() >= 1 ? TelekinesisRange : GrabRange;
}

bool UTakeGrabComponent::IsHolding() const
{
	return Handle && Handle->GetGrabbedComponent() != nullptr;
}

bool UTakeGrabComponent::TryGrabOrDrop()
{
	if (IsHolding()) { Drop(); return false; }

	FVector Loc; FRotator Rot;
	if (!GetView(Loc, Rot)) { return false; }
	const FVector End = Loc + Rot.Vector() * CurrentRange();
	FCollisionQueryParams Params(SCENE_QUERY_STAT(TakeGrab), false, GetOwner());
	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Loc, End, ECC_Visibility, Params)) { return false; }

	UPrimitiveComponent* Comp = Hit.GetComponent();
	if (!Comp || !Comp->IsSimulatingPhysics()) { return false; }
	if (Comp->GetMass() > MaxMassWithoutTelekinesis && GetSoulBits() < 3) { return false; }

	Handle->GrabComponentAtLocationWithRotation(Comp, NAME_None, Hit.ImpactPoint, Comp->GetComponentRotation());
	OnGrabbed.Broadcast(Comp);
	return true;
}

void UTakeGrabComponent::Drop()
{
	if (Handle) { Handle->ReleaseComponent(); }
}

void UTakeGrabComponent::Throw()
{
	if (!IsHolding()) { return; }
	UPrimitiveComponent* Comp = Handle->GetGrabbedComponent();
	FVector Loc; FRotator Rot;
	GetView(Loc, Rot);
	Handle->ReleaseComponent();
	Comp->AddImpulse(Rot.Vector() * ThrowStrength, NAME_None, /*bVelChange=*/true);
	OnThrown.Broadcast(Comp);
}

void UTakeGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!IsHolding()) { return; }
	FVector Loc; FRotator Rot;
	if (GetView(Loc, Rot))
	{
		Handle->SetTargetLocationAndRotation(Loc + Rot.Vector() * HoldDistance, Rot);
	}
}
