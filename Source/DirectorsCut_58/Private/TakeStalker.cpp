// TAKE 4471 - the stalker
#include "TakeStalker.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Navigation/PathFollowingComponent.h"
#include "DirectorsCut_58.h"
#include "TakeGameInstance.h"
#include "TakeRewindSubsystem.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"

ATakeStalker::ATakeStalker()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCapsuleComponent()->InitCapsuleSize(40.f, 117.f);           // 2.35 m tall
	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);       // so thrown props register hits

	CatchSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CatchSphere"));
	CatchSphere->SetupAttachment(GetCapsuleComponent());
	CatchSphere->InitSphereRadius(95.f);
	CatchSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(GetCapsuleComponent());
	Body->SetRelativeLocation(FVector(0.f, 0.f, -117.f));   // feet on the floor
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> StalkerMesh(TEXT("/Game/Props/Meshes/SM_Stalker.SM_Stalker"));
	if (StalkerMesh.Succeeded()) { Body->SetStaticMesh(StalkerMesh.Object); }

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 300.f, 0.f);
	bUseControllerRotationYaw = false;
}

void ATakeStalker::BeginPlay()
{
	Super::BeginPlay();
	SpawnLocation = GetActorLocation();
	// The rigged mannequin (dark, tall), walking: no more sliding sideways
	USkeletalMesh* SK = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequin/SK_MannequinStalker.SK_MannequinStalker"));
	AnimWalk = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Characters/Mannequin/AS_Walk.AS_Walk"));
	AnimStare = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/Characters/Mannequin/AS_Stare.AS_Stare"));
	if (SK && AnimWalk)
	{
		GetMesh()->SetSkeletalMeshAsset(SK);
		GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -117.f), FRotator(0.f, -90.f, 0.f));
		GetMesh()->SetRelativeScale3D(FVector(1.3f));
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		GetMesh()->PlayAnimation(AnimStare ? AnimStare.Get() : AnimWalk.Get(), true);
		// Sol's stalker face (approved Nov 14: red eyes, curled grin); the coworkers' Fear face if it isn't imported yet
		UMaterialInterface* Face = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Props/Materials/MI_Face_Stalker.MI_Face_Stalker"));
		if (!Face) { Face = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Props/Materials/MI_Face_Fear.MI_Face_Fear")); }
		if (Face) { GetMesh()->SetMaterialByName(FName(TEXT("M_Face")), Face); }
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Stalker: rigged mannequin, face %s, face slot %s"),
			Face ? *Face->GetName() : TEXT("(none)"),
			GetMesh()->GetMaterialIndex(FName(TEXT("M_Face"))) != INDEX_NONE ? TEXT("found") : TEXT("MISSING"));
		Body->SetHiddenInGame(true);
		bRigged = true;
	}
	else
	{
		Body->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));   // the static figure faces +Y: turn it to face where it walks
	}
	CatchSphere->OnComponentBeginOverlap.AddDynamic(this, &ATakeStalker::OnCatchOverlap);
	Deactivate();
}

APawn* ATakeStalker::GetPlayerPawn() const
{
	return UGameplayStatics::GetPlayerPawn(this, 0);
}

float ATakeStalker::ChaseSpeed() const
{
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	return ChaseBaseSpeed + ChaseSpeedPerSoulBit * (GI ? GI->SoulBits : 0);
}

void ATakeStalker::SetState(ETakeStalkerState NewState)
{
	if (State == NewState) { return; }
	State = NewState;
	StateTime = 0.f;
	RepathTimer = 0.f;
	float Speed = 0.f;
	switch (NewState)
	{
	case ETakeStalkerState::Stalk: Speed = StalkSpeed; break;
	case ETakeStalkerState::Chase: Speed = ChaseSpeed(); break;
	default: break;
	}
	GetCharacterMovement()->MaxWalkSpeed = Speed * SlowMultiplier;
	if (NewState == ETakeStalkerState::Stunned || NewState == ETakeStalkerState::Dormant)
	{
		if (AAIController* AI = Cast<AAIController>(GetController())) { AI->StopMovement(); }
	}
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Stalker -> %s"), *UEnum::GetValueAsString(NewState));
	OnStateChanged.Broadcast(NewState);
}

void ATakeStalker::Activate()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetState(ETakeStalkerState::Stalk);
}

void ATakeStalker::Deactivate()
{
	SetState(ETakeStalkerState::Dormant);
	if (bHiddenWhileDormant)
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
	SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
}

void ATakeStalker::Stun(float Seconds)
{
	if (State == ETakeStalkerState::Dormant) { return; }
	if (State == ETakeStalkerState::Stunned) { StunFor = FMath::Max(StunFor, StateTime + Seconds); return; }   // a stapler never shortens a coffee
	StunFor = Seconds;
	SetState(ETakeStalkerState::Stunned);
}

void ATakeStalker::Slow(float Multiplier, float Seconds)
{
	SlowMultiplier = FMath::Clamp(Multiplier, 0.f, 1.f);
	SlowTimeLeft = Seconds;
	if (State == ETakeStalkerState::Chase) { GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed() * SlowMultiplier; }
}

void ATakeStalker::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (State == ETakeStalkerState::Dormant) { return; }

	StateTime += DeltaSeconds;
	RepathTimer -= DeltaSeconds;
	if (SlowTimeLeft > 0.f)
	{
		SlowTimeLeft -= DeltaSeconds;
		if (SlowTimeLeft <= 0.f) { SlowMultiplier = 1.f; if (State == ETakeStalkerState::Chase) { GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed(); } }
	}

	APawn* Player = GetPlayerPawn();
	AAIController* AI = Cast<AAIController>(GetController());
	if (!Player || !AI) { return; }

	// Wedged in furniture (it tries to move but hasn't for a second)? Pop it out to the nearest free spot
	// (only while chasing and still far from you: when stalking it is allowed to stand and watch)
	if (State == ETakeStalkerState::Chase && FVector::Dist2D(GetActorLocation(), Player->GetActorLocation()) > 200.f)
	{
		const FVector Here = GetActorLocation();
		if (FVector::DistSquared2D(Here, StuckCheckFrom) > FMath::Square(25.f)) { StuckCheckFrom = Here; StuckTime = 0.f; }
		else if ((StuckTime += DeltaSeconds) > 1.f)
		{
			FVector Free = Here + (Player->GetActorLocation() - Here).GetSafeNormal2D() * 80.f;   // a step toward you
			if (GetWorld()->FindTeleportSpot(this, Free, GetActorRotation()))
			{
				SetActorLocation(Free, false, nullptr, ETeleportType::TeleportPhysics);
				UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Stalker was stuck at %s, moved to %s"), *Here.ToCompactString(), *Free.ToCompactString());
			}
			StuckCheckFrom = GetActorLocation();
			StuckTime = 0.f;
		}
	}
	else { StuckTime = 0.f; StuckCheckFrom = GetActorLocation(); }

	// Footsteps: heavy, slow when it stalks, quicker when it runs, so you can hear where it is
	{
		const float Speed2D = GetVelocity().Size2D();
		const float StepEvery = State == ETakeStalkerState::Chase ? 0.34f : 0.6f;
		if (Speed2D > 60.f && FMath::FloorToInt32(StateTime / StepEvery) != FMath::FloorToInt32((StateTime - DeltaSeconds) / StepEvery))
		{
			static const TCHAR* Steps[] = { TEXT("sfx_step_1"), TEXT("sfx_step_2"), TEXT("sfx_step_3"), TEXT("sfx_step_4") };
			const FString Name = Steps[FMath::RandRange(0, 3)];
			if (USoundBase* Step = LoadObject<USoundBase>(nullptr, *FString::Printf(TEXT("/Game/Audio/Placeholder/%s.%s"), *Name, *Name)))
			{
				UGameplayStatics::PlaySoundAtLocation(this, Step, GetActorLocation() - FVector(0.f, 0.f, 90.f),
					State == ETakeStalkerState::Chase ? 1.2f : 0.9f, 0.7f, 0.f, TakeAudio::World3D());   // lower pitch: something heavier than you; louder as it nears
			}
		}
	}
	if (bRigged)
	{
		const bool bMoving = GetVelocity().SizeSquared2D() > 100.f;
		UAnimSequence* Want = (bMoving && AnimWalk) ? AnimWalk.Get() : (AnimStare ? AnimStare.Get() : AnimWalk.Get());
		if (Want != AnimPlaying) { AnimPlaying = Want; GetMesh()->PlayAnimation(Want, true); }
	}
	// No nav mesh where we are (the ward)? Walk straight at them instead of standing still
	if ((State == ETakeStalkerState::Chase || State == ETakeStalkerState::Stalk) && AI->GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		const FVector To = Player->GetActorLocation() - GetActorLocation();
		if (To.Size2D() > 60.f)
		{
			FVector Dir = FVector(To.X, To.Y, 0.f).GetSafeNormal();
			// Something in the way (a bed, a curtain)? Steer along it instead of pushing into it
			FHitResult Hit;
			FCollisionQueryParams Q(SCENE_QUERY_STAT(StalkerSteer), false, this);
			Q.AddIgnoredActor(Player);
			const FVector From = GetActorLocation();
			if (GetWorld()->SweepSingleByChannel(Hit, From, From + Dir * 150.f, FQuat::Identity, ECC_Pawn,
				FCollisionShape::MakeSphere(35.f), Q) && !(Hit.GetActor() && Hit.GetActor()->ActorHasTag(TEXT("Throwable"))))
			{
				const FVector N = FVector(Hit.ImpactNormal.X, Hit.ImpactNormal.Y, 0.f).GetSafeNormal();
				FVector Slide = Dir - N * FVector::DotProduct(Dir, N);          // the part of the way that isn't into the obstacle
				if (Slide.SizeSquared() < 0.09f)                              // head-on: go round, on the side nearer the player
				{
					Slide = FVector::CrossProduct(N, FVector::UpVector);
					if (FVector::DotProduct(Slide, To) < 0.f) { Slide = -Slide; }
				}
				Dir = (Slide.GetSafeNormal() * 0.85f + N * 0.15f).GetSafeNormal();   // slide along, eased off the surface
			}
			AddMovementInput(Dir, 1.f);
		}
	}

	switch (State)
	{
	case ETakeStalkerState::Stalk:
	{
		// Being looked at straight on, or stalking too long -> chase
		FVector EyeLoc; FRotator EyeRot;
		Player->GetActorEyesViewPoint(EyeLoc, EyeRot);
		const FVector ToMe = (GetActorLocation() - EyeLoc).GetSafeNormal();
		if (FVector::DotProduct(EyeRot.Vector(), ToMe) > LookAtDot || StateTime > StalkDuration)
		{
			SetState(ETakeStalkerState::Chase);
			break;
		}
		if (RepathTimer <= 0.f)
		{
			RepathTimer = 1.f;
			const FVector Behind = Player->GetActorLocation() - Player->GetActorForwardVector() * StalkDistance;
			AI->MoveToLocation(Behind, 50.f);
		}
		break;
	}
	case ETakeStalkerState::Chase:
		if (RepathTimer <= 0.f)
		{
			RepathTimer = 0.25f;
			AI->MoveToActor(Player, 30.f);
		}
		break;
	case ETakeStalkerState::Stunned:
		if (StateTime >= StunFor) { SetState(ETakeStalkerState::Chase); }
		break;
	default:
		break;
	}
}

void ATakeStalker::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
                             FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	if (!Other || !OtherComp || !Other->ActorHasTag(TEXT("Throwable"))) { return; }
	if (OtherComp->GetPhysicsLinearVelocity().Size() >= MinHitSpeed)
	{
		Stun(StunSeconds);
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Stalker hit by %s"), *Other->GetName());
	}
}

void ATakeStalker::OnCatchOverlap(UPrimitiveComponent*, AActor* Other, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (State != ETakeStalkerState::Chase && State != ETakeStalkerState::Stalk) { return; }
	const APawn* Pawn = Cast<APawn>(Other);
	if (!Pawn || !Pawn->IsPlayerControlled()) { return; }
	if (bSoftCatch) { bSoftCaught = true; return; }
	if (UTakeRewindSubsystem* Rewind = GetWorld()->GetSubsystem<UTakeRewindSubsystem>())
	{
		Rewind->Rewind();
	}
}
