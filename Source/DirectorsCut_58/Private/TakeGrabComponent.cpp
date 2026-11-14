// TAKE 4471 - interact / grab / throw / telekinesis / desk login / doors
#include "TakeGrabComponent.h"
#include "DirectorsCut_58.h"
#include "TakeGameInstance.h"
#include "TakeCoworker.h"
#include "TakeFearDirector.h"
#include "TakeWidgets.h"
#include "TakePauseMenu.h"
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
	if (PauseMenu) { PauseMenu->RemoveFromParent(); PauseMenu = nullptr; }
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
	IC->BindKey(EKeys::Escape, IE_Pressed, this, &UTakeGrabComponent::TogglePause);   // in the editor Esc stops Play: use P
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

ATakeCoworker* UTakeGrabComponent::CoworkerInView() const
{
	// Coworkers are Pawns, and Pawns ignore the Visibility trace: pick the one you're facing instead.
	FVector Loc; FRotator Rot;
	if (!GetView(Loc, Rot)) { return nullptr; }
	ATakeCoworker* Best = nullptr;
	float BestDot = 0.9f;
	for (TActorIterator<ATakeCoworker> It(GetWorld()); It; ++It)
	{
		if (!It->bStoryNPC) { continue; }   // office extras talk on their own as you pass
		const FVector To = It->GetActorLocation() + FVector(0.f, 0.f, 40.f) - Loc;
		if (To.Size() > UseRange) { continue; }
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
	const FVector End = Loc + Rot.Vector() * FMath::Max(CurrentRange(), UseRange);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(TakeLook), false, GetOwner());
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Loc, End, ECC_Visibility, Params)) { return ETakeLookKind::None; }
	AActor* A = Hit.GetActor();
	UPrimitiveComponent* Comp = Hit.GetComponent();
	const bool bNear = Hit.Distance <= UseRange;
	if (A && bNear)
	{
		if (const ATakeCoworker* C = Cast<ATakeCoworker>(A))
		{
			return C->bStoryNPC ? ETakeLookKind::Coworker : ETakeLookKind::None;   // extras talk on their own
		}
		if (A->ActorHasTag(TEXT("Terminal"))) { return ETakeLookKind::Terminal; }
		if (A->ActorHasTag(TEXT("Openable"))) { return ETakeLookKind::Openable; }
		if (A->ActorHasTag(TEXT("CoffeeMachine"))) { return ETakeLookKind::Coffee; }
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
	case ETakeLookKind::Coffee:    PourCoffee(); break;
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
	PlaySfx(TEXT("sfx_pickup"), Comp->GetComponentLocation(), 0.6f);
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
	PlaySfx(TEXT("sfx_throw"), Comp->GetComponentLocation(), 0.7f);
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
	const FVector Where = Openable->GetActorLocation() + FVector(0.f, 0.f, 100.f);
	if (IsLockedForMe(Openable))
	{
		S->JiggleLeft = 0.45f;
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
	PlaySfx(S->bOpen ? TEXT("sfx_door_open") : TEXT("sfx_door_close"), Where, 0.8f);
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
	if (!PC || bSeated) { return; }
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
void UTakeGrabComponent::PourCoffee()
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	static const TCHAR* Pours[] = {
		TEXT("You pour a coffee. It's exactly the right temperature. It always is."),
		TEXT("Another cup. The mug already has your name on it."),
		TEXT("The machine hums the HALCYON jingle while it pours."),
	};
	const int32 Count = UE_ARRAY_COUNT(Pours);
	ShowSubtitle(TEXT(""), Pours[(GI && GI->bGotCoffee) ? FMath::RandRange(1, Count - 1) : 0], 4.f);
	if (GI) { GI->bGotCoffee = true; }
	FHitResult Hit;
	if (Look(Hit) == ETakeLookKind::Coffee) { PlaySfx(TEXT("sfx_coffee_pour"), Hit.ImpactPoint); }
}

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

void UTakeGrabComponent::Sip()
{
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
	StepAccum += Speed * DeltaTime;
	const float Stride = C->bIsCrouched ? 55.f : 78.f;
	if (StepAccum < Stride) { return; }
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
}

void UTakeGrabComponent::TickClock(float DeltaTime)
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI || !Prompt) { return; }
	// 1 game minute every 3 s. The morning stops at 11:59 until your tasks are done: it's always almost noon.
	const float Cap = GI->MorningTasksDone() ? 12.f * 60.f + 0.f : 11.f * 60.f + 59.f;
	{ GI->WorkdayMinutes = FMath::Min(GI->WorkdayMinutes + DeltaTime / 3.f, FMath::Max(Cap, GI->WorkdayMinutes)); }
	const int32 M = FMath::FloorToInt(GI->WorkdayMinutes);
	const int32 H24 = M / 60;
	const int32 H12 = (H24 + 11) % 12 + 1;
	Prompt->SetClock(FString::Printf(TEXT("%d:%02d %s\nTAKE %d   \u00B7   PASS %d / 3"), H12, M % 60, H24 < 12 ? TEXT("AM") : TEXT("PM"), GI->TakeNumber, GI->SoulBits));
}

void UTakeGrabComponent::UpdateTasks()
{
	if (!Prompt) { return; }
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI) { return; }
	const int32 Mask = (GI->bReadEmail ? 1 : 0) | (GI->bGotCoffee ? 2 : 0) | (GI->bTrainingComplete ? 4 : 0);
	if (Mask == TasksShown) { return; }
	const int32 NewlyDone = TasksShown < 0 ? 0 : (Mask & ~TasksShown);
	TasksShown = Mask;
	auto Box = [](bool bDone) { return bDone ? TEXT("[x]") : TEXT("[  ]"); };
	FString Text = TEXT("TODAY  \u00B7  EMPLOYEE 4471\n");
	Text += FString::Printf(TEXT("%s  Check email\n"), Box(GI->bReadEmail));
	Text += FString::Printf(TEXT("%s  Get coffee\n"), Box(GI->bGotCoffee));
	Text += FString::Printf(TEXT("%s  Complete Compliance Training"), Box(GI->bTrainingComplete));
	if (Mask == 7) { Text += TEXT("\n\nNext: 10:00 meeting, Conference Room"); }
	Prompt->SetTasks(Text);
	if (Mask == 7 && NewlyDone)
	{
		ShowSubtitle(TEXT("PA"), TEXT("Thank you, 4471. Your morning is complete. Please proceed to your 10:00 in the Conference Room."), 6.f);
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Morning tasks complete"));
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
		Prompt->SetPrompt(IsHoldingMug() ? TEXT("F  Sip        E  Drop        Click  Throw") : TEXT("E  Drop        Click  Throw"));
		return;
	}
	FHitResult Hit;
	switch (Look(Hit))
	{
	case ETakeLookKind::Coworker: Prompt->SetPrompt(TEXT("E  Talk")); break;
	case ETakeLookKind::Terminal: Prompt->SetPrompt(TEXT("E  Sit and log in")); break;
	case ETakeLookKind::Openable:
	{
		const FSwing* S = Swings.Find(Hit.GetActor());
		Prompt->SetPrompt(IsLockedForMe(Hit.GetActor()) ? TEXT("E  Try the door") : ((S && S->bOpen) ? TEXT("E  Close") : TEXT("E  Open")));
		break;
	}
	case ETakeLookKind::Coffee:   Prompt->SetPrompt(TEXT("E  Pour a coffee")); break;
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
	UpdatePrompt();
	UpdateTasks();
	if (SubtitleLeft > 0.f)
	{
		SubtitleLeft -= DeltaTime;
		if (SubtitleLeft <= 0.f && Prompt) { Prompt->SetSubtitle(TEXT("")); }
	}
	if (!IsHolding()) { return; }
	FVector Loc; FRotator Rot;
	if (GetView(Loc, Rot))
	{
		if (SipLeft > 0.f && IsHoldingMug())   // bring the mug up to your face and tip it
		{
			const float A = FMath::Sin(PI * FMath::Clamp(1.f - SipLeft / 0.9f, 0.f, 1.f));
			const FVector Target = Loc + Rot.Vector() * FMath::Lerp(HoldDistance, 38.f, A) - FVector(0.f, 0.f, 8.f * A);
			Handle->SetTargetLocationAndRotation(Target, FRotator(HeldPitchRoll.Pitch, Rot.Yaw + HeldYawOffset, HeldPitchRoll.Roll + 35.f * A));
		}
		else
		{
			Handle->SetTargetLocationAndRotation(Loc + Rot.Vector() * HoldDistance, FRotator(HeldPitchRoll.Pitch, Rot.Yaw + HeldYawOffset, HeldPitchRoll.Roll));
		}
	}
}
