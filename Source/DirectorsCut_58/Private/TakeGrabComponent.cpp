// TAKE 4471 - interact / grab / throw / telekinesis / desk login / doors
#include "TakeGrabComponent.h"
#include "DirectorsCut_58.h"
#include "TakeGameInstance.h"
#include "TakeCoworker.h"
#include "TakeFearDirector.h"
#include "TakeWidgets.h"
#include "TakePauseMenu.h"
#include "TakeWisp.h"
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
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMeshActor.h"

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
	UTakePauseMenu::ApplySavedSettings();
	PlayStartTime = GetWorld()->GetTimeSeconds();
	for (int32 I = 1; I <= 4; ++I)
	{
		const FString Name = FString::Printf(TEXT("sfx_step_%d"), I);
		if (USoundBase* Snd = LoadObject<USoundBase>(nullptr, *FString::Printf(TEXT("/Game/Audio/Placeholder/%s.%s"), *Name, *Name))) { StepSounds.Add(Snd); }
	}
	BindPropImpacts();
	CollectFunProps();
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
	BindStoryEvents();
	const UTakeGameInstance* TitleGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (bPlayColdOpen && TitleGI && !TitleGI->bTitleDone) { ShowTitle(); }   // the title screen first, once per launch
	else if (bPlayColdOpen) { StartSlate(false); }
	else if (Prompt) { Prompt->SetSlate(0.f, FString(), 0.f); }
}

void UTakeGrabComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (Prompt) { Prompt->RemoveFromParent(); Prompt = nullptr; }
	if (Terminal) { Terminal->RemoveFromParent(); Terminal = nullptr; }
	if (PauseMenu) { PauseMenu->RemoveFromParent(); PauseMenu = nullptr; }
	if (Inspecting) { Inspecting->RemoveFromParent(); Inspecting = nullptr; }
	if (LockPanel) { LockPanel->RemoveFromParent(); LockPanel = nullptr; }
	StopRinging();
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
	IC->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &UTakeGrabComponent::ThrowInput);   // only while holding
	IC->BindKey(EKeys::F, IE_Pressed, this, &UTakeGrabComponent::Sip);
	IC->BindKey(EKeys::P, IE_Pressed, this, &UTakeGrabComponent::TogglePause);
	IC->BindKey(EKeys::Escape, IE_Pressed, this, &UTakeGrabComponent::TogglePause);
	IC->BindKey(EKeys::N, IE_Pressed, this, &UTakeGrabComponent::OpenNotes);   // playtest note, any time
	IC->BindKey(EKeys::F9, IE_Pressed, this, &UTakeGrabComponent::TakeScreenshot);   // -> Saved/Screenshots   // in the editor Esc stops Play: use P
	if (bDebugFearKeys)
	{
		IC->BindKey(EKeys::Zero, IE_Pressed, this, &UTakeGrabComponent::Fear0);
		IC->BindKey(EKeys::One, IE_Pressed, this, &UTakeGrabComponent::Fear1);
		IC->BindKey(EKeys::Two, IE_Pressed, this, &UTakeGrabComponent::Fear2);
		IC->BindKey(EKeys::Three, IE_Pressed, this, &UTakeGrabComponent::Fear3);
		IC->BindKey(EKeys::Four, IE_Pressed, this, &UTakeGrabComponent::Fear4);
		IC->BindKey(EKeys::F8, IE_Pressed, this, &UTakeGrabComponent::SkipTasks);   // playtest: finish the current task list
		IC->BindKey(EKeys::F7, IE_Pressed, this, &UTakeGrabComponent::TestStalker);  // playtest: the stalker, now
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

ATakeCoworker* UTakeGrabComponent::CoworkerInView() const
{
	// Coworkers are Pawns, and Pawns ignore the Visibility trace: pick the one you're facing instead.
	FVector Loc; FRotator Rot;
	if (!GetView(Loc, Rot)) { return nullptr; }
	ATakeCoworker* Best = nullptr;
	float BestDot = 0.9f;
	for (TActorIterator<ATakeCoworker> It(GetWorld()); It; ++It)
	{
		if (!It->bStoryNPC || !It->HasNewConversation() || It->IsHidden()) { continue; }   // E only when they have a "!" (something new to say)
		const FVector To = It->GetActorLocation() + FVector(0.f, 0.f, 40.f) - Loc;
		if (To.Size() > UseRange) { continue; }
		FCollisionQueryParams Sight(SCENE_QUERY_STAT(TakeTalkSight), false, GetOwner());
		Sight.AddIgnoredActor(*It);
		if (GetWorld()->LineTraceTestByChannel(Loc, Loc + To, ECC_Visibility, Sight)) { continue; }   // not through a closed door
		const float Dot = FVector::DotProduct(Rot.Vector(), To.GetSafeNormal());
		if (Dot > BestDot) { BestDot = Dot; Best = *It; }
	}
	return Best;
}

ETakeLookKind UTakeGrabComponent::Look(FHitResult& Hit) const
{
	FVector Loc; FRotator Rot;
	if (!GetView(Loc, Rot)) { return ETakeLookKind::None; }
	if (ATakeCoworker* C = CoworkerInView())
	{
		Hit = FHitResult(C, nullptr, C->GetActorLocation(), FVector::UpVector);
		return ETakeLookKind::Coworker;
	}
	// The soul-energy wisp: small and bobbing, so facing it roughly is enough
	if (ATakeWisp* W = WispInView())
	{
		Hit = FHitResult(W, nullptr, W->GetActorLocation(), FVector::UpVector);
		return ETakeLookKind::Wisp;
	}
	// A ringing phone right next to you wins over the desk, the monitor and the props around it
	if (PhoneRingingNear())
	{
		AActor* P = RingingPhone.Get();
		const FVector To = (P->GetActorLocation() - Loc).GetSafeNormal();
		if (FVector::DotProduct(Rot.Vector(), To) > 0.3f)
		{
			Hit = FHitResult(P, nullptr, P->GetActorLocation(), FVector::UpVector);
			return ETakeLookKind::Phone;
		}
	}
	const FVector End = Loc + Rot.Vector() * FMath::Max(CurrentRange(), UseRange);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(TakeLook), false, GetOwner());
	if (IsHolding()) { Params.AddIgnoredActor(Handle->GetGrabbedComponent()->GetOwner()); }   // see past what's in your hands
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Loc, End, ECC_Visibility, Params)) { return ETakeLookKind::None; }
	AActor* A = Hit.GetActor();
	UPrimitiveComponent* Comp = Hit.GetComponent();
	const bool bNear = Hit.Distance <= UseRange;
	if (A && bNear)
	{
		if (const ATakeCoworker* C = Cast<ATakeCoworker>(A))
		{
			return (C->bStoryNPC && C->HasNewConversation()) ? ETakeLookKind::Coworker : ETakeLookKind::None;   // extras talk on their own
		}
		if (A->ActorHasTag(TEXT("Terminal"))) { return ETakeLookKind::Terminal; }
		if (A->ActorHasTag(TEXT("Openable"))) { return ETakeLookKind::Openable; }
		if (A->ActorHasTag(TEXT("CoffeeMachine"))) { return ETakeLookKind::Coffee; }
		if (A->ActorHasTag(TEXT("Vending"))) { return ETakeLookKind::Vending; }
		if (A->ActorHasTag(TEXT("CallScreen")) && CanJoinCall()) { return ETakeLookKind::Call; }
		if (A->ActorHasTag(TEXT("KeyLockPanel")) || A->ActorHasTag(TEXT("KeyLockPanel=3"))) { return ETakeLookKind::LockPanel; }
		if (A->ActorHasTag(TEXT("PassBlock=2"))) { return ETakeLookKind::Openable; }
		if (!UseKey(A).IsEmpty()) { return ETakeLookKind::Use; }
		if (RingingPhone.IsValid() && A == RingingPhone.Get()) { return ETakeLookKind::Phone; }
		if (A->ActorHasTag(TEXT("Inspectable"))) { return ETakeLookKind::Inspect; }
		if (const ATakeWisp* W = Cast<ATakeWisp>(A)) { return W->IsAvailable() ? ETakeLookKind::Wisp : ETakeLookKind::None; }
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
	FHitResult Hit;
	if (IsHolding())
	{
		if (Look(Hit) == ETakeLookKind::Coffee) { PourCoffee(); return; }   // refill the mug you're holding (or... whatever that is)
		Drop();
		return;
	}
	switch (Look(Hit))
	{
	case ETakeLookKind::Coworker:  if (ATakeCoworker* C = Cast<ATakeCoworker>(Hit.GetActor())) { C->Talk(); } break;
	case ETakeLookKind::Terminal:  SitAndLogIn(Hit.GetActor()); break;
	case ETakeLookKind::Openable:  ToggleOpen(Hit.GetActor()); break;
	case ETakeLookKind::Coffee:    PourCoffee(); break;
	case ETakeLookKind::Phone:     AnswerPhone(Hit.GetActor()); break;
	case ETakeLookKind::Inspect:   Inspect(Hit.GetActor()); break;
	case ETakeLookKind::Call:      JoinCall(); break;
	case ETakeLookKind::Vending:   Vend(Hit.GetActor()); break;
	case ETakeLookKind::LockPanel: OpenLockPanel(Hit.GetActor()->ActorHasTag(TEXT("KeyLockPanel=3")) ? 3 : 2); break;
	case ETakeLookKind::Use:       UseThing(Hit.GetActor()); break;
	case ETakeLookKind::Wisp:      CollectWisp(Cast<ATakeWisp>(Hit.GetActor())); break;
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
	Comp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);   // a held mug can't shove you (or be shoved by you)
	PlaySfx(TEXT("sfx_pickup"), Comp->GetComponentLocation(), 0.6f);
	// Took someone's computer (or mug)? The nearest coworker notices
	if (const UStaticMeshComponent* Took = Cast<UStaticMeshComponent>(Comp))
	{
		const FString TookName = Took->GetStaticMesh() ? Took->GetStaticMesh()->GetName() : FString();
		if (TookName.Contains(TEXT("Monitor")) || TookName.Contains(TEXT("Keyboard")) || TookName.Contains(TEXT("Laptop")) || TookName.Contains(TEXT("Mug")))
		{
			ATakeCoworker* Owner = nullptr;
			float Best = 230.f;
			for (TActorIterator<ATakeCoworker> It(GetWorld()); It; ++It)
			{
				const float D = FVector::Dist2D(It->GetActorLocation(), Comp->GetComponentLocation());
				if (D < Best) { Best = D; Owner = *It; }
			}
			if (Owner) { Owner->ReactToTheft(); }
		}
	}
	OnGrabbed.Broadcast(Comp);
	return true;
}

void UTakeGrabComponent::Drop()
{
	if (!Handle) { return; }
	if (UPrimitiveComponent* C = Handle->GetGrabbedComponent()) { C->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block); }
	Handle->ReleaseComponent();
}

void UTakeGrabComponent::Throw()
{
	if (!IsHolding()) { return; }
	UPrimitiveComponent* Comp = Handle->GetGrabbedComponent();
	FVector Loc; FRotator Rot;
	GetView(Loc, Rot);
	Handle->ReleaseComponent();
	Comp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);   // pawns include coworkers: let it hit them
	Comp->SetUseCCD(true);   // fast and the floors/curtains are thin (10 cm / 4 cm): without this a hard throw can pass right through
	TossFrom = Comp->GetComponentLocation();
	Comp->AddImpulse(Rot.Vector() * ThrowStrength, NAME_None, /*bVelChange=*/true);
	PlaySfx(TEXT("sfx_throw"), Comp->GetComponentLocation(), 0.7f);
	TossProp = Comp;   // paper toss: watch where it lands
	TossLeft = 3.5f;
	OnThrown.Broadcast(Comp);
}

// ------------------------------------------------------------------ Doors, fridge
void UTakeGrabComponent::ToggleOpen(AActor* Openable)
{
	if (!Openable) { return; }
	if (Openable->ActorHasTag(TEXT("PassBlock=2")))   // the Wellness Center doors: they just let you through with pass 2
	{
		const FVector At = Openable->GetActorLocation();
		if (GetSoulBits() < 2) { PlaySfx(TEXT("sfx_door_locked"), At); ShowSubtitle(TEXT(""), TEXT("WELLNESS CENTER. Access pass 2 required. Wellness is a privilege."), 3.f); return; }
		PlaySfx(TEXT("sfx_badge_ok"), At, 0.8f);
		PlaySfx(TEXT("sfx_door_open"), At, 0.8f);
		Openable->SetActorHiddenInGame(true);
		Openable->SetActorEnableCollision(false);
		ShowSubtitle(TEXT(""), TEXT("The Wellness Center. It smells like a hospital pretending not to be one."), 3.5f);
		return;
	}
	FSwing* S = Swings.Find(Openable);
	if (!S)
	{
		FSwing New;
		New.ClosedYaw = Openable->GetActorRotation().Yaw;
		New.ClosedLoc = Openable->GetActorLocation();
		New.SlideDir = Openable->GetActorRightVector();   // our props face local +Y
		for (const FName& Tag : Openable->Tags)
		{
			const FString T = Tag.ToString();
			if (T.StartsWith(TEXT("OpenYaw="))) { New.OpenYaw = FCString::Atof(*T.Mid(8)); }
			if (T.StartsWith(TEXT("Slide="))) { New.Slide = FCString::Atof(*T.Mid(6)); New.OpenYaw = 0.f; }
		}
		S = &Swings.Add(Openable, New);
	}
	const FVector Where = Openable->GetActorLocation() + FVector(0.f, 0.f, 100.f);
	if (IsLockedForMe(Openable))
	{
		S->JiggleLeft = 0.45f;
		if (Openable->ActorHasTag(TEXT("Occupied")))   // the restroom stall: always occupied
		{
			PlaySfx(TEXT("sfx_knock"), Where, 0.8f);
			float StallFear = 0.f;
			for (TActorIterator<ATakeFearDirector> It(GetWorld()); It; ++It) { StallFear = It->Fear; break; }
			static const TCHAR* Calm[] = {
				TEXT("(from inside)  Occupied."), TEXT("(from inside)  Still occupied."), TEXT("(from inside)  ...Occupied, 4471."),
				TEXT("(from inside)  It's been occupied since this morning. It'll be occupied tomorrow."),
				TEXT("(from inside)  Have you tried the one on the other floor? ...There isn't one."),
			};
			if (StallFear >= 0.5f) { ShowSubtitle(TEXT(""), TEXT("(from inside, very close to the door)  Occupied. Occupied. Occupied."), 3.f); }
			else { ShowSubtitle(TEXT(""), Calm[FMath::Min(Knocks, int32(UE_ARRAY_COUNT(Calm)) - 1)], 3.f); }
			++Knocks;
			return;
		}
		PlaySfx(TEXT("sfx_door_locked"), Where);
		const int32 Need = PassNeeded(Openable);
		ShowSubtitle(TEXT(""), Need > 0 ? FString::Printf(TEXT("Locked. Access pass %d required."), Need) : FString(TEXT("Locked.")), 2.5f);
		return;
	}
	if (PassNeeded(Openable) > 0 && !S->bUnlocked)
	{
		S->bUnlocked = true;
		PlaySfx(TEXT("sfx_badge_ok"), Where, 0.8f);
		ShowSubtitle(TEXT(""), TEXT("Access pass accepted."), 2.f);
	}
	S->bOpen = !S->bOpen;
	PlaySfx(S->bOpen ? TEXT("sfx_door_open") : TEXT("sfx_door_close"), Where, S->Slide > 0.f ? 0.45f : 0.8f);
	if (S->bOpen && S->Slide > 0.f)   // a drawer with something worth reading in it
	{
		for (const FName& Tag : Openable->Tags)
		{
			if (Tag.ToString().StartsWith(TEXT("InspectTex="))) { Inspect(Openable); break; }
		}
	}
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
		if (FMath::IsNearlyEqual(S.Alpha, Goal) && S.JiggleLeft <= 0.f) { continue; }
		S.Alpha = FMath::FInterpConstantTo(S.Alpha, Goal, 1.f, Step);
		const float Eased = FMath::InterpEaseInOut(0.f, 1.f, S.Alpha, 2.f);
		float Jiggle = 0.f;
		if (S.JiggleLeft > 0.f)   // the handle rattles against the lock
		{
			S.JiggleLeft = FMath::Max(0.f, S.JiggleLeft - DeltaTime);
			Jiggle = 1.4f * FMath::Sin(S.JiggleLeft * 70.f) * (S.JiggleLeft / 0.45f);
		}
		if (S.Slide > 0.f)   // drawers slide straight out
		{
			A->SetActorLocation(S.ClosedLoc + S.SlideDir * S.Slide * Eased);
			continue;
		}
		FRotator R = A->GetActorRotation();
		R.Yaw = S.ClosedYaw + S.OpenYaw * Eased + Jiggle;
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
	SeatReturnRotation = PC->GetControlRotation();
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
	if (PC) { PC->SetControlRotation(FRotator(0.f, PC->GetControlRotation().Yaw, 0.f)); }   // look ahead, not at the ceiling
	OnLoggedOut.Broadcast(SeatedAt.Get());
}

// ------------------------------------------------------------------ Pause
void UTakeGrabComponent::TogglePause()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC || bSeated || Inspecting || LockPanel) { return; }
	if (PauseMenu)   // resume
	{
		PauseMenu->RemoveFromParent();
		PauseMenu = nullptr;
		UGameplayStatics::SetGamePaused(this, false);
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
		if (Prompt) { Prompt->SetVisibility(ESlateVisibility::HitTestInvisible); }
		return;
	}
	Drop();
	PauseMenu = CreateWidget<UTakePauseMenu>(PC, UTakePauseMenu::StaticClass());
	if (!PauseMenu) { return; }
	PauseMenu->SetIsFocusable(true);
	PauseMenu->OnResumeRequested.AddDynamic(this, &UTakeGrabComponent::HandleResume);
	PauseMenu->AddToViewport(50);
	if (Prompt) { Prompt->SetVisibility(ESlateVisibility::Collapsed); }
	UGameplayStatics::SetGamePaused(this, true);
	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(PauseMenu->TakeWidget());
	PC->SetInputMode(Mode);
	PC->SetShowMouseCursor(true);
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
void UTakeGrabComponent::PlaySfx(const TCHAR* Name, const FVector& Where, float Volume)
{
	if (USoundBase* Snd = LoadObject<USoundBase>(nullptr, *FString::Printf(TEXT("/Game/Audio/Placeholder/%s.%s"), Name, Name)))
	{
		UGameplayStatics::PlaySoundAtLocation(this, Snd, Where, Volume);
	}
}

bool UTakeGrabComponent::IsHoldingMug() const
{
	const UStaticMeshComponent* C = IsHolding() ? Cast<UStaticMeshComponent>(Handle->GetGrabbedComponent()) : nullptr;
	return C && C->GetStaticMesh() && C->GetStaticMesh()->GetName().Contains(TEXT("Mug"));
}

bool UTakeGrabComponent::IsHoldingSnack() const
{
	const UStaticMeshComponent* C = IsHolding() ? Cast<UStaticMeshComponent>(Handle->GetGrabbedComponent()) : nullptr;
	if (!C || !C->GetStaticMesh()) { return false; }
	const FString N = C->GetStaticMesh()->GetName();
	return N.Contains(TEXT("Snack")) || N.Contains(TEXT("Fruit")) || N.Contains(TEXT("SodaCan")) || N.Contains(TEXT("WaterBottle"))
		|| N.Contains(TEXT("LunchBox")) || N.Contains(TEXT("MilkCarton")) || N.Contains(TEXT("Sandwich")) || N.Contains(TEXT("Pizza"))
		|| N.Contains(TEXT("Salad")) || N.Contains(TEXT("Donut"));
}

void UTakeGrabComponent::Eat()
{
	if (!IsHoldingSnack() || bSeated) { return; }
	UPrimitiveComponent* Comp = Handle->GetGrabbedComponent();
	const FString N = Cast<UStaticMeshComponent>(Comp)->GetStaticMesh()->GetName();
	const bool bDrink = N.Contains(TEXT("Soda")) || N.Contains(TEXT("Water")) || N.Contains(TEXT("Milk"));
	PlaySfx(bDrink ? TEXT("sfx_sip_slurp") : TEXT("sfx_eat_crunch"), Comp->GetComponentLocation(), 0.85f);
	static const TCHAR* Bites[] = {
		TEXT("*crunch* Tastes like the vending machine smells."),
		TEXT("*crunch* Someone wrote 4471 on the wrapper."),
		TEXT("*crunch* Best before: today. It's always today."),
		TEXT("*crunch* ...It was your favourite. You don't remember why."),
	};
	static const TCHAR* Drinks[] = {
		TEXT("*gulp* Room temperature. Every room's temperature."),
		TEXT("*gulp* Refreshing. Mandatory refreshment."),
	};
	// Lunch (Act 2) has its own lines, one per dish
	struct FDish { const TCHAR* Key; const TCHAR* Line; };
	static const FDish Dishes[] = {
		{ TEXT("Sandwich"), TEXT("*crunch* Turkey. You asked for turkey. You don't remember asking.") },
		{ TEXT("Pizza"), TEXT("*crunch* Still warm. It was delivered at 12:00. It's always 12:00 somewhere in here.") },
		{ TEXT("Salad"), TEXT("*crunch* Crisp. Every leaf is the same size. Every leaf.") },
		{ TEXT("Donut"), TEXT("*chew* Sprinkles spell something. You ate it before you could read it.") },
		{ TEXT("LunchBox"), TEXT("*chew* Leftovers. The note on the lid says 4471. In your handwriting.") },
	};
	const TCHAR* DishLine = nullptr;
	for (const FDish& D : Dishes) { if (N.Contains(D.Key)) { DishLine = D.Line; break; } }
	if (DishLine) { ShowSubtitle(TEXT(""), DishLine, 3.f); }
	else if (bDrink) { ShowSubtitle(TEXT(""), Drinks[Eaten % int32(UE_ARRAY_COUNT(Drinks))], 2.5f); }
	else { ShowSubtitle(TEXT(""), Bites[Eaten % int32(UE_ARRAY_COUNT(Bites))], 2.5f); }
	++Eaten;
	if (UTakeGameInstance* EatGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { EatGI->AddEnergy(bDrink ? 12.f : 20.f); }
	Handle->ReleaseComponent();
	if (AActor* Food = Comp->GetOwner())
	{
		Food->SetActorHiddenInGame(true);       // gone from the kitchen
		Food->SetActorEnableCollision(false);
		Comp->SetSimulatePhysics(false);
		FoodEaten(Food);                         // ...and back on the shelf later (the kitchen is always stocked)
	}
}

void UTakeGrabComponent::Sip()
{
	if (IsHoldingSnack()) { Eat(); return; }   // F: eat / drink whatever you're holding
	if (!IsHoldingMug() || SipLeft > 0.f || bSeated) { return; }
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	SipLeft = 0.9f;
	if (!GI || !GI->bGotCoffee)
	{
		ShowSubtitle(TEXT(""), TEXT("It's empty. The coffee machine is in the kitchen."), 3.f);
		return;
	}
	PlaySfx(TEXT("sfx_sip_slurp"), Handle->GetGrabbedComponent()->GetComponentLocation(), 0.9f);
	static const TCHAR* Lines[] = {
		TEXT("*slurp* Perfect. It's always perfect."),
		TEXT("*slurp* Tastes like Monday."),
		TEXT("*slurp* Still exactly the right temperature."),
		TEXT("*slurp* ...Is it getting fuller?"),
		TEXT("*slurrrp*"),
	};
	ShowSubtitle(TEXT(""), Lines[Sips % int32(UE_ARRAY_COUNT(Lines))], 2.5f);
	++Sips;
	GI->AddEnergy(10.f);
}

int32 UTakeGrabComponent::PassNeeded(const AActor* Door) const
{
	if (!Door) { return 0; }
	for (const FName& Tag : Door->Tags)
	{
		const FString T = Tag.ToString();
		if (T.StartsWith(TEXT("NeedsPass="))) { return FCString::Atoi(*T.Mid(10)); }
	}
	return 0;
}

bool UTakeGrabComponent::IsLockedForMe(const AActor* Door) const
{
	if (!Door) { return false; }
	if (Door->ActorHasTag(TEXT("Locked"))) { return true; }   // locked for good (or until a script unlocks it)
	const int32 Need = PassNeeded(Door);
	return Need > 0 && GetSoulBits() < Need;
}

void UTakeGrabComponent::TickFootsteps(float DeltaTime)
{
	const ACharacter* C = Cast<ACharacter>(GetOwner());
	if (!C || bSeated || StepSounds.Num() == 0) { return; }
	const UCharacterMovementComponent* Move = C->GetCharacterMovement();
	const float Speed = C->GetVelocity().Size2D();
	if (!Move || !Move->IsMovingOnGround() || Speed < 40.f) { StepAccum = 0.f; return; }
	// Walking pace: about two steps a second at walking speed, a little quicker if you run, slower crouched
	StepAccum += DeltaTime;
	const float Interval = C->bIsCrouched ? 0.7f : FMath::Clamp(300.f / Speed, 0.45f, 0.65f);
	if (StepAccum < Interval) { return; }
	StepAccum = 0.f;
	int32 Pick = FMath::RandRange(0, StepSounds.Num() - 1);
	if (Pick == LastStep && StepSounds.Num() > 1) { Pick = (Pick + 1) % StepSounds.Num(); }
	LastStep = Pick;
	const FVector Feet = C->GetActorLocation() - FVector(0.f, 0.f, C->GetSimpleCollisionHalfHeight());
	UGameplayStatics::PlaySoundAtLocation(this, StepSounds[Pick], Feet, C->bIsCrouched ? 0.18f : 0.32f, FMath::FRandRange(0.94f, 1.06f));
}

void UTakeGrabComponent::BindPropImpacts()
{
	int32 N = 0;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (!It->ActorHasTag(TEXT("Throwable"))) { continue; }
		TArray<UPrimitiveComponent*> Prims;
		It->GetComponents<UPrimitiveComponent>(Prims);
		for (UPrimitiveComponent* P : Prims)
		{
			if (!P->IsSimulatingPhysics()) { continue; }
			P->SetNotifyRigidBodyCollision(true);
			P->OnComponentHit.AddUniqueDynamic(this, &UTakeGrabComponent::OnPropHit);
			++N;
		}
	}
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Impact sounds on %d prop(s)"), N);
	for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
	{
		const UStaticMeshComponent* SM = It->GetStaticMeshComponent();
		if (SM && SM->GetStaticMesh() && SM->GetStaticMesh()->GetName().Contains(TEXT("DeskPhone")))
		{
			Phones.Add(*It);
			if (It->ActorHasTag(TEXT("Phone4471"))) { MyPhone = *It; }
		}
	}
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] %d desk phone(s), yours %s"), Phones.Num(), MyPhone.IsValid() ? TEXT("found") : TEXT("missing"));

	// Screens: every monitor's "M_Screen" slot flips between the MI_Screen_* images
	auto LoadMat = [](const TCHAR* Name) -> UMaterialInterface*
	{
		return LoadObject<UMaterialInterface>(nullptr, *FString::Printf(TEXT("/Game/Props/Materials/%s.%s"), Name, Name));
	};
	for (const TCHAR* MatName : { TEXT("MI_Screen_Inbox"), TEXT("MI_Screen_Sheet"), TEXT("MI_Screen_Code"), TEXT("MI_Screen_Chart"),
	                        TEXT("MI_Screen_Call"), TEXT("MI_Screen_Saver") })
	{
		if (UMaterialInterface* M = LoadMat(MatName)) { WorkScreens.Add(M); }
	}
	for (const TCHAR* MatName : { TEXT("MI_Screen_Fear"), TEXT("MI_Screen_Glyphs") })
	{
		if (UMaterialInterface* M = LoadMat(MatName)) { FearScreens.Add(M); }
	}
	LoginScreen = LoadMat(TEXT("MI_Screen_Login"));
	InboxScreen = LoadMat(TEXT("MI_Screen_Inbox"));
	for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
	{
		UStaticMeshComponent* SM = It->GetStaticMeshComponent();
		if (!SM || !SM->GetStaticMesh()) { continue; }
		if (SM->GetStaticMesh()->GetName().Contains(TEXT("Monitor")))
		{
			const int32 Slot = SM->GetMaterialIndex(FName(TEXT("M_Screen")));
			if (Slot != INDEX_NONE && WorkScreens.Num() > 0 && !It->ActorHasTag(TEXT("FixedScreen")))   // security monitors keep their feeds
			{
				FScreen Scr;
				Scr.Comp = SM;
				Scr.Slot = Slot;
				Scr.Next = FMath::FRandRange(0.f, 2.f);
				Scr.bMine = It->ActorHasTag(TEXT("Monitor4471"));
				Screens.Add(Scr);
			}
		}
		if (It->ActorHasTag(TEXT("DeskItem")))
		{
			FDeskItem D;
			D.Actor = *It;
			D.Home = It->GetActorTransform();
			D.bPhysics = SM->IsSimulatingPhysics();
			DeskItems.Add(D);
		}
	}
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] %d screen(s) (%d images), %d desk item(s)"), Screens.Num(), WorkScreens.Num(), DeskItems.Num());
}

void UTakeGrabComponent::OnPropHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HitComp) { return; }
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now - PlayStartTime < 2.0) { return; }   // props settling at the start
	const float Speed = HitComp->GetComponentVelocity().Size();
	if (Speed < 160.f) { return; }
	double& Last = LastImpact.FindOrAdd(HitComp);
	if (Now - Last < 0.15) { return; }
	Last = Now;
	const bool bHeavy = HitComp->GetMass() > 8.f;
	const float Vol = FMath::GetMappedRangeValueClamped(FVector2D(160.f, 900.f), FVector2D(0.2f, 0.9f), Speed);
	PlaySfx(bHeavy ? TEXT("sfx_impact_heavy") : TEXT("sfx_impact_light"), Hit.ImpactPoint, Vol);
	if (ATakeCoworker* Victim = Cast<ATakeCoworker>(OtherActor))
	{
		Victim->HitByProp(Speed);
		// (the new-mail chime comes from TickMail when Security / HR write to you)
	}
}

void UTakeGrabComponent::ShowSubtitleSequence(const FString& Who, const TArray<FString>& Lines)
{
	// The sequence starts now; anything that was already queued (an objective cue) plays after it
	while (SubQueueWho.Num() < SubQueue.Num()) { SubQueueWho.Add(FString()); }
	SubQueue.Insert(Lines, 0);
	TArray<FString> Whos;
	Whos.Init(Who, Lines.Num());
	SubQueueWho.Insert(Whos, 0);
	SubtitleLeft = 0.f;   // start now
}

void UTakeGrabComponent::RingPhone(AActor* Phone)
{
	AActor* Target = Phone ? Phone : MyPhone.Get();
	if (!Target && Phones.Num() > 0) { Target = Phones[0].Get(); }
	if (!Target || RingingPhone.IsValid()) { return; }
	RingingPhone = Target;
	RingLeft = 12.f;    // four rings, then they give up
	RingCycle = 0.f;
}

void UTakeGrabComponent::StopRinging()
{
	if (RingAudio) { RingAudio->Stop(); RingAudio = nullptr; }
	RingingPhone = nullptr;
	RingLeft = 0.f;
}

void UTakeGrabComponent::TickPhones(float DeltaTime)
{
	if (Phones.Num() == 0) { return; }
	if (AActor* P = RingingPhone.Get())
	{
		if (!bSeated) { RingLeft -= DeltaTime; }   // it waits for you to log out
		RingCycle -= DeltaTime;
		if (RingLeft <= 0.f)
		{
			const bool bWasMine = P == MyPhone.Get();
			StopRinging();
			// Missed your own phone? It calls back soon (reminders sooner), like a real office. Other desks give up.
			bCallBackMine = bWasMine;
			NextRingIn = bWasMine ? (ReminderRinging > 0 ? FMath::FRandRange(15.f, 25.f) : FMath::FRandRange(30.f, 45.f)) : FMath::FRandRange(60.f, 120.f);
			ReminderRinging = 0;
			return;
		}
		if (RingCycle <= 0.f)
		{
			RingCycle = 3.f;
			if (USoundBase* Ring = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Placeholder/sfx_phone_ring.sfx_phone_ring")))
			{
				RingAudio = UGameplayStatics::SpawnSoundAtLocation(this, Ring, P->GetActorLocation() + FVector(0.f, 0.f, 10.f), FRotator::ZeroRotator, 0.7f);
			}
		}
		return;
	}
	if (PauseMenu) { return; }
	NextRingIn -= DeltaTime;
	if (TickReminders()) { NextRingIn = FMath::Max(NextRingIn, FMath::FRandRange(60.f, 120.f)); return; }   // the calendar calls about the 12:30 / 1:00
	if (NextRingIn > 0.f) { return; }
	NextRingIn = FMath::FRandRange(60.f, 120.f);
	// Your phone the first time (and again if you missed it), then mostly other people's (yours now and then)
	const bool bMine = MyPhone.IsValid() && (MyCalls == 0 || bCallBackMine || (MyCalls < 5 && FMath::FRand() < 0.35f));
	bCallBackMine = false;
	RingPhone(bMine ? MyPhone.Get() : Phones[FMath::RandRange(0, Phones.Num() - 1)].Get());
}

bool UTakeGrabComponent::PhoneRingingNear() const
{
	const AActor* P = RingingPhone.Get();
	return P && FVector::Dist(P->GetActorLocation(), GetOwner()->GetActorLocation()) <= UseRange + 100.f;
}

void UTakeGrabComponent::HandleTerminalLogout()
{
	LogOut();   // close your email first; the phone keeps ringing until you stand up
}

void UTakeGrabComponent::AnswerPhone(AActor* Phone)
{
	const bool bMine = Phone && Phone == MyPhone.Get();
	StopRinging();
	const FVector Where = Phone ? Phone->GetActorLocation() : GetOwner()->GetActorLocation();
	PlaySfx(TEXT("sfx_phone_pickup"), Where, 0.8f);
	if (UTakeGameInstance* PhoneGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { PhoneGI->AddEnergy(-5.f); }   // calls are draining
	PlaySfx(TEXT("sfx_phone_static"), Where, 0.5f);   // the line crackles under the voices (voices coming later)
	TArray<FString> Lines;
	if (bMine && ReminderRinging > 0)
	{
		if (ReminderRinging == 1)
		{
			bCallReminded = true;
			Lines = { TEXT("HALCYON Calendar. This is a reminder for: Sync. Twelve thirty. Conference Room."),
			          TEXT("Your camera is optional. Theirs is not."), TEXT("Press E on the big screen to join. *click*") };
		}
		else
		{
			bMeetingReminded = true;
			Lines = { TEXT("HALCYON Calendar. Your one o'clock is starting. Conference Room."),
			          TEXT("It is the same as your twelve thirty. Please attend both."), TEXT("*click*") };
		}
		ReminderRinging = 0;
		ShowSubtitleSequence(TEXT("Calendar"), Lines);
		return;
	}
	if (bMine)
	{
		switch (MyCalls++)
		{
		case 0:
			Lines = { TEXT("HALCYON automated wellness check. Please hold."), TEXT("(soft hold music, the same four notes)"),
			          TEXT("Thank you for holding, 4471. You are exactly where you need to be.") };
			break;
		case 1:
			Lines = { TEXT("(static) ...is this take four?"), TEXT("No. Four-four-seven-one. Sorry. Wrong take."), TEXT("*click*") };
			break;
		case 2:
			Lines = { TEXT("(breathing)"), TEXT("...Places.") };
			break;
		case 3:
			Lines = { TEXT("Hi. It's you. From later."), TEXT("Don't trust the fragments. Look closer."), TEXT("*click*") };
			for (TActorIterator<ATakeFearDirector> It(GetWorld()); It; ++It) { It->StartCreep(FMath::Max(It->Fear, 0.2f), 3.f); break; }
			break;
		default:
			Lines = { TEXT("(dial tone)") };
			PlaySfx(TEXT("sfx_phone_dialtone"), Where, 0.6f);
			break;
		}
		ShowSubtitleSequence(MyCalls == 1 ? TEXT("HALCYON") : TEXT("Phone"), Lines);
	}
	else
	{
		static const TCHAR* Others[] = {
			TEXT("Wrong desk. *click*"),
			TEXT("...Hello? ...Oh. You're not supposed to be there yet. *click*"),
			TEXT("Is Dana there? ...Dana's never been there. Sorry. *click*"),
			TEXT("(someone laughing, very far away) *click*"),
		};
		if (FMath::FRand() < 0.25f)
		{
			PlaySfx(TEXT("sfx_phone_dialtone"), Where, 0.6f);
			ShowSubtitle(TEXT("Phone"), TEXT("(dial tone)"), 2.f);
		}
		else
		{
			ShowSubtitle(TEXT("Phone"), Others[FMath::RandRange(0, int32(UE_ARRAY_COUNT(Others)) - 1)], 4.f);
		}
	}
}

void UTakeGrabComponent::Inspect(AActor* Target)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC || !Target || Inspecting) { return; }
	// The picture is whatever texture the board's material shows right now (so a changed poster looks changed)
	UTexture2D* Tex = nullptr;
	for (const FName& Tag : Target->Tags)   // InspectTex=MI_X: show that picture instead (the moving box shows its label)
	{
		const FString T = Tag.ToString();
		if (!T.StartsWith(TEXT("InspectTex="))) { continue; }
		const FString Mi = T.Mid(11);
		if (UMaterialInterface* Mat = LoadObject<UMaterialInterface>(nullptr, *FString::Printf(TEXT("/Game/Props/Materials/%s.%s"), *Mi, *Mi)))
		{
			UTexture* Found = nullptr;
			if (Mat->GetTextureParameterValue(FMaterialParameterInfo(FName(TEXT("Tex"))), Found)) { Tex = Cast<UTexture2D>(Found); }
		}
	}
	if (!Tex)
	if (const UStaticMeshComponent* SM = Target->FindComponentByClass<UStaticMeshComponent>())
	{
		if (UMaterialInterface* Mat = SM->GetMaterial(0))
		{
			UTexture* T = nullptr;
			if (Mat->GetTextureParameterValue(FMaterialParameterInfo(FName(TEXT("Tex"))), T)) { Tex = Cast<UTexture2D>(T); }
		}
	}
	const FString Name = Tex ? Tex->GetName() : FString();
	const bool bBrokeLoop = TryBreakLoopAt(Target);   // after reading the texture: breaking restores the poster
	FString Title, Caption;
	int32 GlyphKey = 0;   // which Key Lock's signature is hidden on this one (0 = none)
	struct FCaption { const TCHAR* Key; const TCHAR* Title; const TCHAR* Text; int32 Glyphs; };
	static const FCaption Captions[] = {
		{ TEXT("File_4470"), TEXT("PERSONNEL FILE: 4470"), TEXT("Status: WRAPPED. Replacement: 4471 (scheduled).\nSomeone has written underneath, by hand: \"they looked closer.\""), 0 },
		{ TEXT("Label_4470"), TEXT("4470 - PERSONAL EFFECTS"), TEXT("A moving box with someone's desk in it. A mug, a frame, papers.\nYour badge says 4471. Whoever had this desk before you was 4470."), 0 },
		{ TEXT("Polaroid_4470"), TEXT("DESK 4471 - DAY 1"), TEXT("Your desk. Someone sitting at it, in a mint suit.\nThe face has been scribbled out. Hard. The pen went through the photo."), 0 },
		{ TEXT("Sticky_4470"), TEXT("A NOTE"), TEXT("\"if you're reading this it's not your first day. LOOK CLOSER. - 4470\"\nThe handwriting looks a lot like yours."), 0 },
		{ TEXT("Nameplate_4471"), TEXT("YOUR NAMEPLATE"), TEXT("4471, on a sticker. It doesn't quite cover the number underneath.\nPeel back the corner and it says 4470."), 0 },
		{ TEXT("Chart_4471"), TEXT("PATIENT 4471"), TEXT("Admitted: today (again). Condition: looked closer.\n\"Patient is reading this chart now.\" Squares in the margin."), 3 },
		{ TEXT("Chart_Stage"), TEXT("BED 7"), TEXT("INT. WARD - CONTINUOUS. \"4471 turns to the next bed. They will hesitate here.\"\nYou hesitate. The same squares, in the same pen."), 3 },
		{ TEXT("Chart_4470"), TEXT("PATIENT 4470"), TEXT("Status: WRAPPED. The squares on this chart were added later.\nThe handwriting doesn't match."), 7 },
		{ TEXT("Chart_Blank"), TEXT("PATIENT ____"), TEXT("Admitted: next take."), 0 },
		{ TEXT("Whiteboard"), TEXT("THE WHITEBOARD"), TEXT("Q4 energy targets, up and to the right. Retention: 100%. Retakes: as needed.\nIn the corner, in a different marker: a row of squares."), 2 },
		{ TEXT("KitchenBoard"), TEXT("KITCHEN NOTICES"), TEXT("Lost: red stapler. Cake in the fridge for 4471's birthday (again).\nSomeone keeps labelling a mug \"4470\". Morale is mandatory. Fun is at 2:14."), 0 },
		{ TEXT("ScriptNotes"), TEXT("THE RECEPTIONIST'S SCRIPT"), TEXT("INT. HALCYON LOBBY - MORNING. \"Welcome back. (beat) Oh -- first day. Of course.\"\nIn red pen, from C: smile with the eyes. AGAIN."), 0 },
		{ TEXT("SignInSheet"), TEXT("VISITOR SIGN-IN"), TEXT("4471, 9:00. 4471, 9:00. 4471, 9:00. Every line, the same handwriting.\nOne line says 4470, OUT: wrapped. The last line is waiting for you."), 0 },
		{ TEXT("Magazine"), TEXT("HALCYON QUARTERLY"), TEXT("Employee of the Take: 4471. The photo is you, at your desk.\nIssue date: tomorrow."), 0 },
		{ TEXT("LunchSpecials"), TEXT("TODAY'S SPECIALS"), TEXT("Pizza, delivered 12:00. Turkey, because you asked for turkey.\nSoup of the day: yesterday's."), 0 },
		{ TEXT("Mirror"), TEXT("THE MIRROR"), TEXT("You, in a mint suit. Your reflection blinks a moment after you do.\nThere are fingerprints on the glass. On the inside."), 0 },
		{ TEXT("Chairman"), TEXT("THE CHAIRMAN"), TEXT("Founder. Visionary. Always just out of frame.\nThe plaque says he's proud of you. It doesn't say for what."), 0 },
		{ TEXT("CallSheet"), TEXT("CALL SHEET"), TEXT("Today's date. A shooting schedule for an office.\nOne line is highlighted: EMPLOYEE 4471. You."), 0 },
		{ TEXT("FridgeNote_Yogurt"), TEXT("KITCHEN"), TEXT("Someone has strong feelings about yogurt."), 0 },
		{ TEXT("FridgeNote_Memo"), TEXT("KITCHEN"), TEXT("A memo about the fridge. It's signed by the fridge."), 0 },
		{ TEXT("Energy_Changed"), TEXT("YOUR ENERGY IS OUR GREATEST ASSET"), TEXT("The bolt is pointing at you now.\nIn the corner, very small, someone has drawn a row of squares."), 2 },
		{ TEXT("Present_Changed"), TEXT("BE PRESENT."), TEXT("The eye is closed.\nWasn't it open a minute ago?"), 0 },
		{ TEXT("CorridorB_Changed"), TEXT("THERE IS NO CORRIDOR B"), TEXT("The arrow points back the way you came.\nThat's the one."), 0 },
		{ TEXT("Smile_Changed"), TEXT("SMILE!"), TEXT("The smile is wider. Too wide."), 0 },
		{ TEXT("Mark_Changed"), TEXT("HIT YOUR MARK"), TEXT("The X is on the floor where you're standing."), 0 },
		{ TEXT("Wellness_Changed"), TEXT("WELLNESS CENTER"), TEXT("Open late. Open later. Open."), 0 },
		{ TEXT("Energy"), TEXT("YOUR ENERGY IS OUR GREATEST ASSET"), TEXT("A cheerful lightning bolt. HALCYON Morale Team, Q4.\nSomeone has scratched a pattern of squares into the frame."), 2 },
		{ TEXT("Present"), TEXT("BE PRESENT. BE PRESENT. BE PRESENT."), TEXT("A single open eye. It's printed slightly off-centre, so it's looking at you."), 0 },
		{ TEXT("CorridorB"), TEXT("THERE IS NO CORRIDOR B"), TEXT("Facilities notice. The arrow points down the corridor."), 0 },
		{ TEXT("Smile"), TEXT("SMILE!"), TEXT("The Morale Team reminds you that cameras are always rolling.\nUnder the logo: a faint row of squares."), 3 },
		{ TEXT("Mark"), TEXT("HIT YOUR MARK"), TEXT("Stand where you're told. Say what you're given."), 0 },
		{ TEXT("Wellness"), TEXT("WELLNESS CENTER"), TEXT("Rest. Recharge. Return. The building's own clinic, down Corridor B."), 0 },
	};
	for (const FCaption& C : Captions)
	{
		if (Name.Contains(C.Key)) { Title = C.Title; Caption = C.Text; GlyphKey = C.Glyphs; break; }
	}
	if (Title.IsEmpty() && Name.Contains(TEXT("Poster")))
	{
		Title = TEXT("HALCYON");
		Caption = TEXT("Every hallway has one. Every hallway has this one.");
	}
	Inspecting = CreateWidget<UTakeInspectWidget>(PC, UTakeInspectWidget::StaticClass());
	if (!Inspecting) { return; }
	Drop();
	Inspecting->Setup(Tex, Title, Caption);
	if (GlyphKey > 0)   // a Key Lock signature hidden in the artwork: shown as glyphs under the picture
	{
		// 7 = a decoy: squares from a different lock, noted under Key 3 (it's for you to notice they disagree)
		const FTakeKeyLock Lock = UTakeKeyLockLibrary::GenerateKeyLockForRun(this, GlyphKey, 0);
		Inspecting->SetGlyphs(Lock.Target, Lock.Bits);
		const int32 NoteKey = GlyphKey == 7 ? 3 : GlyphKey;
		const FString Source = Title.IsEmpty() ? FString(TEXT("a poster")) : FString::Printf(TEXT("\"%s\""), *Title.Left(28));
		if (UTakeGameInstance* NGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			if (NGI->AddGlyphNote(NoteKey, Source, Lock.Target, Lock.Bits)) { QueueSubtitle(TEXT(""), TEXT("(You note down the squares. Pause > Tasks to see your notes.)")); }
		}
	}
	Inspecting->SetIsFocusable(true);
	Inspecting->OnCloseRequested.AddDynamic(this, &UTakeGrabComponent::CloseInspect);
	Inspecting->AddToViewport(30);
	if (Prompt) { Prompt->SetVisibility(ESlateVisibility::Collapsed); }
	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(Inspecting->TakeWidget());
	PC->SetInputMode(Mode);
	PC->SetIgnoreLookInput(true);
	PC->SetIgnoreMoveInput(true);
}

void UTakeGrabComponent::CloseInspect()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (Inspecting) { Inspecting->RemoveFromParent(); Inspecting = nullptr; }
	if (Prompt) { Prompt->SetVisibility(ESlateVisibility::HitTestInvisible); }
	if (PC)
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetIgnoreLookInput(false);
		PC->SetIgnoreMoveInput(false);
	}
}

void UTakeGrabComponent::TickScreens(float DeltaTime)
{
	if (Screens.Num() == 0) { return; }
	float Fear = 0.f;
	for (TActorIterator<ATakeFearDirector> It(GetWorld()); It; ++It) { Fear = It->Fear; break; }
	const bool bFear = Fear >= 0.75f && FearScreens.Num() > 0;
	for (FScreen& Scr : Screens)
	{
		UStaticMeshComponent* C = Scr.Comp.Get();
		if (!C) { continue; }
		Scr.Next -= DeltaTime;
		if (Scr.Next > 0.f) { continue; }
		UMaterialInterface* M = nullptr;
		if (bFear)
		{
			M = FearScreens[FMath::RandRange(0, FearScreens.Num() - 1)];
			Scr.Next = FMath::FRandRange(0.08f, 0.6f);   // stutter
		}
		else if (Scr.bMine)
		{
			M = bSeated ? InboxScreen.Get() : LoginScreen.Get();
			Scr.Next = 0.5f;
		}
		else
		{
			M = WorkScreens[FMath::RandRange(0, WorkScreens.Num() - 1)];
			Scr.Next = FMath::FRandRange(2.5f, 7.f);       // someone switching windows
		}
		if (M) { C->SetMaterial(Scr.Slot, M); }
	}
}

void UTakeGrabComponent::TickDesk()
{
	if (DeskItems.Num() == 0) { return; }
	float Fear = 0.f;
	for (TActorIterator<ATakeFearDirector> It(GetWorld()); It; ++It) { Fear = It->Fear; break; }
	if (!bDeskThrown && Fear >= 0.75f)
	{
		bDeskThrown = true;   // everything on your desk lifts and scatters
		for (const FDeskItem& D : DeskItems)
		{
			AActor* A = D.Actor.Get();
			UPrimitiveComponent* P = A ? Cast<UPrimitiveComponent>(A->GetRootComponent()) : nullptr;
			if (!P || (IsHolding() && Handle->GetGrabbedComponent() == P)) { continue; }
			P->SetSimulatePhysics(true);
			P->AddImpulse(FVector(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(1.2f, 2.f)) * 180.f, NAME_None, true);
			P->AddAngularImpulseInDegrees(FVector(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f)) * 500.f, NAME_None, true);
		}
		return;
	}
	if (bDeskThrown && Fear < 0.25f)
	{
		// Only tidy up when you aren't watching: you come back and it's as if nothing happened
		const AActor* Any = DeskItems[0].Actor.Get();
		FVector Loc; FRotator Rot;
		if (Any && GetView(Loc, Rot))
		{
			const FVector To = Any->GetActorLocation() - Loc;
			if (To.Size() < 500.f && FVector::DotProduct(Rot.Vector(), To.GetSafeNormal()) > 0.2f) { return; }
		}
		bDeskThrown = false;
		for (const FDeskItem& D : DeskItems)
		{
			AActor* A = D.Actor.Get();
			UPrimitiveComponent* P = A ? Cast<UPrimitiveComponent>(A->GetRootComponent()) : nullptr;
			if (!P || (IsHolding() && Handle->GetGrabbedComponent() == P)) { continue; }
			P->SetSimulatePhysics(false);
			A->SetActorTransform(D.Home, false, nullptr, ETeleportType::ResetPhysics);
			if (D.bPhysics) { P->SetSimulatePhysics(true); }   // the mug stays a mug you can pick up
		}
	}
}

void UTakeGrabComponent::TickClock(float DeltaTime)
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI || !Prompt) { return; }
	// 1 game minute every 3 s. The morning stops at 11:59 until your tasks are done: it's always almost noon.
	// Act 2 starts at 12:00 (beat 1.10) and every clock stops at 2:14 (beat 2.6, FreezeClocks).
	if (GI->CurrentAct() >= 2)
	{
		if (GI->bClocksFrozen) { GI->WorkdayMinutes = FrozenClockMinutes; }
		else
		{
			// The afternoon keeps time with the story too: it's always almost 12:30 until you join the call,
			// almost 1:00 until you've sat through the meeting, and 2:13 until the rest of your list is done.
			float Cap = FrozenClockMinutes;
			if (!GI->bJoinedCall) { Cap = 12.f * 60.f + 29.f; }
			else if (!GI->bAttendedMeeting) { Cap = 12.f * 60.f + 59.f; }
			else if (!GI->Act2TasksDone()) { Cap = FrozenClockMinutes - 1.f; }
			if (CallLeft > 0.f) { Cap = FMath::Min(Cap, GI->WorkdayMinutes); }   // nobody checks the time during a call
			GI->WorkdayMinutes = FMath::Clamp(FMath::Min(GI->WorkdayMinutes + DeltaTime / 3.f, FMath::Max(Cap, GI->WorkdayMinutes)), 12.f * 60.f, FrozenClockMinutes);
			if (GI->WorkdayMinutes >= FrozenClockMinutes) { FreezeClocks(); }
		}
	}
	else
	{
		// Act 1 keeps time with the story: it's always almost ten until your morning tasks are done, ten o'clock
		// for the meeting, then the corridor eats the rest of the morning (time runs 3x faster) up to 11:59.
		float Cap = 9.f * 60.f + 55.f;
		float SecondsPerMinute = 3.f;
		if (GI->Beat == ETakeBeat::Meeting) { Cap = 10.f * 60.f; }
		else if (GI->Beat >= ETakeBeat::Corridor) { Cap = 10.f * 60.f + 59.f; }   // the corridor eats the hour, then the jump to 12:00
		if (GI->Beat >= ETakeBeat::Corridor && GI->WorkdayMinutes < 10.f * 60.f + 1.f) { GI->WorkdayMinutes = 10.f * 60.f + 1.f; }
		GI->WorkdayMinutes = FMath::Min(GI->WorkdayMinutes + DeltaTime / SecondsPerMinute, FMath::Max(Cap, GI->WorkdayMinutes));
	}
	const int32 M = FMath::FloorToInt(GI->WorkdayMinutes);
	const int32 H24 = M / 60;
	const int32 H12 = (H24 + 11) % 12 + 1;
	// Energy: ten little cells under the clock ("Your energy is our greatest asset")
	FString Cells;
	const int32 Lit = FMath::Clamp(FMath::CeilToInt(GI->Energy / 10.f), 0, 10);
	for (int32 C = 0; C < 10; ++C) { Cells += C < Lit ? TEXT("\u25A0") : TEXT("\u25A1"); }
	Prompt->SetClock(FString::Printf(TEXT("%d:%02d %s\nTAKE %d   \u00B7   PASS %d / 3\nENERGY  %s"), H12, M % 60, H24 < 12 ? TEXT("AM") : TEXT("PM"),
		GI->TakeNumber, GI->SoulBits, *Cells));
}

void UTakeGrabComponent::UpdateTasks()
{
	if (!Prompt) { return; }
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI) { return; }
	// TaskSignature changes whenever a task on the current act's list changes (and when the act changes)
	const int32 Mask = GI->TaskSignature();
	const bool bObjectiveChanged = GI->Objective != ObjectiveShown;
	if (Mask == TasksShown && !bObjectiveChanged) { return; }
	ObjectiveShown = GI->Objective;
	const bool bMorningJustDone = GI->CurrentAct() == 1 && Mask == 7 && TasksShown >= 0 && TasksShown != 7;
	TasksShown = Mask;
	FString Text = GI->CurrentAct() >= 2 ? TEXT("MIDDAY  \u00B7  EMPLOYEE 4471\n") : TEXT("TODAY  \u00B7  EMPLOYEE 4471\n");
	Text += GI->TaskListText();
	if (!GI->Objective.IsEmpty()) { Text += FString::Printf(TEXT("\n\nNEXT  \u00B7  %s"), *GI->Objective); }
	else if (Mask == 7) { Text += TEXT("\n\nNext: 10:00 meeting, Conference Room"); }
	Prompt->SetTasks(Text);
	if (bMorningJustDone)
	{
		ShowSubtitle(TEXT("PA"), TEXT("Thank you, 4471. Your morning is complete. Please proceed to your 10:00 in the Conference Room."), 6.f);
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Morning tasks complete"));
	}
	if (bObjectiveChanged && !GI->Objective.IsEmpty())   // after the PA line above, so the cue queues behind it
	{
		QueueSubtitle(TEXT(""), FString::Printf(TEXT("New objective:  %s"), *GI->Objective));
		PlaySfx2D(TEXT("sfx_badge_ok"), 0.35f);
	}
}

void UTakeGrabComponent::ShowSubtitle(const FString& Who, const FString& Line, float Seconds)
{
	if (!Prompt) { return; }
	Prompt->SetSubtitle(Who.IsEmpty() ? Line : FString::Printf(TEXT("%s:  %s"), *Who, *Line));
	SubtitleLeft = Seconds;
}

void UTakeGrabComponent::UpdatePrompt()
{
	if (!Prompt) { return; }
	if (bSeated) { Prompt->SetPrompt(TEXT("")); return; }
	if (IsHolding())
	{
		FHitResult MugHit;
		if (Look(MugHit) == ETakeLookKind::Coffee)
		{
			Prompt->SetPrompt(IsHoldingMug() ? TEXT("E  Refill        F  Sip        Click  Throw") : TEXT("E  Fill it anyway        Click  Throw"));
			return;
		}
		Prompt->SetPrompt(IsHoldingMug() ? TEXT("F  Sip        E  Drop        Click  Throw")
			: IsHoldingSnack() ? TEXT("F  Eat        E  Drop        Click  Throw") : TEXT("E  Drop        Click  Throw"));
		return;
	}
	FHitResult Hit;
	switch (Look(Hit))
	{
	case ETakeLookKind::Coworker:
	{
		const ATakeCoworker* C = Cast<ATakeCoworker>(Hit.GetActor());
		Prompt->SetPrompt((C && C->IsInConversation()) ? TEXT("E  Continue") : TEXT("E  Talk"));
		break;
	}
	case ETakeLookKind::Terminal: Prompt->SetPrompt(TEXT("E  Sit and log in")); break;
	case ETakeLookKind::Openable:
	{
		const FSwing* S = Swings.Find(Hit.GetActor());
		const bool bDrawer = Hit.GetActor()->ActorHasTag(TEXT("FileDrawer"));
		Prompt->SetPrompt(Hit.GetActor()->ActorHasTag(TEXT("Occupied")) ? TEXT("E  Knock")
			: IsLockedForMe(Hit.GetActor()) ? TEXT("E  Try the door")
			: ((S && S->bOpen) ? (bDrawer ? TEXT("E  Close the drawer") : TEXT("E  Close")) : (bDrawer ? TEXT("E  Open the drawer") : TEXT("E  Open"))));
		break;
	}
	case ETakeLookKind::Coffee:   Prompt->SetPrompt(TEXT("E  Pour a coffee")); break;
	case ETakeLookKind::Phone:    Prompt->SetPrompt(TEXT("E  Answer the phone")); break;
	case ETakeLookKind::Inspect:  Prompt->SetPrompt(TEXT("E  Look closer")); break;
	case ETakeLookKind::Call:
	{
		const UTakeGameInstance* CallGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
		Prompt->SetPrompt((CallGI && CallGI->bJoinedCall) ? TEXT("E  Join the 1:00 meeting") : TEXT("E  Join the call  (Sync 12:30)"));
		break;
	}
	case ETakeLookKind::Vending:  Prompt->SetPrompt(TEXT("E  Buy a snack")); break;
	case ETakeLookKind::LockPanel: Prompt->SetPrompt(TEXT("E  Log in")); break;
	case ETakeLookKind::Use:
	{
		const FString K = UseKey(Hit.GetActor());
		Prompt->SetPrompt(K == TEXT("Sink") ? TEXT("E  Wash your hands") : K == TEXT("Dryer") ? TEXT("E  Dry your hands")
			: K == TEXT("Toilet") ? TEXT("E  Flush") : K == TEXT("Printer") ? TEXT("E  Print") : K == TEXT("Cooler") ? TEXT("E  Get some water")
			: K == TEXT("Elevator") ? TEXT("E  Press a button") : K == TEXT("TrustFall") ? TEXT("E  Fall backward")
			: K == TEXT("ServiceElevator") ? TEXT("E  Call the service elevator") : TEXT("E  Use"));
		break;
	}
	case ETakeLookKind::Wisp:     Prompt->SetPrompt(TEXT("E  Reach for the light")); break;
	case ETakeLookKind::Prop:     Prompt->SetPrompt(TEXT("E  Pick up")); break;
	case ETakeLookKind::TooHeavy: Prompt->SetPrompt(TEXT("Too heavy")); break;
	default:                      Prompt->SetPrompt(TEXT("")); break;
	}
}

void UTakeGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickSwings(DeltaTime);
	SipLeft = FMath::Max(0.f, SipLeft - DeltaTime);
	TickClock(DeltaTime);
	TickFootsteps(DeltaTime);
	TickPhones(DeltaTime);
	TickScreens(DeltaTime);
	TickDesk();
	TickSlate(DeltaTime);
	TickStory();
	TickAct2(DeltaTime);
	TickCall(DeltaTime);
	TickAct3(DeltaTime);
	TickFun(DeltaTime);
	TickMail(DeltaTime);
	TickFearReveals();
	TickEnergy(DeltaTime);
	TickCams(DeltaTime);
	UpdatePrompt();
	UpdateTasks();
	if (SubtitleLeft > 0.f)
	{
		SubtitleLeft -= DeltaTime;
		if (SubtitleLeft <= 0.f && Prompt) { Prompt->SetSubtitle(TEXT("")); }
	}
	if (SubtitleLeft <= 0.f && SubQueue.Num() > 0)
	{
		const FString Next = SubQueue[0];
		const FString NextWho = SubQueueWho.Num() > 0 ? SubQueueWho[0] : FString();
		SubQueue.RemoveAt(0);
		if (SubQueueWho.Num() > 0) { SubQueueWho.RemoveAt(0); }
		ShowSubtitle(NextWho, Next, FMath::Max(2.5f, Next.Len() / 14.f));
	}
	if (!IsHolding()) { return; }
	FVector Loc; FRotator Rot;
	if (GetView(Loc, Rot))
	{
		if (SipLeft > 0.f && IsHoldingMug())   // bring the mug up to your mouth and tip it toward you
		{
			const float A = FMath::Sin(PI * FMath::Clamp(1.f - SipLeft / 0.9f, 0.f, 1.f));
			const FVector Flat = FRotator(0.f, Rot.Yaw, 0.f).Vector();
			const FVector Right = FRotator(0.f, Rot.Yaw + 90.f, 0.f).Vector();
			const FVector Rest = Loc + Rot.Vector() * MugHoldDistance - FVector(0.f, 0.f, 12.f);
			const FVector Mouth = Loc + Flat * 30.f - FVector(0.f, 0.f, 8.f);
			const FQuat Base = FRotator(HeldPitchRoll.Pitch, Rot.Yaw + HeldYawOffset, HeldPitchRoll.Roll).Quaternion();
			const FQuat Tip = FQuat(Right, FMath::DegreesToRadians(-55.f * A));   // top of the mug tips back toward your face
			Handle->SetTargetLocationAndRotation(FMath::Lerp(Rest, Mouth, A), (Tip * Base).Rotator());
		}
		else if (IsHoldingMug() || IsHoldingSnack())   // small things are held closer, a little low, like a real hand
		{
			Handle->SetTargetLocationAndRotation(Loc + Rot.Vector() * MugHoldDistance - FVector(0.f, 0.f, 12.f),
			                                     FRotator(HeldPitchRoll.Pitch, Rot.Yaw + HeldYawOffset, HeldPitchRoll.Roll));
		}
		else
		{
			Handle->SetTargetLocationAndRotation(Loc + Rot.Vector() * HoldDistance, FRotator(HeldPitchRoll.Pitch, Rot.Yaw + HeldYawOffset, HeldPitchRoll.Roll));
		}
	}
}
