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
#include "DirectorsCut_58.h"
#include "TakeGameInstance.h"
#include "TakeFearDirector.h"
#include "TakeKeyLockLibrary.h"
#include "TakeWidgets.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Kismet/GameplayStatics.h"

namespace
{
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
	GI->WorkdayMinutes = FMath::Max(GI->WorkdayMinutes, bCallIsMeeting ? 13.f * 60.f : 12.f * 60.f + 30.f);
	PlaySfx2D(TEXT("sfx_badge_ok"), 0.5f);

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
		if (It->ActorHasTag(TEXT("Act2Reveal")))
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
		if (UStaticMeshComponent* SM = Cast<UStaticMeshComponent>(A->GetRootComponent())) { SM->SetSimulatePhysics(R.bPhysics); }
	}
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Lunch is out: revealed %d Act 2 thing(s)"), Act2Things.Num());
}

void UTakeGrabComponent::FoodEaten(AActor* Food)
{
	for (FFood& F : Foods)
	{
		if (F.Actor.Get() == Food) { F.Back = FMath::FRandRange(40.f, 70.f); return; }
	}
	// A vended snack: gone for good (the machine has more)
	Spawned.Remove(Food);
	if (Food) { Food->SetLifeSpan(0.5f); }
}

void UTakeGrabComponent::TickFun(float DeltaTime)
{
	FVector Eye; FRotator View;
	const bool bView = GetView(Eye, View);
	// Food comes back to where it was, when you aren't looking at the spot
	for (FFood& F : Foods)
	{
		if (F.Back < 0.f) { continue; }
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
	if (MailSeen >= 0 && Total > MailSeen && !bSeated)
	{
		PlaySfx2D(TEXT("sfx_badge_ok"), 0.25f);
		QueueSubtitle(TEXT(""), FString::Printf(TEXT("(New email. %d unread on your terminal.)"), Unread));
	}
	MailSeen = Total;
}

// ------------------------------------------------------------------ restroom and other "Use=<Key>" things
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
