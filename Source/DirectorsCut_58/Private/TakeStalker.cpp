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
		if (UMaterialInterface* Fear = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Props/Materials/MI_Face_Fear.MI_Face_Fear"))) { GetMesh()->SetMaterialByName(FName(TEXT("M_Face")), Fear); }
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
		if (To.Size2D() > 60.f) { AddMovementInput(FVector(To.X, To.Y, 0.f).GetSafeNormal(), 1.f); }
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
	if (UTakeRewindSubsystem* Rewind = GetWorld()->GetSubsystem<UTakeRewindSubsystem>())
	{
		Rewind->Rewind();
	}
}
