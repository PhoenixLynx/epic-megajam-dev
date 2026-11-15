// TAKE 4471 - Saturday batch. Part of UTakeGrabComponent, kept in its own file:
//   the 12:30 call     : E on the conference screen (GB_CallScreen) or a conference laptop (tag CallScreen)
//   the 1:00 meeting   : the same call again, word for word, then "Item four. Retake."
//   calendar reminders : your desk phone rings at 12:20 / 12:55 if you haven't gone; missed calls ring back
//   coffee machine     : pours into the mug you're holding, or gives you a fresh mug; never runs out
//   vending machine    : E = a snack (always B4)
//   food comes back    : eaten food reappears where it was, when you aren't looking
//   paper toss         : anything you throw that lands in a bin counts; streak of 3 = an email from Morale
//   Key Lock 2         : the "Employee 4471" login panel in the supply closet (signature hidden: use your notes)
//   new mail           : a chime and a line when something new arrives (unread count)
#include "TakeGrabComponent.h"
#include "TakeCoworker.h"
#include "DirectorsCut_58.h"
#include "TakeGameInstance.h"
#include "TakeFearDirector.h"
#include "TakeKeyLockLibrary.h"
#include "TakeWidgets.h"
#include "TakePauseMenu.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TextRenderActor.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "CollisionQueryParams.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TakeStalker.h"
#include "TakeTypes.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace
{
	// The water-cooler gossiper: a coworker who stands by the kitchen cooler all day (spawned at play, tag "Gossip")
	ATakeCoworker* FindGossiper(UWorld* World)
	{
		for (TActorIterator<ATakeCoworker> It(World); It; ++It) { if (It->ActorHasTag(TEXT("Gossip"))) { return *It; } }
		return nullptr;
	}

	void SpawnGossiper(UWorld* World)
	{
		if (!World || FindGossiper(World)) { return; }
		FVector Where(3640.f, 1150.f, 92.f);   // beside the kitchen cooler (3730, 1250)
		const FTransform T(FRotator(0.f, 180.f, 0.f), Where);
		ATakeCoworker* G = World->SpawnActorDeferred<ATakeCoworker>(ATakeCoworker::StaticClass(), T, nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		if (!G) { return; }
		G->NPCRole = TEXT("Coworker");
		G->bWanders = false;
		G->bStoryNPC = false;
		G->Tags = { FName(TEXT("NPC")), FName(TEXT("Gossip")) };
		if (UStaticMesh* Body = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Props/Meshes/SM_Mannequin.SM_Mannequin"))) { G->BodyMesh = Body; }
		G->FinishSpawning(T);
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Water-cooler gossiper by the kitchen cooler"));
	}

	bool IsFoodMesh(const FString& N)
	{
		return N.Contains(TEXT("Snack")) || N.Contains(TEXT("Fruit")) || N.Contains(TEXT("SodaCan")) || N.Contains(TEXT("WaterBottle"))
			|| N.Contains(TEXT("LunchBox")) || N.Contains(TEXT("MilkCarton")) || N.Contains(TEXT("Sandwich")) || N.Contains(TEXT("Pizza"))
			|| N.Contains(TEXT("Salad")) || N.Contains(TEXT("Donut"));
	}
}

// ------------------------------------------------------------------ the call and the meeting
bool UTakeGrabComponent::CanJoinCall() const
{
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	return GI && GI->CurrentAct() >= 2 && CallLeft < 0.f && !(GI->bJoinedCall && GI->bAttendedMeeting);
}

void UTakeGrabComponent::JoinCall()
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI || !CanJoinCall()) { return; }
	bCallIsMeeting = GI->bJoinedCall;
	if (RingingPhone.IsValid() && ReminderRinging > 0) { StopRinging(); ReminderRinging = 0; }   // you're here: no reminder needed
	if (bCallIsMeeting) { bMeetingReminded = true; } else { bCallReminded = true; }
	GI->AddEnergy(-10.f);   // meetings are draining (that's the joke; it's also the mechanic)
	GI->WorkdayMinutes = FMath::Max(GI->WorkdayMinutes, bCallIsMeeting ? 13.f * 60.f : 12.f * 60.f + 30.f);
	PlaySfx2D(TEXT("sfx_badge_ok"), 0.5f);
	StopMusic(3.f);   // the call: the music steps out of the room
	PlaySfx2D(TEXT("sfx_phone_static"), 0.25f);   // a laptop speaker, someone's open mic

	// The live captions don't transcribe words: they show Key 2's signature (a clue for the supply closet panel)
	const FTakeKeyLock Lock2 = UTakeKeyLockLibrary::GenerateKeyLockForRun(this, 2, 0);
	const FString Captions = UTakeGameInstance::GlyphString(Lock2.Target, Lock2.Bits);
	// Beat 2.3 / 2.4: the meeting is the call again, word for word (that's the point)
	TArray<FString> Lines = {
		TEXT("(Joining \"Sync (12:30)\"...)"),
		TEXT("(Eleven tiles. Your coworkers, all smiling. One of them is frozen mid-laugh.)"),
		TEXT("(The organizer's tile is black. Just a letter: C. Camera off.)"),
		TEXT("C:  Welcome, everyone. Good energy today."),
		TEXT("C:  Item two. Quarterly energy targets. We're at ninety-nine percent. Someone is holding back."),
		FString::Printf(TEXT("(Live captions:  %s   It isn't transcribing words.)"), *Captions),
		TEXT("C:  Item three. Morale. Morale is mandatory."),
	};
	if (!bCallIsMeeting)
	{
		Lines.Add(TEXT("C:  Item four... we'll get to item four at one o'clock. Same room. Don't be late."));
		Lines.Add(TEXT("(The call ends. Behind you, the camera light on the wall stays on.)"));
	}
	else
	{
		Lines[0] = TEXT("(Joining \"Meeting (1:00)\"...)");
		Lines.Insert(TEXT("...This is the same call. Word for word."), 4);
		Lines.Add(TEXT("C:  Item four. Retake."));
		Lines.Add(TEXT("(Every tile turns to look into its camera. At you.)"));
		Lines.Add(TEXT("C:  Employee 4471. From the top, please."));
	}
	CallLeft = 0.5f;
	for (const FString& L : Lines) { CallLeft += FMath::Max(2.5f, L.Len() / 14.f); }   // same timing as the subtitle queue
	ShowSubtitleSequence(TEXT(""), Lines);
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Joined the %s"), bCallIsMeeting ? TEXT("1:00 meeting") : TEXT("12:30 call"));
}

void UTakeGrabComponent::TickCall(float DeltaTime)
{
	if (CallLeft < 0.f) { return; }
	CallLeft -= DeltaTime;
	if (CallLeft > 0.f) { return; }
	CallLeft = -1.f;
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI) { return; }
	NoteGlyphs(2, TEXT("call captions"));
	if (!bCallIsMeeting)
	{
		GI->bJoinedCall = true;
	}
	else
	{
		GI->bAttendedMeeting = true;
		if (ATakeFearDirector* FearDir = FindFearDirector())
		{
			FearDir->FlickerBurst(1.5f);
			FearDir->StartCreep(FMath::Max(FearDir->Fear, 0.3f), 4.f);
		}
		PlaySfx2D(TEXT("sfx_clapper_snap"), 0.4f);
	}
	GI->SetObjective(GI->ObjectiveForBeat());
}

bool UTakeGrabComponent::TickReminders()
{
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI || GI->CurrentAct() < 2 || GI->bClocksFrozen || !MyPhone.IsValid() || RingingPhone.IsValid() || CallLeft > 0.f) { return false; }
	if (NextRingIn > 0.f && bCallBackMine) { return false; }   // a missed reminder waits for its call-back time
	int32 Which = 0;
	if (!GI->bJoinedCall && !bCallReminded && GI->WorkdayMinutes >= 12.f * 60.f + 20.f) { Which = 1; }
	else if (GI->bJoinedCall && !GI->bAttendedMeeting && !bMeetingReminded && GI->WorkdayMinutes >= 12.f * 60.f + 55.f) { Which = 2; }
	if (Which == 0) { return false; }
	bCallBackMine = false;
	RingPhone(MyPhone.Get());
	ReminderRinging = Which;
	RingLeft = 15.f;
	const APawn* Me = Cast<APawn>(GetOwner());
	if (Me && FVector::Dist(Me->GetActorLocation(), MyPhone->GetActorLocation()) > 1200.f)
	{
		QueueSubtitle(TEXT(""), TEXT("(Back at your desk, your phone is ringing.)"));
	}
	return true;
}

// ------------------------------------------------------------------ coffee, vending, food, paper toss
AActor* UTakeGrabComponent::SpawnProp(const TCHAR* MeshName, const FVector& Where, const FRotator& Rot)
{
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *FString::Printf(TEXT("/Game/Props/Meshes/%s.%s"), MeshName, MeshName));
	if (!Mesh) { return nullptr; }
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AStaticMeshActor* A = GetWorld()->SpawnActor<AStaticMeshActor>(Where, Rot, Params);
	if (!A) { return nullptr; }
	UStaticMeshComponent* SM = A->GetStaticMeshComponent();
	SM->SetMobility(EComponentMobility::Movable);
	SM->SetStaticMesh(Mesh);
	SM->SetCollisionProfileName(TEXT("PhysicsActor"));
	SM->SetSimulatePhysics(true);
	SM->CanCharacterStepUpOn = ECB_No;
	SM->SetNotifyRigidBodyCollision(true);
	SM->OnComponentHit.AddUniqueDynamic(this, &UTakeGrabComponent::OnPropHit);
	A->Tags.Add(TEXT("Throwable"));
	Spawned.Add(A);
	if (Spawned.Num() > 10)   // recycle the oldest one nobody is holding
	{
		for (int32 I = 0; I < Spawned.Num() - 1; ++I)
		{
			AActor* Old = Spawned[I].Get();
			const bool bHeld = Old && IsHolding() && Handle->GetGrabbedComponent() && Handle->GetGrabbedComponent()->GetOwner() == Old;
			if (bHeld) { continue; }
			if (Old) { Old->Destroy(); }
			Spawned.RemoveAt(I);
			break;
		}
	}
	return A;
}

void UTakeGrabComponent::PourCoffee()
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	FHitResult Hit;
	AActor* Machine = (Look(Hit) == ETakeLookKind::Coffee) ? Hit.GetActor() : nullptr;
	const FVector At = Machine ? Machine->GetActorLocation() : GetOwner()->GetActorLocation();
	PlaySfx(TEXT("sfx_coffee_pour"), At);
	const float Fear = FindFearDirector() ? FindFearDirector()->Fear : 0.f;
	const int32 Cup = GI ? GI->CoffeesPoured : 0;
	FString Line;
	const bool bFrozen = GI && GI->bClocksFrozen;
	UPrimitiveComponent* Held = IsHolding() ? Handle->GetGrabbedComponent() : nullptr;
	if (Fear >= 0.5f)
	{
		Line = TEXT("It pours black. Then blacker. It doesn't stop until you look away.");
	}
	else if (bFrozen && Cup % 2 == 0)
	{
		Line = TEXT("The coffee comes out and stops in mid-air. It's 2:14 for the coffee too.");
	}
	else if (Cup == 9)
	{
		Line = TEXT("The screen: CUP 10. YOU HAVE EXCEEDED TODAY'S JOY ALLOWANCE. ...JOY GRANTED ANYWAY.");
	}
	else if (Cup == 14)
	{
		Line = TEXT("The screen: HELLO 4470. ...SORRY. HELLO 4471. THE USUAL?");
	}
	else if (Held && !IsHoldingMug())
	{
		Line = TEXT("You hold it under the spout. The machine fills it anyway. You don't ask how.");
	}
	else if (IsHoldingMug())
	{
		if (AActor* MugActor = Handle->GetGrabbedComponent()->GetOwner()) { MugActor->Tags.Remove(TEXT("Spilled")); }   // full again
		static const TCHAR* Refills[] = {
			TEXT("You fill your mug. It's exactly the right temperature. It always is."),
			TEXT("You press DECAF. It pours coffee. You press TEA. It pours coffee."),
			TEXT("The machine hums the HALCYON jingle while it pours."),
			TEXT("A refill. The screen says: CUP 1 OF 1."),
			TEXT("The machine waits until your mug is exactly full. It knows your mug."),
		};
		Line = Refills[Cup % int32(UE_ARRAY_COUNT(Refills))];
	}
	else
	{
		// No mug in your hands: a fresh one slides out, already full (the kitchen never runs out)
		FVector Out = At + FVector(0.f, 0.f, 6.f);
		if (Machine)
		{
			FVector ToMe = GetOwner()->GetActorLocation() - At;
			ToMe.Z = 0.f;
			Out += ToMe.GetSafeNormal() * 32.f;
		}
		SpawnProp(TEXT("SM_Mug"), Out, FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f));
		Line = Cup == 0 ? TEXT("A fresh mug slides out, already full. It has your name on it.")
			: TEXT("Another mug slides out. Your name again. How many of you drink here?");
	}
	ShowSubtitle(TEXT(""), Line, 4.f);
	if (GI)
	{
		if (!GI->bGotCoffee && !bMusicCoffeeDone) { bMusicCoffeeDone = true; PlayMusicCue(TEXT("first coffee")); }
		GI->bGotCoffee = true;
		++GI->CoffeesPoured;
	}
}

void UTakeGrabComponent::Vend(AActor* Machine)
{
	if (!Machine) { return; }
	FVector ToMe = GetOwner()->GetActorLocation() - Machine->GetActorLocation();
	ToMe.Z = 0.f;
	const FVector Out = Machine->GetActorLocation() + ToMe.GetSafeNormal() * 55.f + FVector(0.f, 0.f, 45.f);
	PlaySfx(TEXT("sfx_impact_heavy"), Machine->GetActorLocation() + FVector(0.f, 0.f, 50.f), 0.5f);
	if (AActor* Snack = SpawnProp(TEXT("SM_SnackBag"), Out, FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f)))
	{
		if (UPrimitiveComponent* P = Cast<UPrimitiveComponent>(Snack->GetRootComponent()))
		{
			P->AddImpulse(ToMe.GetSafeNormal() * 60.f, NAME_None, true);
		}
	}
	static const TCHAR* Lines[] = {
		TEXT("You didn't press anything. B4 drops anyway."),
		TEXT("B4 again. It's always B4."),
		TEXT("The little screen says: THANK YOU, 4471. You never told it your name."),
		TEXT("Exact change: none. It didn't ask for any."),
		TEXT("Something is stuck behind the glass. It's a badge. 4470."),
		TEXT("You try to type 4470. The keypad only has the numbers 4, 7 and 1."),
		TEXT("The screen: YOUR USUAL. You've never been here before. ...Have you?"),
	};
	const UTakeGameInstance* VGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	const float VFear = FindFearDirector() ? FindFearDirector()->Fear : 0.f;
	if (VFear >= 0.5f) { ShowSubtitle(TEXT(""), TEXT("The machine is empty. Something drops anyway. It's still B4."), 3.5f); }
	else if (VGI && VGI->bClocksFrozen) { ShowSubtitle(TEXT(""), TEXT("The screen: OUT OF TIME. It drops a snack anyway. Best before 2:14."), 3.5f); }
	else { ShowSubtitle(TEXT(""), Lines[Vended % int32(UE_ARRAY_COUNT(Lines))], 3.5f); }
	++Vended;
}

void UTakeGrabComponent::CollectFunProps()
{
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
	{
		UStaticMeshComponent* SM = It->GetStaticMeshComponent();
		if (!SM || !SM->GetStaticMesh()) { continue; }
		const FString N = SM->GetStaticMesh()->GetName();
		if (It->ActorHasTag(TEXT("Act2Reveal")) && !It->ActorHasTag(TEXT("NPC")))   // (NPC placeholders stay hidden: the real one is a coworker)
		{
			FReveal R;
			R.Actor = *It;
			R.bPhysics = It->ActorHasTag(TEXT("Throwable"));
			Act2Things.Add(R);
			if (!GI || GI->CurrentAct() < 2)   // not there yet: invisible, nothing to bump into, not falling
			{
				It->SetActorHiddenInGame(true);
				It->SetActorEnableCollision(false);
				SM->SetSimulatePhysics(false);
			}
		}
		if (It->ActorHasTag(TEXT("FearReveal"))) { FearReveals.Add(*It); }
		if (It->ActorHasTag(TEXT("SecurityCam"))) { Cams.Add(*It); }
		if (N.Contains(TEXT("TrashBin"))) { Bins.Add(*It); }
		else if (IsFoodMesh(N) && It->ActorHasTag(TEXT("Throwable")))
		{
			FFood F;
			F.Actor = *It;
			F.Home = It->GetActorTransform();
			Foods.Add(F);
		}
	}
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] %d bin(s) for the paper toss, %d food item(s) that come back"), Bins.Num(), Foods.Num());
	MakeElevatorClock();
	SpawnGossiper(GetWorld());
	// 4470's polaroid was hanging off the edge of the moving box: lay it flat in the middle, on whatever is inside
	AStaticMeshActor* Polaroid = nullptr;
	AStaticMeshActor* MovingBox = nullptr;
	for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
	{
		UStaticMeshComponent* SM = It->GetStaticMeshComponent();
		if (!SM || !SM->GetStaticMesh()) { continue; }
		if (SM->GetStaticMesh()->GetName().Contains(TEXT("MovingBox"))) { MovingBox = *It; }
		const UMaterialInterface* M = SM->GetMaterial(0);
		if (M && M->GetName().Contains(TEXT("Polaroid"))) { Polaroid = *It; }
	}
	if (Polaroid && MovingBox)
	{
		FVector Origin, Extent;
		MovingBox->GetActorBounds(false, Origin, Extent);
		FHitResult In;
		FCollisionQueryParams PQ(SCENE_QUERY_STAT(TakePolaroid), true, Polaroid);   // complex: finds the inside of an open box
		const FVector Top(Origin.X, Origin.Y, Origin.Z + Extent.Z + 30.f);
		const FVector Rest = GetWorld()->LineTraceSingleByChannel(In, Top, Origin - FVector(0.f, 0.f, Extent.Z), ECC_Visibility, PQ)
			? In.ImpactPoint : FVector(Origin.X, Origin.Y, Origin.Z + Extent.Z);
		Polaroid->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
		Polaroid->SetActorLocation(Rest + FVector(0.f, 0.f, 0.4f));
		Polaroid->SetActorRotation(FRotator(0.f, MovingBox->GetActorRotation().Yaw + 12.f, 0.f));
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Polaroid placed in the moving box at %s"), *Rest.ToCompactString());
	}
}

bool UTakeGrabComponent::SpillCoffee(UStaticMeshComponent* Mug, const FHitResult& Hit, AActor* OtherActor, float Speed)
{
	// Only a mug, only fast enough, only once until it's refilled at the machine
	if (!Mug || !Mug->GetStaticMesh() || !Mug->GetStaticMesh()->GetName().Contains(TEXT("Mug"))) { return false; }
	AActor* MugActor = Mug->GetOwner();
	if (!MugActor || MugActor->ActorHasTag(TEXT("Spilled")) || Speed < 380.f) { return false; }
	if (IsHolding() && Handle->GetGrabbedComponent() == Mug) { return false; }   // knocking it against the desk while you carry it
	MugActor->Tags.Add(TEXT("Spilled"));
	++CoffeeSplashes;
	UWorld* World = GetWorld();
	const FVector At = Hit.ImpactPoint;
	PlaySfx(TEXT("sfx_coffee_pour"), At, 0.9f);

	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UStaticMesh* Disc = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UMaterialInstanceDynamic* Brown = Base ? UMaterialInstanceDynamic::Create(Base, this) : nullptr;
	if (Brown) { Brown->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.09f, 0.035f, 0.01f)); }
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Droplets: a burst of little brown spheres, thrown up and out from the impact, gone in a moment
	const FVector Out = Hit.ImpactNormal.IsNearlyZero() ? FVector::UpVector : Hit.ImpactNormal;
	const FVector Along = Mug->GetComponentVelocity().GetSafeNormal();
	for (int32 I = 0; Sphere && I < 14; ++I)
	{
		AStaticMeshActor* D = World->SpawnActor<AStaticMeshActor>(At + Out * 6.f, FRotator::ZeroRotator, Params);
		if (!D) { continue; }
		UStaticMeshComponent* SM = D->GetStaticMeshComponent();
		SM->SetMobility(EComponentMobility::Movable);
		SM->SetStaticMesh(Sphere);
		SM->SetWorldScale3D(FVector(FMath::FRandRange(0.025f, 0.06f)));   // 2.5 to 6 cm
		if (Brown) { SM->SetMaterial(0, Brown); }
		SM->SetCastShadow(false);
		SM->SetCollisionProfileName(TEXT("PhysicsActor"));
		SM->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		SM->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
		SM->SetSimulatePhysics(true);
		SM->SetMassOverrideInKg(NAME_None, 0.02f);
		const FVector Dir = (Out * 0.9f + FVector::UpVector * 0.6f - Along * 0.3f + FMath::VRand() * 0.7f).GetSafeNormal();
		SM->SetPhysicsLinearVelocity(Dir * FMath::FRandRange(180.f, 420.f));
		D->SetLifeSpan(FMath::FRandRange(0.9f, 1.6f));
	}
	// The puddle: on the floor under the impact, stays for a while
	FHitResult Floor;
	FCollisionQueryParams Q(SCENE_QUERY_STAT(TakeCoffeePuddle), false, MugActor);
	if (OtherActor) { Q.AddIgnoredActor(OtherActor); }
	if (Disc && World->LineTraceSingleByChannel(Floor, At + FVector(0.f, 0.f, 20.f), At - FVector(0.f, 0.f, 400.f), ECC_Visibility, Q))
	{
		if (AStaticMeshActor* P = World->SpawnActor<AStaticMeshActor>(Floor.ImpactPoint + FVector(0.f, 0.f, 0.6f),
			FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f), Params))
		{
			UStaticMeshComponent* SM = P->GetStaticMeshComponent();
			SM->SetMobility(EComponentMobility::Movable);
			SM->SetStaticMesh(Disc);
			const float R = FMath::FRandRange(0.45f, 0.7f);
			SM->SetWorldScale3D(FVector(R, R * FMath::FRandRange(0.6f, 0.9f), 0.004f));   // 45-70 cm across, paper thin
			if (Brown) { SM->SetMaterial(0, Brown); }
			SM->SetCastShadow(false);
			SM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			P->Tags.Add(TEXT("CoffeePuddle"));
			P->SetLifeSpan(25.f);
		}
	}

	// Who got it
	if (ATakeStalker* Stalker = Cast<ATakeStalker>(OtherActor))
	{
		Stalker->Stun(4.f);   // longer than any stapler
		QueueSubtitle(TEXT(""), TEXT("It stops. It touches its face. Nobody has ever thrown coffee at it before."));
		return true;
	}
	if (ATakeCoworker* Victim = Cast<ATakeCoworker>(OtherActor))
	{
		if (Victim->State == ETakeCoworkerState::Watching) { return true; }   // Fear: they don't even flinch
		++HotBeverageIncidents;
		if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { ++GI->HotBeverageIncidents; }   // HR mail
		PlaySfx2D(TEXT("sfx_pa_chime"), 0.6f);
		static const TCHAR* PA[] = {
			TEXT("A reminder: hot beverages are to be carried, not thrown."),
			TEXT("Hot beverage incident number two has been logged. Facilities is bringing lids."),
			TEXT("All staff: please keep your mugs below shoulder height until further notice."),
			TEXT("Employee {EMP}, put the mug down."),
		};
		const int32 N = FMath::Min(HotBeverageIncidents, int32(UE_ARRAY_COUNT(PA))) - 1;
		QueueSubtitle(TEXT("PA"), TakeText::Fill(this, PA[N]));
		if (HotBeverageIncidents == 4) { QueueSubtitle(TEXT(""), TEXT("They know your number. Of course they know your number.")); }
		return true;
	}
	if (CoffeeSplashes == 1) { QueueSubtitle(TEXT(""), TEXT("Coffee everywhere. Somewhere, a custodian sighs without knowing why.")); }
	return true;
}

void UTakeGrabComponent::MakeElevatorClock()
{
	if (ElevatorClock.IsValid()) { return; }
	// On the lobby side of the elevator wall, just right of the doorway, up high (elevator room x 1800-2400, its south wall
	// at y 500; the doorway x 1950-2250 is full height, so there's no lintel to hang it on)
	const FVector At(2325.f, 514.f, 250.f);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		if (AStaticMeshActor* Case = GetWorld()->SpawnActor<AStaticMeshActor>(At - FVector(0.f, 3.f, 0.f), FRotator::ZeroRotator, Params))
		{
			UStaticMeshComponent* SM = Case->GetStaticMeshComponent();
			SM->SetMobility(EComponentMobility::Movable);
			SM->SetStaticMesh(Cube);
			SM->SetWorldScale3D(FVector(0.62f, 0.04f, 0.22f));   // 62 x 4 x 22 cm
			SM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			if (UMaterialInterface* Steel = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Props/Materials/M_BrushedSteel.M_BrushedSteel"))) { SM->SetMaterial(0, Steel); }
		}
	}
	// The doors: the doorway is x 1950-2250 in the south wall (y 500); each panel slides into the car, behind the wall
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		UMaterialInterface* Steel = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Props/Materials/M_BrushedSteel.M_BrushedSteel"));
		for (int32 Side = 0; Side < 2; ++Side)
		{
			AStaticMeshActor* D = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(Side == 0 ? 2025.f : 2175.f, 488.f, 150.f), FRotator::ZeroRotator, Params);
			if (!D) { continue; }
			UStaticMeshComponent* SM = D->GetStaticMeshComponent();
			SM->SetMobility(EComponentMobility::Movable);
			SM->SetStaticMesh(Cube);
			SM->SetWorldScale3D(FVector(1.5f, 0.05f, 3.f));   // 150 x 5 x 300 cm
			if (Steel) { SM->SetMaterial(0, Steel); }
			SM->SetCollisionProfileName(TEXT("BlockAll"));
			SM->CanCharacterStepUpOn = ECB_No;
			D->Tags.Add(TEXT("ElevatorDoor"));
			(Side == 0 ? ElevDoorL : ElevDoorR) = D;
		}
		// Inside the car: the steel walls stay, the floor and ceiling don't (playtest: "too chrome")
		for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
		{
			UStaticMeshComponent* SM = It->GetStaticMeshComponent();
			const FVector L = It->GetActorLocation();
			if (SM && SM->GetStaticMesh() == Cube && FMath::Abs(L.X - 2100.f) < 20.f && FMath::Abs(L.Y - 250.f) < 20.f && L.Z < 5.f)
			{
				if (UMaterialInterface* Floor = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Fear/MI_Greybox_Floors.MI_Greybox_Floors"))) { SM->SetMaterial(0, Floor); }
			}
		}
		auto Box = [&](const FVector& BoxAt, const FVector& SizeCm, UMaterialInterface* Mat)
		{
			if (AStaticMeshActor* B = GetWorld()->SpawnActor<AStaticMeshActor>(BoxAt, FRotator::ZeroRotator, Params))
			{
				UStaticMeshComponent* SM = B->GetStaticMeshComponent();
				SM->SetMobility(EComponentMobility::Movable);
				SM->SetStaticMesh(Cube);
				SM->SetWorldScale3D(SizeCm / 100.f);
				SM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				if (Mat) { SM->SetMaterial(0, Mat); }
			}
		};
		UMaterialInterface* CeilingMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Fear/M_CeilingPanel.M_CeilingPanel"));
		Box(FVector(2100.f, 256.f, 296.f), FVector(572.f, 484.f, 2.f), CeilingMat);              // ceiling panel
		Box(FVector(1818.f, 256.f, 95.f), FVector(4.f, 420.f, 5.f), Steel);                      // west handrail
		Box(FVector(2382.f, 256.f, 95.f), FVector(4.f, 420.f, 5.f), Steel);                      // east handrail
		if (APointLight* Lamp = GetWorld()->SpawnActor<APointLight>(FVector(2100.f, 250.f, 270.f), FRotator::ZeroRotator, Params))
		{
			Lamp->PointLightComponent->SetMobility(EComponentMobility::Movable);
			Lamp->PointLightComponent->SetIntensity(2500.f);
			Lamp->PointLightComponent->SetAttenuationRadius(450.f);
			Lamp->PointLightComponent->SetLightColor(FLinearColor(1.f, 0.88f, 0.72f));   // warm, a little old
			Lamp->PointLightComponent->SetCastShadows(false);
		}
		ElevDoorOpen = 1.f;
		bElevDoorWantOpen = true;   // they start open (you start in the car, the cold open looks out through them)
	}
	if (ATextRenderActor* T = GetWorld()->SpawnActor<ATextRenderActor>(At, FRotator(0.f, 90.f, 0.f), Params))   // text faces +Y (the lobby)
	{
		UTextRenderComponent* TR = T->GetTextRender();
		TR->SetHorizontalAlignment(EHTA_Center);
		TR->SetVerticalAlignment(EVRTA_TextCenter);
		TR->SetWorldSize(17.f);
		TR->SetTextRenderColor(FColor(255, 40, 30));
		TR->SetText(FText::FromString(TEXT("9:00")));
		ElevatorClock = TR;
	}
}

void UTakeGrabComponent::RevealAct2()
{
	bAct2Revealed = true;
	for (const FReveal& R : Act2Things)
	{
		AActor* A = R.Actor.Get();
		if (!A) { continue; }
		A->SetActorHiddenInGame(false);
		A->SetActorEnableCollision(true);
		if (UStaticMeshComponent* SM = Cast<UStaticMeshComponent>(A->GetRootComponent()))
		{
			if (R.bPhysics)   // lunch you can pick up: a fresh physics body, then simulate
			{
				SM->SetMobility(EComponentMobility::Movable);
				SM->SetCollisionProfileName(TEXT("PhysicsActor"));
				SM->RecreatePhysicsState();
				SM->SetSimulatePhysics(true);
				SM->SetNotifyRigidBodyCollision(true);
				SM->OnComponentHit.AddUniqueDynamic(this, &UTakeGrabComponent::OnPropHit);
			}
		}
	}
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Lunch is out: revealed %d Act 2 thing(s)"), Act2Things.Num());
}

void UTakeGrabComponent::FoodEaten(AActor* Food)
{
	// Lunch (Act 2): eat every dish on the island and the chef has something to say about it
	const UTakeGameInstance* FoodGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	const UStaticMeshComponent* FoodSM = Food ? Cast<UStaticMeshComponent>(Food->GetRootComponent()) : nullptr;
	const FString FoodName = FoodSM && FoodSM->GetStaticMesh() ? FoodSM->GetStaticMesh()->GetName() : FString();
	if (FoodGI && FoodGI->CurrentAct() == 2 && !bChefRemarked && Food && Food->ActorHasTag(TEXT("Act2Reveal"))
	    && (FoodName.Contains(TEXT("Pizza")) || FoodName.Contains(TEXT("Sandwich")) || FoodName.Contains(TEXT("Salad")) || FoodName.Contains(TEXT("Donut"))))
	{
		LunchEaten.Add(Food);
		if (LunchEaten.Num() >= 6)
		{
			bChefRemarked = true;
			for (TActorIterator<ATakeCoworker> It(GetWorld()); It; ++It)
			{
				if (!It->ActorHasTag(TEXT("Chef")) || It->IsHidden()) { continue; }
				It->SayLine(TEXT("All of it? That was for the whole floor, {EMP}. ...No. No, it's fine. I'll make more. I always make more."));
				break;
			}
		}
	}
	for (FFood& F : Foods)
	{
		if (F.Actor.Get() == Food) { F.Back = FMath::FRandRange(40.f, 70.f); return; }
	}
	// A vended snack: gone for good (the machine has more)
	Spawned.Remove(Food);
	if (Food) { Food->SetLifeSpan(0.5f); }
}

void UTakeGrabComponent::TickElevatorDoors(float DeltaTime)
{
	AStaticMeshActor* L = ElevDoorL.Get();
	AStaticMeshActor* R = ElevDoorR.Get();
	const APawn* Me = Cast<APawn>(GetOwner());
	if (!L || !R || !Me) { return; }
	// Open when you're near the doorway (either side), or anyone is standing in it; close when you've gone
	const FVector Doorway(2100.f, 500.f, 0.f);
	const FVector P = Me->GetActorLocation();
	const bool bInCar = P.X > 1800.f && P.X < 2400.f && P.Y > 0.f && P.Y < 500.f && P.Z > -100.f && P.Z < 400.f;
	bool bWant = bInCar || FVector::Dist2D(P, Doorway) < 260.f;   // in the car they stay open (you never get shut in)
	const bool bPartyShut = EndT >= 0.f && EndStep != 11 && !bPenthouseBuilt;   // the wrap party: shut until C is lit
	if (bPartyShut && !bInCar) { bWant = false; }
	for (TActorIterator<ACharacter> It(GetWorld()); It && !bWant && !bPartyShut; ++It)
	{
		if (FVector::Dist2D(It->GetActorLocation(), Doorway) < 170.f) { bWant = true; }
	}
	if (bWant != bElevDoorWantOpen)
	{
		bElevDoorWantOpen = bWant;
		PlaySfx(bWant ? TEXT("sfx_badge_ok") : TEXT("sfx_door_close"), FVector(2100.f, 500.f, 150.f), 0.45f);   // ding / thunk
	}
	ElevDoorOpen = FMath::FInterpConstantTo(ElevDoorOpen, bWant ? 1.f : 0.f, DeltaTime, bWant ? 1.1f : 0.8f);
	const float Ease = ElevDoorOpen * ElevDoorOpen * (3.f - 2.f * ElevDoorOpen);   // smoothstep: starts and stops gently
	const float DoorSlide = 138.f * Ease;
	L->SetActorLocation(FVector(2025.f - DoorSlide, 488.f, 150.f));
	R->SetActorLocation(FVector(2175.f + DoorSlide, 488.f, 150.f));
}

void UTakeGrabComponent::TickFun(float DeltaTime)
{
	TickElevatorDoors(DeltaTime);
	FVector Eye; FRotator View;
	const bool bView = GetView(Eye, View);
	// The elevator clock follows the game clock (h:mm, 12-hour, like an office clock)
	if (UTextRenderComponent* Clock = ElevatorClock.Get())
	{
		if (const UTakeGameInstance* CGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			const int32 Mins = FMath::FloorToInt32(CGI->WorkdayMinutes);
			if (Mins != ElevatorClockShown)
			{
				ElevatorClockShown = Mins;
				const int32 H = ((Mins / 60) % 12 == 0) ? 12 : (Mins / 60) % 12;
				Clock->SetText(FText::FromString(FString::Printf(TEXT("%d:%02d"), H, Mins % 60)));
			}
		}
	}

	// Food comes back to where it was, when you aren't looking at the spot (not after lunch is put away in Act 3)
	for (FFood& F : Foods)
	{
		if (F.Back < 0.f || bLunchPutAway) { continue; }
		F.Back -= DeltaTime;
		AActor* A = F.Actor.Get();
		if (F.Back > 0.f || !A) { continue; }
		const FVector To = F.Home.GetLocation() - Eye;
		if (bView && To.Size() < 900.f && FVector::DotProduct(View.Vector(), To.GetSafeNormal()) > 0.35f) { F.Back = 3.f; continue; }
		A->SetActorHiddenInGame(false);
		A->SetActorEnableCollision(true);
		A->SetActorTransform(F.Home, false, nullptr, ETeleportType::ResetPhysics);
		if (UPrimitiveComponent* P = Cast<UPrimitiveComponent>(A->GetRootComponent())) { P->SetSimulatePhysics(true); }
		F.Back = -1.f;
	}

	// Paper toss: anything you throw that comes to rest inside a bin
	UPrimitiveComponent* Toss = TossProp.Get();
	if (!Toss || TossLeft <= 0.f) { return; }
	TossLeft -= DeltaTime;
	if (Toss->GetComponentLocation().Z < TossFrom.Z - 500.f)   // went through the floor anyway: put it back where you threw it
	{
		Toss->SetPhysicsLinearVelocity(FVector::ZeroVector);
		Toss->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		Toss->SetWorldLocation(TossFrom, false, nullptr, ETeleportType::ResetPhysics);
		UE_LOG(LogTake, Log, TEXT("[TAKE 4471] %s fell through the floor: put back at %s"), *GetNameSafe(Toss->GetOwner()), *TossFrom.ToCompactString());
		TossProp = nullptr;
		TossLeft = 0.f;
		return;
	}
	const FVector P = Toss->GetComponentLocation();
	const bool bSettled = Toss->GetComponentVelocity().Size() < 60.f;
	for (const TWeakObjectPtr<AActor>& B : Bins)
	{
		const AActor* Bin = B.Get();
		if (!Bin || Bin == Toss->GetOwner()) { continue; }
		FVector Origin, Extent;
		Bin->GetActorBounds(true, Origin, Extent);
		const bool bInside = FVector::Dist2D(P, Origin) < FMath::Min(Extent.X, Extent.Y) * 0.85f
			&& P.Z > Origin.Z - Extent.Z - 2.f && P.Z < Origin.Z + Extent.Z * 0.9f;
		if (!bInside) { continue; }
		TossProp = nullptr;
		TossLeft = 0.f;
		UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
		++TossStreak;
		if (GI)
		{
			++GI->Baskets;
			GI->BestTossStreak = FMath::Max(GI->BestTossStreak, TossStreak);
		}
		PlaySfx2D(TEXT("sfx_badge_ok"), 0.45f);
		static const TCHAR* Cheers[] = { TEXT("Nothing but bin."), TEXT("Swish."), TEXT("And the crowd goes mild."), TEXT("From downtown."), TEXT("The bin accepts your offering.") };
		ShowSubtitle(TEXT(""), FString::Printf(TEXT("%s   (streak %d)"), Cheers[(TossStreak - 1) % int32(UE_ARRAY_COUNT(Cheers))], TossStreak), 2.5f);
		return;
	}
	// It stopped somewhere else near a bin: that's a miss, and the streak ends
	if (bSettled && TossLeft < 2.5f)
	{
		for (const TWeakObjectPtr<AActor>& B : Bins)
		{
			if (B.IsValid() && FVector::Dist2D(P, B->GetActorLocation()) < 160.f && TossStreak > 0)
			{
				ShowSubtitle(TEXT(""), FString::Printf(TEXT("Miss. (streak was %d)"), TossStreak), 2.f);
				TossStreak = 0;
				break;
			}
		}
		TossProp = nullptr;
		TossLeft = 0.f;
	}
}

// ------------------------------------------------------------------ Key Lock 2 panel
void UTakeGrabComponent::OpenLockPanel(int32 Key)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC || LockPanel || Inspecting) { return; }
	LockPanel = CreateWidget<UTakeLockPanelWidget>(PC, UTakeLockPanelWidget::StaticClass());
	if (!LockPanel) { return; }
	Drop();
	LockPanel->Setup(Key);
	LockPanel->SetIsFocusable(true);
	LockPanel->OnCloseRequested.AddDynamic(this, &UTakeGrabComponent::CloseLockPanel);
	LockPanel->AddToViewport(30);
	if (Prompt) { Prompt->SetVisibility(ESlateVisibility::Collapsed); }
	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(LockPanel->TakeWidget());
	PC->SetInputMode(Mode);
	PC->SetShowMouseCursor(true);
	PC->SetIgnoreLookInput(true);
	PC->SetIgnoreMoveInput(true);
	PlaySfx2D(TEXT("sfx_badge_ok"), 0.3f);
}

void UTakeGrabComponent::CloseLockPanel()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (LockPanel) { LockPanel->RemoveFromParent(); LockPanel = nullptr; }
	if (Prompt) { Prompt->SetVisibility(ESlateVisibility::HitTestInvisible); }
	if (PC)
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
		PC->SetIgnoreLookInput(false);
		PC->SetIgnoreMoveInput(false);
	}
}

void UTakeGrabComponent::NoteGlyphs(int32 Key, const FString& Source)
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GI) { return; }
	const FTakeKeyLock Lock = UTakeKeyLockLibrary::GenerateKeyLockForRun(this, Key, 0);
	if (GI->AddGlyphNote(Key, Source, Lock.Target, Lock.Bits))
	{
		QueueSubtitle(TEXT(""), TEXT("(You note down the squares. Pause > Tasks to see your notes.)"));
	}
}

// ------------------------------------------------------------------ new mail
void UTakeGrabComponent::TickMail(float DeltaTime)
{
	MailCheckIn -= DeltaTime;
	if (MailCheckIn > 0.f) { return; }
	MailCheckIn = 2.f;
	int32 Total = 0;
	const int32 Unread = UTakeTerminalWidget::CountUnread(this, &Total);
	if (MailSeen >= 0 && Total > MailSeen && !bSeated && EndT < 0.f)   // (nobody checks mail at the wrap party)
	{
		PlaySfx2D(TEXT("sfx_badge_ok"), 0.25f);
		QueueSubtitle(TEXT(""), FString::Printf(TEXT("(New email. %d unread on your terminal.)"), Unread));
	}
	MailSeen = Total;
}

// ------------------------------------------------------------------ restroom and other "Use=<Key>" things
static void GI_AddEnergy(const UObject* Ctx, float Amount)
{
	if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(Ctx))) { GI->AddEnergy(Amount); }
}

FString UTakeGrabComponent::UseKey(const AActor* Thing)
{
	if (!Thing) { return FString(); }
	for (const FName& Tag : Thing->Tags)
	{
		const FString T = Tag.ToString();
		if (T.StartsWith(TEXT("Use="))) { return T.Mid(4); }
	}
	return FString();
}

void UTakeGrabComponent::UseThing(AActor* Thing)
{
	const FString Key = UseKey(Thing);
	if (Key.IsEmpty()) { return; }
	const int32 N = UseCounts.FindOrAdd(Key)++;
	const FVector Where = Thing->GetActorLocation() + FVector(0.f, 0.f, 80.f);
	const float Fear = FindFearDirector() ? FindFearDirector()->Fear : 0.f;
	auto Pick = [N](std::initializer_list<const TCHAR*> L) { return FString(*(L.begin() + FMath::Min<int32>(N, int32(L.size()) - 1))); };
	if (Key == TEXT("Elevator") && EndStep == 11) { GoToPenthouse(); return; }   // YES: up to C
	if (Key == TEXT("DirectorsChair")) { if (EndStep == 12) { SitDirector(); } return; }
	if (Key == TEXT("Screen"))   // someone else's monitor: never what you'd expect
	{
		struct FPeek { const TCHAR* Mi; const TCHAR* Title; const TCHAR* Caption; };
		static const FPeek Peeks[] = {
			{ TEXT("MI_Screen_CCTV"), TEXT("SECURITY FEED 07"), TEXT("A camera in the ceiling. It's pointed at this desk. At you, looking at this screen.") },
			{ TEXT("MI_Screen_Saver"), TEXT("404"), TEXT("EMPLOYEE NOT FOUND.\nThe request for this person could not be completed. Please try again tomorrow.") },
			{ TEXT("MI_Screen_Fear"), TEXT(":("), TEXT("Your workday ran into a problem and needs to restart.\nWe're just collecting some error info, and then we'll restart for you. (0% complete)") },
			{ TEXT("MI_Screen_Glyphs"), TEXT("UNTITLED.DOC"), TEXT("Rows of little squares. Some filled, some empty. The cursor is still blinking at the end.") },
			{ TEXT("MI_Screen_Code"), TEXT("SCENE_REPLAY.LOG"), TEXT("take 4469 ... ok\ntake 4470 ... ok\ntake 4471 ... RUNNING") },
			{ TEXT("MI_Screen_Promo"), TEXT("WE'RE HIRING"), TEXT("Join the HALCYON family. Same desk. Same mug. Starting tomorrow.") },
		};
		const FPeek& Pk = Peeks[N % int32(UE_ARRAY_COUNT(Peeks))];
		UTexture2D* PeekTex = nullptr;
		if (UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, *FString::Printf(TEXT("/Game/Props/Materials/%s.%s"), Pk.Mi, Pk.Mi)))
		{
			UTexture* T = nullptr;
			if (M->GetTextureParameterValue(FHashedMaterialParameterInfo(FName(TEXT("Tex"))), T)) { PeekTex = Cast<UTexture2D>(T); }
		}
		ShowScreenPeek(PeekTex, Pk.Title, TakeText::Fill(this, Pk.Caption));
		return;
	}
	if (Key == TEXT("Cake"))
	{
		PlaySfx(TEXT("sfx_eat_crunch"), Where, 0.7f);
		if (UTakeGameInstance* CakeGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { CakeGI->AddEnergy(25.f); }
		static const TCHAR* Slices[] = {
			TEXT("You cut a slice. Mint frosting. It tastes like the first day of something."),
			TEXT("Another slice. ...It's the same slice. The cake isn't any smaller."),
			TEXT("You've had three slices of a cake that hasn't been cut."),
		};
		ShowSubtitle(TEXT(""), Slices[FMath::Min(N, int32(UE_ARRAY_COUNT(Slices)) - 1)], 3.5f);
		if (N == 0) { CutTheCake(Thing); }
		bAteCake = true;
		return;
	}
	if (Key == TEXT("Elevator"))
	{
		if (const UTakeGameInstance* EGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			if (EGI->Beat == ETakeBeat::Wrap && EGI->SoulBits >= 3)
			{
				PlaySfx(TEXT("sfx_badge_ok"), Where, 0.8f);
				ShowSubtitle(TEXT(""), TEXT("You press C. It was already lit."), 2.5f);
				StartEnding();
				return;
			}
		}
		PlaySfx(TEXT("sfx_badge_ok"), Where, 0.5f);
		static const TCHAR* Presses[] = {
			TEXT("You press 2. The light comes on, then goes out. 2 hasn't happened yet."),
			TEXT("You press 3. The display: FLOOR 3 IS IN POST-PRODUCTION."),
			TEXT("You press C. The display: AUTHORISED PERSONNEL ONLY. ...YET."),
			TEXT("You press 1. The display: YOU ARE ON 1. YOU HAVE ALWAYS BEEN ON 1."),
			TEXT("You press B4. The display: B4 IS NOT A FLOOR. (The vending machine hums, somewhere.)"),
			TEXT("You press the alarm. Nothing. Then, very quietly, the PA: \"We heard you.\""),
		};
		ShowSubtitle(TEXT(""), Presses[N % int32(UE_ARRAY_COUNT(Presses))], 4.f);
		return;
	}
	if (Key == TEXT("TrustFall"))
	{
		const UTakeGameInstance* TGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
		if (TGI && TGI->Beat == ETakeBeat::Overtime) { StartTrustFall(); }
		else { ShowSubtitle(TEXT(""), TEXT("A mint X on the floor. You're not supposed to stand on it yet."), 2.5f); }
		return;
	}
	if (Key == TEXT("ServiceElevator"))
	{
		const UTakeGameInstance* SGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
		if (SGI && SGI->SoulBits >= 3) { ExitWard(); }
		else
		{
			PlaySfx(TEXT("sfx_door_locked"), Where);
			ShowSubtitle(TEXT(""), TEXT("SERVICE ELEVATOR. Access pass 3 required. (The nurses' station terminal.)"), 3.f);
		}
		return;
	}
	if (Key == TEXT("Printer"))
	{
		PlaySfx(TEXT("sfx_printer"), Where, 0.7f);
		const APawn* PrintMe = Cast<APawn>(GetOwner());
		FVector Out = Thing->GetActorLocation() + FVector(0.f, 0.f, 100.f);
		if (PrintMe) { FVector D = PrintMe->GetActorLocation() - Thing->GetActorLocation(); D.Z = 0.f; Out += D.GetSafeNormal() * 40.f; }
		if (AActor* Page = SpawnProp(TEXT("SM_Brochure"), Out, FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f)))
		{
			const int32 Which = N % 3 + 1;
			const FString Mi = FString::Printf(TEXT("/Game/Props/Materials/MI_Printout_%d.MI_Printout_%d"), Which, Which);
			if (UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, *Mi))
			{
				if (UStaticMeshComponent* SM = Cast<UStaticMeshComponent>(Page->GetRootComponent())) { SM->SetMaterialByName(FName(TEXT("M_SignFace")), M); }
			}
		}
		static const TCHAR* Prints[] = {
			TEXT("It prints a memo you didn't send: \"Continuity is everyone's responsibility.\""),
			TEXT("PRINT JOB 4470 (queued). It's been waiting for a long time. The page is blank except for a row of squares."),
			TEXT("\"YOUR PERFORMANCE: 99%. Please give the remaining 1%.\""),
		};
		ShowSubtitle(TEXT(""), Prints[N % 3], 4.f);
		return;
	}
	if (Key == TEXT("Cooler"))
	{
		PlaySfx(TEXT("sfx_tap_water"), Where, 0.5f);
		GI_AddEnergy(this, 5.f);
		static const TCHAR* Gossip[] = {
			TEXT("...did you hear about 4470? No? ...Nobody did."),
			TEXT("I heard the Chairman's never been in the building. I heard he's always in the building."),
			TEXT("Don't drink from the one in Corridor B. There isn't one in Corridor B."),
			TEXT("The new one's doing well. They always do well. Until the retake."),
			TEXT("...shh. They're here."),
		};
		// Overtime: the gossip slips out of character, like actors waiting between setups
		static const TCHAR* GossipAct3[] = {
			TEXT("Is it a wrap yet? My call time was nine. Nine yesterday."),
			TEXT("Overtime pays double. They said that last take too."),
			TEXT("Don't look at me, I'm not in this scene. ...Am I in this scene?"),
			TEXT("The tall one's a stunt double. Nobody knows whose."),
			TEXT("...shh. Rolling. We're rolling."),
		};
		const UTakeGameInstance* CGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
		const bool bOvertime = CGI && CGI->CurrentAct() >= 3;
		const FString Line = bOvertime ? GossipAct3[N % int32(UE_ARRAY_COUNT(GossipAct3))] : Gossip[N % int32(UE_ARRAY_COUNT(Gossip))];
		ATakeCoworker* Gossiper = FindGossiper(GetWorld());
		if (Gossiper && !Gossiper->IsHidden() && FVector::Dist(Gossiper->GetActorLocation(), Thing->GetActorLocation()) < 600.f)
		{
			Gossiper->SayLine(Line);   // the one by this cooler says it (as a subtitle and over their head)
		}
		else
		{
			ShowSubtitle(TEXT("Coworker"), TEXT("(by the cooler) ") + Line, 4.f);   // the other cooler: someone you can't see
		}
		QueueSubtitle(TEXT(""), TEXT("A paper cup of water. Cold. Exactly cold."));
		return;
	}
	if (Key == TEXT("Sink"))
	{
		PlaySfx(TEXT("sfx_tap_water"), Where, 0.7f);
		ShowSubtitle(TEXT(""), Fear >= 0.5f ? FString(TEXT("The water runs grey. Then clear. Then it was always clear."))
			: Pick({ TEXT("The water is exactly the right temperature. Of course it is."), TEXT("You wash your hands. The sign said twice."),
			         TEXT("Your hands are clean. They've been clean all day.") }), 3.5f);
	}
	else if (Key == TEXT("Dryer"))
	{
		PlaySfx(TEXT("sfx_hand_dryer"), Where, 0.8f);
		ShowSubtitle(TEXT(""), Pick({ TEXT("It starts before your hands are under it."), TEXT("It roars. Somewhere, a coworker says \"Bless you.\""),
		                              TEXT("It stops the moment you look at it. It waits for you to look away.") }), 3.5f);
	}
	else if (Key == TEXT("Toilet"))
	{
		PlaySfx(TEXT("sfx_flush"), Where, 0.8f);
		ShowSubtitle(TEXT(""), Pick({ TEXT("*flush*"), TEXT("*flush*  ...and, from the next stall, an identical flush."),
		                              TEXT("*flush*  The PA, very quietly: \"Thank you.\"") }), 3.f);
	}
}

void UTakeGrabComponent::TickFearReveals()
{
	if (FearReveals.Num() == 0) { return; }
	const float Fear = FindFearDirector() ? FindFearDirector()->Fear : 0.f;
	const bool bShow = Fear >= 0.5f;
	if (bShow == bFearRevealShown) { return; }
	bFearRevealShown = bShow;
	for (const TWeakObjectPtr<AActor>& A : FearReveals)
	{
		if (AActor* Act = A.Get())
		{
			Act->SetActorHiddenInGame(!bShow);   // the shoes under the stall: there only when you're afraid
			Act->SetActorEnableCollision(false);
		}
	}
}

// ------------------------------------------------------------------ energy, cameras, skip
void UTakeGrabComponent::TickEnergy(float DeltaTime)
{
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	ACharacter* Me = Cast<ACharacter>(GetOwner());
	if (!GI || !Me || PauseMenu || SlateT >= 0.f) { return; }
	if (BaseWalkSpeed < 0.f) { BaseWalkSpeed = Me->GetCharacterMovement()->MaxWalkSpeed; }
	const float FearNow = FindFearDirector() ? FindFearDirector()->Fear : 0.f;
	// Full to empty in about 15 minutes; Fear burns it three times as fast
	GI->Energy = FMath::Max(0.f, GI->Energy - DeltaTime * (100.f / 900.f) * (FearNow >= 0.5f ? 3.f : 1.f));
	// Fear stinger: the moment the office turns (crossing 0.6, where coworkers start to stare)
	if (FearNow >= 0.35f && LastFear < 0.35f)
	{
		PlaySfx2D(TEXT("sfx_fear_hit"), 0.9f);
		if (!bTensionFirstDone) { bTensionFirstDone = true; PlayMusicCue(TEXT("Fear rises for the first time"), TEXT("mus_halcyon_tension"), true); }
	}
	// Fear Lvl 1: Corridor B, the first time it really turns (the loop is on from 0.5)
	if (FearNow >= 0.5f && LastFearLvl < 0.5f && !bFearCorridorDone)
	{
		const UTakeGameInstance* LvlGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
		if (LvlGI && LvlGI->Beat == ETakeBeat::Corridor)
		{
			bFearCorridorDone = true;
			PlayMusicCue(TEXT("Corridor B turns"), TEXT("mus_halcyon_fear1"), true);
		}
	}
	LastFearLvl = FearNow;
	LastFear = FearNow;
	const bool bLow = GI->Energy < 25.f;
	Me->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * (bLow ? 0.8f : 1.f);
	if (bLow && !bLowEnergyNoted)
	{
		bLowEnergyNoted = true;
		QueueSubtitle(TEXT(""), TEXT("You're running low. Your energy is their greatest asset. Coffee? A snack?"));
	}
	if (GI->Energy > 40.f) { bLowEnergyNoted = false; }
}

void UTakeGrabComponent::TickCams(float DeltaTime)
{
	const APawn* Me = Cast<APawn>(GetOwner());
	if (!Me || Cams.Num() == 0) { return; }
	for (const TWeakObjectPtr<AActor>& C : Cams)
	{
		AActor* Cam = C.Get();
		if (!Cam) { continue; }
		const FVector To = Me->GetActorLocation() - Cam->GetActorLocation();
		if (To.Size2D() > 1800.f) { continue; }
		// The lens looks along the actor's +Y: yaw = direction to you - 90. It turns slowly, like it's thinking about it.
		const float Want = To.Rotation().Yaw - 90.f;
		const FRotator Now = Cam->GetActorRotation();
		Cam->SetActorRotation(FRotator(Now.Pitch, FMath::FixedTurn(Now.Yaw, Want, 40.f * DeltaTime), Now.Roll));
	}
}

void UTakeGrabComponent::SkipTasks()
{
	if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->TakeSkip();
		ShowSubtitle(TEXT(""), TEXT("(Playtest skip: tasks done.)"), 2.f);
	}
}

void UTakeGrabComponent::OpenNotes()
{
	if (!PauseMenu) { TogglePause(); }          // pauses the game (not while seated / inspecting)
	if (PauseMenu) { PauseMenu->OnMenu(6); }    // 6 = the Playtest notes page, typing box focused
}

void UTakeGrabComponent::TakeScreenshot()
{
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{
			PC->ConsoleCommand(TEXT("HighResShot 1"));
			PlaySfx2D(TEXT("sfx_clapper_snap"), 0.25f);
			ShowSubtitle(TEXT(""), TEXT("(Screenshot saved: Saved/Screenshots)"), 1.5f);
		}
	}
}

void UTakeGrabComponent::TestStalker()
{
	if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->TakeStalker();
		ShowSubtitle(TEXT(""), TEXT("(Playtest: the stalker is coming.)"), 2.f);
	}
}
