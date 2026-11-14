// TAKE 4471 - mannequin coworkers
#include "TakeCoworker.h"
#include "DirectorsCut_58.h"
#include "TakeTypes.h"
#include "TakeFearDirector.h"
#include "TakeGrabComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "Engine/TargetPoint.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ATakeCoworker::ATakeCoworker()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCapsuleComponent()->InitCapsuleSize(32.f, 90.f);
	GetMesh()->SetHiddenInGame(true);

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(GetCapsuleComponent());
	// Mesh pivot at the feet. Our Blender props face local +Y in Unreal, a Character walks along +X: turn the body -90.
	Body->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -90.f), FRotator(0.f, -90.f, 0.f));
	Body->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Body->SetCollisionResponseToAllChannels(ECR_Ignore);
	Body->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);   // so E / look traces find us

	Speech = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Speech"));
	Speech->SetupAttachment(GetCapsuleComponent());
	Speech->SetRelativeLocation(FVector(0.f, 0.f, 105.f));
	Speech->SetHorizontalAlignment(EHTA_Center);
	Speech->SetVerticalAlignment(EVRTA_TextBottom);
	Speech->SetWorldSize(14.f);
	Speech->SetTextRenderColor(FColor(30, 40, 48));
	Speech->SetText(FText::GetEmpty());

	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->MaxWalkSpeed = WalkSpeed;
	Move->bOrientRotationToMovement = true;
	Move->RotationRate = FRotator(0.f, 240.f, 0.f);
	bUseControllerRotationYaw = false;
}

void ATakeCoworker::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (BodyMesh) { Body->SetStaticMesh(BodyMesh); }
}

void ATakeCoworker::BeginPlay()
{
	Super::BeginPlay();
	if (BodyMesh && Body->GetStaticMesh() != BodyMesh) { Body->SetStaticMesh(BodyMesh); }
	Home = GetActorLocation();
	HomeRot = GetActorRotation();
	LastPos = Home;
	Phase = FMath::FRandRange(0.f, 10.f);
	StateTimer = FMath::FRandRange(WorkMin * 0.3f, WorkMax);   // stagger, so they don't all get up at once
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	for (TActorIterator<ATargetPoint> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(TEXT("WorkSpot"))) { Spots.Add(*It); }
	}
	for (TActorIterator<ATakeFearDirector> It(GetWorld()); It; ++It) { FearDirector = *It; break; }
	LoadLines();
}

void ATakeCoworker::LoadLines()
{
	Lines.Reset();
	const FString Want = Role.Equals(TEXT("Receptionist"), ESearchCase::IgnoreCase) ? TEXT("Coworker") : Role;
	if (const UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_NPCLines.DT_NPCLines")))
	{
		TArray<FTakeNPCLineRow*> Rows;
		Table->GetAllRows<FTakeNPCLineRow>(TEXT("TakeCoworker"), Rows);
		for (const FTakeNPCLineRow* R : Rows)
		{
			if (R && R->Act == 1 && R->Role.Equals(Want, ESearchCase::IgnoreCase) && (!R->IsAgent || bIsAgent))
			{
				Lines.Add(R->Line);
			}
		}
	}
	if (Lines.Num() == 0)   // fallback until the DataTable is imported
	{
		Lines = { TEXT("Oh! Sorry, didn't see you there."), TEXT("Big day. Everyone knows their marks."),
		          TEXT("Have you done your compliance training?"), TEXT("Coffee machine's making that noise again."),
		          TEXT("Love the energy today, {EMP}."), TEXT("Is it still morning? Feels like it's been morning for a while.") };
		if (Role.Equals(TEXT("Guard"), ESearchCase::IgnoreCase)) { Lines = { TEXT("Badge, please."), TEXT("Keep it moving."), TEXT("Corridor B is off limits.") }; }
		if (Role.Equals(TEXT("Custodian"), ESearchCase::IgnoreCase)) { Lines = { TEXT("Mind the floor. Wet."), TEXT("I clean this hall every night. Same scuffs every morning."), TEXT("Don't go in my closet.") }; }
	}
}

float ATakeCoworker::GetFear() const
{
	return FearDirector.IsValid() ? FearDirector->Fear : 0.f;
}

bool ATakeCoworker::Speak(const FString& Context)
{
	if (CooldownLeft > 0.f || Lines.Num() == 0) { return false; }
	SayLine(Lines[FMath::RandRange(0, Lines.Num() - 1)]);
	return true;
}

void ATakeCoworker::SayLine(const FString& Line)
{
	const FString Filled = TakeText::Fill(this, Line);
	Speech->SetText(FText::FromString(Filled));
	SpeechLeft = SpeechSeconds;
	CooldownLeft = SpeechCooldown;
	if (APawn* P = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (UTakeGrabComponent* Hands = P->FindComponentByClass<UTakeGrabComponent>())
		{
			Hands->ShowSubtitle(Role, Filled, SpeechSeconds + 0.5f);
		}
	}
	// Turn to face the player while talking
	if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (State == ETakeCoworkerState::Working || State == ETakeCoworkerState::AtSpot)
		{
			FRotator Face = (Player->GetActorLocation() - GetActorLocation()).Rotation();
			SetActorRotation(FRotator(0.f, Face.Yaw, 0.f));
		}
	}
}

void ATakeCoworker::GoTo(const FVector& Where, ETakeCoworkerState NewState)
{
	State = NewState;
	Target = Where;
	StuckTimer = 0.f;
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->MoveToLocation(Where, 40.f, /*bStopOnOverlap=*/true, /*bUsePathfinding=*/true, /*bProjectDestinationToNavigation=*/true);
	}
}

bool ATakeCoworker::PickSpot(FVector& Out)
{
	// Mostly the coffee machine / printer / cooler; sometimes a random walk nearby.
	TArray<AActor*> Live;
	for (const TWeakObjectPtr<AActor>& S : Spots) { if (S.IsValid()) { Live.Add(S.Get()); } }
	if (Live.Num() > 0 && FMath::FRand() < 0.8f)
	{
		Out = Live[FMath::RandRange(0, Live.Num() - 1)]->GetActorLocation();
		Out += FVector(FMath::FRandRange(-60.f, 60.f), FMath::FRandRange(-60.f, 60.f), 0.f);
		return true;
	}
	if (UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		FNavLocation P;
		if (Nav->GetRandomReachablePointInRadius(Home, 900.f, P)) { Out = P.Location; return true; }
	}
	return false;
}

bool ATakeCoworker::PlayerCanSee(const APawn* Player) const
{
	FVector Eye; FRotator View;
	Player->GetActorEyesViewPoint(Eye, View);
	const FVector To = (GetActorLocation() - Eye).GetSafeNormal();
	return FVector::DotProduct(View.Vector(), To) > 0.55f;
}

void ATakeCoworker::Animate(float DeltaSeconds, bool bMoving)
{
	Phase += DeltaSeconds;
	// Body faces local +Y, so Roll = lean forward/back and Pitch = sway side to side.
	FRotator R(0.f, -90.f, 0.f);
	FVector L(0.f, 0.f, -90.f);
	switch (State)
	{
	case ETakeCoworkerState::Working:   // typing, checking email: small lean and head-nod rhythm
		R.Roll = 2.f * FMath::Sin(Phase * 7.f) * FMath::Max(0.f, FMath::Sin(Phase * 0.9f));
		R.Yaw += 4.f * FMath::Sin(Phase * 0.35f);   // glancing between screens
		break;
	case ETakeCoworkerState::AtSpot:    // waiting for the coffee: shifting weight
		R.Pitch = 2.5f * FMath::Sin(Phase * 1.3f);
		break;
	case ETakeCoworkerState::Watching:  // perfectly still
		break;
	default:
		if (bMoving) { L.Z += 2.5f * FMath::Abs(FMath::Sin(Phase * 9.f)); R.Pitch = 2.f * FMath::Sin(Phase * 4.5f); }
		break;
	}
	Body->SetRelativeLocationAndRotation(L, R);
}

void ATakeCoworker::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CooldownLeft = FMath::Max(0.f, CooldownLeft - DeltaSeconds);
	if (SpeechLeft > 0.f)
	{
		SpeechLeft -= DeltaSeconds;
		if (SpeechLeft <= 0.f) { Speech->SetText(FText::GetEmpty()); }
	}

	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (Player)
	{
		// Speech text faces the camera
		FVector Eye; FRotator View;
		Player->GetActorEyesViewPoint(Eye, View);
		Speech->SetWorldRotation(FRotator(0.f, (Eye - Speech->GetComponentLocation()).Rotation().Yaw, 0.f));
		// Bump
		if (FVector::Dist2D(Player->GetActorLocation(), GetActorLocation()) < 95.f) { Speak(TEXT("Bump")); }
	}

	AAIController* AI = Cast<AAIController>(GetController());
	const float Fear = GetFear();
	const float Watch = bIsAgent ? WatchFear - 0.15f : WatchFear;

	// --- Augmented Fear: stop pretending ---------------------------------------
	if (Player && Fear >= Watch)
	{
		if (State != ETakeCoworkerState::Watching)
		{
			State = ETakeCoworkerState::Watching;
			Speech->SetText(FText::GetEmpty());
		}
		if (PlayerCanSee(Player))
		{
			if (AI) { AI->StopMovement(); }
			const FRotator Face = (Player->GetActorLocation() - GetActorLocation()).Rotation();
			SetActorRotation(FRotator(0.f, Face.Yaw, 0.f));   // it was always looking at you
		}
		else if (FVector::Dist2D(Player->GetActorLocation(), GetActorLocation()) > 140.f && AI)
		{
			GetCharacterMovement()->MaxWalkSpeed = CreepSpeed * (bIsAgent ? 2.f : 1.f);
			AI->MoveToActor(Player, 120.f);
		}
		Animate(DeltaSeconds, false);
		return;
	}
	if (State == ETakeCoworkerState::Watching)   // fear dropped: go back to work as if nothing happened
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		GoTo(Home, ETakeCoworkerState::Returning);
	}

	// --- Ordinary workday ---------------------------------------------------------
	const bool bMoving = GetVelocity().SizeSquared2D() > 25.f;
	StateTimer -= DeltaSeconds;
	switch (State)
	{
	case ETakeCoworkerState::Working:
		if (bWanders && StateTimer <= 0.f)
		{
			FVector Spot;
			if (PickSpot(Spot)) { GoTo(Spot, ETakeCoworkerState::Walking); }
			else { StateTimer = FMath::FRandRange(WorkMin, WorkMax); }
		}
		break;
	case ETakeCoworkerState::Walking:
	case ETakeCoworkerState::Returning:
	{
		const bool bArrived = FVector::Dist2D(GetActorLocation(), Target) < 80.f
			|| (AI && AI->GetMoveStatus() == EPathFollowingStatus::Idle && StuckTimer > 1.f);
		StuckTimer += DeltaSeconds;
		if (FVector::DistSquared2D(LastPos, GetActorLocation()) > 4.f) { LastPos = GetActorLocation(); }
		const bool bGaveUp = StuckTimer > 20.f;
		if (bArrived || bGaveUp)
		{
			if (State == ETakeCoworkerState::Walking)
			{
				State = ETakeCoworkerState::AtSpot;
				StateTimer = FMath::FRandRange(4.f, 10.f);
			}
			else
			{
				if (bGaveUp) { SetActorLocation(Home); }   // nobody saw that
				SetActorRotation(HomeRot);
				State = ETakeCoworkerState::Working;
				StateTimer = FMath::FRandRange(WorkMin, WorkMax);
			}
		}
		break;
	}
	case ETakeCoworkerState::AtSpot:
		if (StateTimer <= 0.f) { GoTo(Home, ETakeCoworkerState::Returning); }
		break;
	default:
		break;
	}
	Animate(DeltaSeconds, bMoving);
}
