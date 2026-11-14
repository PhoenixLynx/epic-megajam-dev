// TAKE 4471 - interact / grab / throw / telekinesis / desk login / doors
#include "TakeGrabComponent.h"
#include "DirectorsCut_58.h"
#include "TakeGameInstance.h"
#include "TakeCoworker.h"
#include "TakeFearDirector.h"
#include "TakeWidgets.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
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
	if (bHideTemplateMeshes) { HideTemplateMeshes(); }
	if (bAutoBindKeys)
	{
		// Input component exists once the pawn is possessed; try now and shortly after.
		BindKeys();
		FTimerHandle T;
		GetWorld()->GetTimerManager().SetTimer(T, this, &UTakeGrabComponent::BindKeys, 0.5f, false);
	}
	if (bShowPrompt)
	{
		APawn* Pawn = Cast<APawn>(Owner);
		if (APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr)
		{
			Prompt = CreateWidget<UTakePromptWidget>(PC, UTakePromptWidget::StaticClass());
			if (Prompt) { Prompt->AddToViewport(5); }
		}
	}
}

void UTakeGrabComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (Prompt) { Prompt->RemoveFromParent(); Prompt = nullptr; }
	if (Terminal) { Terminal->RemoveFromParent(); Terminal = nullptr; }
	Super::EndPlay(Reason);
}

void UTakeGrabComponent::HideTemplateMeshes()
{
	TArray<USkeletalMeshComponent*> Meshes;
	GetOwner()->GetComponents<USkeletalMeshComponent>(Meshes);
	for (USkeletalMeshComponent* M : Meshes)
	{
		M->SetHiddenInGame(true, /*bPropagateToChildren=*/true);   // template arms, body and anything held in them
		M->SetCastShadow(false);
	}
	if (Meshes.Num() > 0) { UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Hid %d template mesh(es) on the player"), Meshes.Num()); }
}

void UTakeGrabComponent::BindKeys()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn || !Pawn->InputComponent) { return; }
	static const FName Bound(TEXT("TakeGrabBound"));
	if (Pawn->Tags.Contains(Bound)) { return; }
	Pawn->Tags.Add(Bound);
	UInputComponent* IC = Pawn->InputComponent;
	IC->BindKey(EKeys::E, IE_Pressed, this, &UTakeGrabComponent::Interact);
	IC->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &UTakeGrabComponent::Throw);
	if (bDebugFearKeys)
	{
		IC->BindKey(EKeys::Zero, IE_Pressed, this, &UTakeGrabComponent::Fear0);
		IC->BindKey(EKeys::One, IE_Pressed, this, &UTakeGrabComponent::Fear1);
		IC->BindKey(EKeys::Two, IE_Pressed, this, &UTakeGrabComponent::Fear2);
		IC->BindKey(EKeys::Three, IE_Pressed, this, &UTakeGrabComponent::Fear3);
		IC->BindKey(EKeys::Four, IE_Pressed, this, &UTakeGrabComponent::Fear4);
	}
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Keys bound: E = use / pick up, Right Mouse = throw, 1-4/0 = Fear"));
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

ETakeLookKind UTakeGrabComponent::Look(FHitResult& Hit) const
{
	FVector Loc; FRotator Rot;
	if (!GetView(Loc, Rot)) { return ETakeLookKind::None; }
	const FVector End = Loc + Rot.Vector() * FMath::Max(CurrentRange(), UseRange);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(TakeLook), false, GetOwner());
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Loc, End, ECC_Visibility, Params)) { return ETakeLookKind::None; }
	AActor* A = Hit.GetActor();
	UPrimitiveComponent* Comp = Hit.GetComponent();
	const bool bNear = Hit.Distance <= UseRange;
	if (A && bNear)
	{
		if (A->IsA<ATakeCoworker>()) { return ETakeLookKind::Coworker; }
		if (A->ActorHasTag(TEXT("Terminal"))) { return ETakeLookKind::Terminal; }
		if (A->ActorHasTag(TEXT("Openable"))) { return ETakeLookKind::Openable; }
	}
	if (Comp && Comp->IsSimulatingPhysics() && Hit.Distance <= CurrentRange())
	{
		return (Comp->GetMass() > MaxMassWithoutTelekinesis && GetSoulBits() < 3) ? ETakeLookKind::TooHeavy : ETakeLookKind::Prop;
	}
	return ETakeLookKind::None;
}

void UTakeGrabComponent::Interact()
{
	if (bSeated) { LogOut(); return; }
	if (IsHolding()) { Drop(); return; }
	FHitResult Hit;
	switch (Look(Hit))
	{
	case ETakeLookKind::Coworker:  Cast<ATakeCoworker>(Hit.GetActor())->Speak(TEXT("Talk")); break;
	case ETakeLookKind::Terminal:  SitAndLogIn(Hit.GetActor()); break;
	case ETakeLookKind::Openable:  ToggleOpen(Hit.GetActor()); break;
	case ETakeLookKind::Prop:      TryGrabOrDrop(); break;
	default: break;
	}
}

bool UTakeGrabComponent::TryGrabOrDrop()
{
	if (IsHolding()) { Drop(); return false; }
	FHitResult Hit;
	if (Look(Hit) != ETakeLookKind::Prop) { return false; }
	UPrimitiveComponent* Comp = Hit.GetComponent();
	FVector Loc; FRotator View;
	GetView(Loc, View);
	HeldYawOffset = Comp->GetComponentRotation().Yaw - View.Yaw;   // keeps the prop's facing relative to you
	HeldPitchRoll = FRotator(Comp->GetComponentRotation().Pitch, 0.f, Comp->GetComponentRotation().Roll);
	Handle->GrabComponentAtLocationWithRotation(Comp, NAME_None, Comp->GetComponentLocation(), Comp->GetComponentRotation());
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

// ------------------------------------------------------------------ Doors, fridge
void UTakeGrabComponent::ToggleOpen(AActor* Openable)
{
	if (!Openable) { return; }
	FSwing* S = Swings.Find(Openable);
	if (!S)
	{
		FSwing New;
		New.ClosedYaw = Openable->GetActorRotation().Yaw;
		for (const FName& Tag : Openable->Tags)
		{
			const FString T = Tag.ToString();
			if (T.StartsWith(TEXT("OpenYaw="))) { New.OpenYaw = FCString::Atof(*T.Mid(8)); }
		}
		S = &Swings.Add(Openable, New);
	}
	S->bOpen = !S->bOpen;
}

void UTakeGrabComponent::TickSwings(float DeltaTime)
{
	const float Step = DeltaTime / FMath::Max(DoorSwingSeconds, 0.05f);
	for (auto It = Swings.CreateIterator(); It; ++It)
	{
		AActor* A = It.Key().Get();
		if (!A) { It.RemoveCurrent(); continue; }
		FSwing& S = It.Value();
		const float Goal = S.bOpen ? 1.f : 0.f;
		if (FMath::IsNearlyEqual(S.Alpha, Goal)) { continue; }
		S.Alpha = FMath::FInterpConstantTo(S.Alpha, Goal, 1.f, Step);
		const float Eased = FMath::InterpEaseInOut(0.f, 1.f, S.Alpha, 2.f);
		FRotator R = A->GetActorRotation();
		R.Yaw = S.ClosedYaw + S.OpenYaw * Eased;
		A->SetActorRotation(R);
	}
}

// ------------------------------------------------------------------ Desk #4471
void UTakeGrabComponent::SitAndLogIn(AActor* TerminalActor)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC || bSeated || !TerminalActor) { return; }

	// The screen is the Monitor4471 actor if we're looking at the desk
	AActor* Screen = TerminalActor;
	if (!Screen->ActorHasTag(TEXT("Monitor4471")))
	{
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			if (It->ActorHasTag(TEXT("Monitor4471"))) { Screen = *It; break; }
		}
	}

	bSeated = true;
	SeatedAt = Screen;
	SeatReturnLocation = Pawn->GetActorLocation();
	Drop();

	// Sit on whichever side of the desk you walked up from, facing the screen
	const FVector ScreenLoc = Screen->GetActorLocation();
	FVector Side = (Pawn->GetActorLocation() - ScreenLoc).GetSafeNormal2D();
	if (Side.IsNearlyZero()) { Side = FVector(0.f, 1.f, 0.f); }
	FVector Seat = ScreenLoc + Side * 95.f;
	Seat.Z = Pawn->GetActorLocation().Z;
	Pawn->SetActorLocation(Seat, false, nullptr, ETeleportType::TeleportPhysics);
	const FVector LookAt = ScreenLoc + FVector(0.f, 0.f, 25.f);
	FVector Eye; FRotator Unused;
	if (ACharacter* C = Cast<ACharacter>(Pawn))
	{
		C->GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
		C->GetCharacterMovement()->StopMovementImmediately();
		C->Crouch();
	}
	Pawn->GetActorEyesViewPoint(Eye, Unused);
	PC->SetControlRotation((LookAt - Eye).Rotation());
	PC->SetIgnoreMoveInput(true);
	PC->SetIgnoreLookInput(true);

	if (Prompt) { Prompt->SetPrompt(TEXT("")); }
	Terminal = CreateWidget<UTakeTerminalWidget>(PC, UTakeTerminalWidget::StaticClass());
	if (Terminal)
	{
		Terminal->SetIsFocusable(true);
		Terminal->OnLogoutRequested.AddDynamic(this, &UTakeGrabComponent::HandleTerminalLogout);
		Terminal->AddToViewport(20);
		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(Terminal->TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		PC->SetInputMode(Mode);
		PC->SetShowMouseCursor(true);
	}
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Logged in at %s"), *Screen->GetName());
	OnLoggedIn.Broadcast(Screen);
}

void UTakeGrabComponent::LogOut()
{
	if (!bSeated) { return; }
	bSeated = false;
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (Terminal) { Terminal->RemoveFromParent(); Terminal = nullptr; }
	if (PC)
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
		PC->SetIgnoreMoveInput(false);
		PC->SetIgnoreLookInput(false);
	}
	if (ACharacter* C = Cast<ACharacter>(Pawn)) { C->UnCrouch(); }
	OnLoggedOut.Broadcast(SeatedAt.Get());
}

// ------------------------------------------------------------------ Debug Fear keys
void UTakeGrabComponent::SetFearDebug(float Value)
{
	for (TActorIterator<ATakeFearDirector> It(GetWorld()); It; ++It)
	{
		It->StartCreep(Value, 1.5f);
		if (GEngine) { GEngine->AddOnScreenDebugMessage(4471, 2.f, FColor::Cyan, FString::Printf(TEXT("Fear -> %.2f"), Value)); }
		return;
	}
}

// ------------------------------------------------------------------ Tick
void UTakeGrabComponent::UpdatePrompt()
{
	if (!Prompt) { return; }
	if (bSeated) { Prompt->SetPrompt(TEXT("")); return; }
	if (IsHolding()) { Prompt->SetPrompt(TEXT("E  Drop        Right Mouse  Throw")); return; }
	FHitResult Hit;
	switch (Look(Hit))
	{
	case ETakeLookKind::Coworker: Prompt->SetPrompt(TEXT("E  Talk")); break;
	case ETakeLookKind::Terminal: Prompt->SetPrompt(TEXT("E  Sit and log in")); break;
	case ETakeLookKind::Openable:
	{
		const FSwing* S = Swings.Find(Hit.GetActor());
		Prompt->SetPrompt((S && S->bOpen) ? TEXT("E  Close") : TEXT("E  Open"));
		break;
	}
	case ETakeLookKind::Prop:     Prompt->SetPrompt(TEXT("E  Pick up")); break;
	case ETakeLookKind::TooHeavy: Prompt->SetPrompt(TEXT("Too heavy")); break;
	default:                      Prompt->SetPrompt(TEXT("")); break;
	}
}

void UTakeGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickSwings(DeltaTime);
	UpdatePrompt();
	if (!IsHolding()) { return; }
	FVector Loc; FRotator Rot;
	if (GetView(Loc, Rot))
	{
		Handle->SetTargetLocationAndRotation(Loc + Rot.Vector() * HoldDistance, FRotator(HeldPitchRoll.Pitch, Rot.Yaw + HeldYawOffset, HeldPitchRoll.Roll));
	}
}
