// TAKE 4471 - mannequin coworkers
#include "TakeCoworker.h"
#include "DirectorsCut_58.h"
#include "TakeTypes.h"
#include "TakeFearDirector.h"
#include "TakeGrabComponent.h"
#include "TakeGameInstance.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/TargetPoint.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

double ATakeCoworker::NextAnyoneMayTalk = 0.0;

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
	Speech->SetRelativeLocation(FVector(0.f, 0.f, 98.f));
	Speech->SetHorizontalAlignment(EHTA_Center);
	Speech->SetVerticalAlignment(EVRTA_TextBottom);
	Speech->SetWorldSize(9.f);
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
	if (bSitsAtDesk)
	{
		bWanders = false;
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);   // the chair under them
	}
	SetupSkeletal();
	LoadLines();
}

void ATakeCoworker::SetupSkeletal()
{
	const TCHAR* MeshName = NPCRole.Equals(TEXT("Guard"), ESearchCase::IgnoreCase) ? TEXT("SK_MannequinGuard")
		: NPCRole.Equals(TEXT("Custodian"), ESearchCase::IgnoreCase) ? TEXT("SK_MannequinCustodian") : TEXT("SK_Mannequin");
	auto Load = [](const TCHAR* Name) -> UAnimSequence*
	{
		return LoadObject<UAnimSequence>(nullptr, *FString::Printf(TEXT("/Game/Characters/Mannequin/%s.%s"), Name, Name));
	};
	USkeletalMesh* SK = LoadObject<USkeletalMesh>(nullptr, *FString::Printf(TEXT("/Game/Characters/Mannequin/%s.%s"), MeshName, MeshName));
	AnimIdle = Load(TEXT("AS_Idle"));
	AnimWalk = Load(TEXT("AS_Walk"));
	AnimSit = Load(TEXT("AS_SitType"));
	AnimStare = Load(TEXT("AS_Stare"));
	if (!SK || !AnimIdle) { return; }   // not imported yet: keep the static mannequin
	bSkeletal = true;
	USkeletalMeshComponent* M = GetMesh();
	M->SetSkeletalMeshAsset(SK);
	M->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -90.f), FRotator(0.f, -90.f, 0.f));
	M->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	M->SetHiddenInGame(false);
	Body->SetHiddenInGame(true);   // still there for E / line-of-sight traces
	PlayAnim(bSitsAtDesk && AnimSit ? AnimSit.Get() : AnimIdle.Get());
}

void ATakeCoworker::PlayAnim(UAnimSequence* Anim)
{
	if (!bSkeletal || !Anim || Anim == AnimPlaying) { return; }
	AnimPlaying = Anim;
	GetMesh()->PlayAnimation(Anim, /*bLooping=*/true);
	GetMesh()->SetPosition(FMath::FRandRange(0.f, Anim->GetPlayLength()), false);   // out of step with each other
}

void ATakeCoworker::LoadLines()
{
	Lines.Reset();
	Greetings.Reset();
	CoffeeLines.Reset();
	auto Matches = [this](const FString& RowRole) { return RowRole.Equals(NPCRole, ESearchCase::IgnoreCase); };
	if (const UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_NPCLines.DT_NPCLines")))
	{
		TArray<FTakeNPCLineRow*> Rows;
		Table->GetAllRows<FTakeNPCLineRow>(TEXT("TakeCoworker"), Rows);
		for (const FTakeNPCLineRow* R : Rows)
		{
			if (!R || R->Act != 1 || !Matches(R->Role) || (R->IsAgent && !bIsAgent)) { continue; }
			if (R->Context == TEXT("Greeting") || R->Context == TEXT("BadgeCheck")) { Greetings.Add(R->Line); }
			else if (R->Context == TEXT("AfterCoffee")) { CoffeeLines.Add(R->Line); }
			else { Lines.Add(R->Line); }
		}
	}
	if (Lines.Num() == 0)   // fallback until the DataTable is imported
	{
		Lines = { TEXT("Oh! Sorry, didn't see you there."), TEXT("Big day. Everyone knows their marks."),
		          TEXT("Have you done your compliance training?"), TEXT("Coffee machine's making that noise again."),
		          TEXT("Love the energy today, {EMP}."), TEXT("Is it still morning? Feels like it's been morning for a while.") };
		if (NPCRole.Equals(TEXT("Guard"), ESearchCase::IgnoreCase)) { Lines = { TEXT("Badge, please."), TEXT("Keep it moving."), TEXT("Corridor B is off limits.") }; }
		if (NPCRole.Equals(TEXT("Custodian"), ESearchCase::IgnoreCase)) { Lines = { TEXT("Mind the floor. Wet."), TEXT("I clean this hall every night. Same scuffs every morning."), TEXT("Don't go in my closet.") }; }
		if (NPCRole.Equals(TEXT("Receptionist"), ESearchCase::IgnoreCase)) { Lines = { TEXT("Welcome to HALCYON. You're expected."), TEXT("Please sign in. You already have.") }; }
	}
	Bag.Reset();
}

FString ATakeCoworker::NextLine()
{
	// First meeting: a greeting
	if (!bGreeted && Greetings.Num() > 0)
	{
		bGreeted = true;
		return Greetings[FMath::RandRange(0, Greetings.Num() - 1)];
	}
	bGreeted = true;
	// Had your coffee? They notice (each coffee line once)
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI && GI->bGotCoffee && CoffeeLines.Num() > 0 && FMath::FRand() < 0.5f)
	{
		const int32 I = FMath::RandRange(0, CoffeeLines.Num() - 1);
		const FString L = CoffeeLines[I];
		CoffeeLines.RemoveAt(I);
		return L;
	}
	// Small talk: shuffle bag, every line once before any repeats, never the same line twice in a row
	if (Lines.Num() == 0) { return FString(); }
	if (Bag.Num() == 0)
	{
		for (int32 I = 0; I < Lines.Num(); ++I) { Bag.Add(I); }
		for (int32 I = Bag.Num() - 1; I > 0; --I) { Bag.Swap(I, FMath::RandRange(0, I)); }
		if (Bag.Num() > 1 && Bag.Last() == LastLine) { Bag.Swap(0, Bag.Num() - 1); }
	}
	LastLine = Bag.Pop();
	return Lines[LastLine];
}

FString ATakeCoworker::WrapLine(const FString& In, int32 Width)
{
	TArray<FString> Words;
	In.ParseIntoArray(Words, TEXT(" "));
	FString Out, LineSoFar;
	for (const FString& W : Words)
	{
		if (!LineSoFar.IsEmpty() && LineSoFar.Len() + 1 + W.Len() > Width) { Out += LineSoFar + TEXT("<br>"); LineSoFar.Reset(); }
		LineSoFar += LineSoFar.IsEmpty() ? W : TEXT(" ") + W;
	}
	return Out + LineSoFar;
}

void ATakeCoworker::UpdateOverhead(const FVector& Eye)
{
	if (SpokenLine.IsEmpty())
	{
		if (bStoryNPC && State != ETakeCoworkerState::Watching)   // someone to talk to
		{
			Speech->SetWorldRotation(FRotator(0.f, (Eye - Speech->GetComponentLocation()).Rotation().Yaw, 0.f));
			Speech->SetTextRenderColor(FColor(80, 200, 180));
			Speech->SetWorldSize(16.f);
			Speech->SetText(FText::FromString(TEXT("!")));
		}
		return;
	}
	Speech->SetTextRenderColor(FColor(30, 40, 48));
	const FVector Here = Speech->GetComponentLocation();
	Speech->SetWorldRotation(FRotator(0.f, (Eye - Here).Rotation().Yaw, 0.f));
	// Scale with distance so it is always about the same size on screen and never runs off it
	const float Dist = FVector::Dist(Eye, Here);
	Speech->SetWorldSize(FMath::Clamp(Dist * 0.028f, 4.f, 14.f));
	if (TakeSettings::DialogueMode == 0)
	{
		Speech->SetText(FText::FromString(TEXT("...")));   // who's talking; the words are in the subtitle
	}
	else
	{
		Speech->SetText(FText::FromString(WrapLine(SpokenLine, 26)));
	}
}

float ATakeCoworker::GetFear() const
{
	return FearDirector.IsValid() ? FearDirector->Fear : 0.f;
}

bool ATakeCoworker::Speak(const FString& Context)
{
	if (CooldownLeft > 0.f) { return false; }
	const FString Line = NextLine();
	if (Line.IsEmpty()) { return false; }
	SayLine(Line);
	return true;
}

void ATakeCoworker::SayLine(const FString& Line)
{
	const FString Filled = TakeText::Fill(this, Line);
	SpokenLine = Filled;
	// Long lines stay up longer: ~14 characters a second, at least SpeechSeconds
	SpeechLeft = FMath::Max(SpeechSeconds, Filled.Len() / 14.f);
	CooldownLeft = SpeechCooldown;
	if (const UWorld* World = GetWorld()) { NextAnyoneMayTalk = World->GetTimeSeconds() + SpeechLeft * 0.8f; }
	if (TakeSettings::DialogueMode != 1)   // subtitles
	{
		if (APawn* P = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			if (UTakeGrabComponent* Hands = P->FindComponentByClass<UTakeGrabComponent>())
			{
				Hands->ShowSubtitle(NPCRole, Filled, SpeechLeft + 0.5f);
			}
		}
	}
	// Turn to face the player while talking
	if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (!bSitsAtDesk && (State == ETakeCoworkerState::Working || State == ETakeCoworkerState::AtSpot))
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
	if (bSkeletal)
	{
		UAnimSequence* Want = AnimIdle;
		if (State == ETakeCoworkerState::Watching) { Want = AnimStare ? AnimStare.Get() : AnimIdle.Get(); }
		else if (bMoving && AnimWalk) { Want = AnimWalk; }
		else if (State == ETakeCoworkerState::Working && bSitsAtDesk && AnimSit) { Want = AnimSit; }
		PlayAnim(Want);
	}
}

void ATakeCoworker::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CooldownLeft = FMath::Max(0.f, CooldownLeft - DeltaSeconds);
	NearCooldownLeft = FMath::Max(0.f, NearCooldownLeft - DeltaSeconds);
	if (SpeechLeft > 0.f)
	{
		SpeechLeft -= DeltaSeconds;
		if (SpeechLeft <= 0.f) { SpokenLine.Reset(); Speech->SetText(FText::GetEmpty()); }
	}

	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (Player)
	{
		FVector Eye; FRotator View;
		Player->GetActorEyesViewPoint(Eye, View);
		UpdateOverhead(Eye);
		const float Dist = FVector::Dist2D(Player->GetActorLocation(), GetActorLocation());
		FCollisionQueryParams SightParams(SCENE_QUERY_STAT(TakeBark), false, Player);
		SightParams.AddIgnoredActor(this);
		// Bump: always reacts
		if (Dist < 95.f) { Speak(TEXT("Bump")); }
		// Walk past: they talk on their own, one NPC at a time, not too often
		else if (bTalksWhenNear && Dist < NearRange && NearCooldownLeft <= 0.f && GetFear() < WatchFear
		         && GetWorld()->GetTimeSeconds() >= NextAnyoneMayTalk
		         && GetWorld()->LineTraceTestByChannel(Eye, GetActorLocation() + FVector(0.f, 0.f, 60.f), ECC_Visibility, SightParams) == false)
		{
			if (Speak(TEXT("Near"))) { NearCooldownLeft = FMath::FRandRange(NearCooldownMin, NearCooldownMax); }
		}
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
			SpokenLine.Reset();
			SpeechLeft = 0.f;
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
