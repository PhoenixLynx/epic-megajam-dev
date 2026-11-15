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
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/TargetPoint.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"

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
		// Stay exactly in the chair: no walking physics, and don't get pushed out by the chair or desk
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->GravityScale = 0.f;
		UCapsuleComponent* Cap = GetCapsuleComponent();
		Cap->SetCollisionResponseToAllChannels(ECR_Ignore);
		Cap->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);          // you still can't walk through them
		Cap->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);   // and things you throw still hit them
	}
	// Every guard and custodian counts for the lunchtime check-in, not only the two by the supply closet
	// (playtest: the lobby guard and the hallway custodian looked like the obvious ones, and didn't count)
	if (NPCRole.Equals(TEXT("Guard"), ESearchCase::IgnoreCase) || NPCRole.Equals(TEXT("Custodian"), ESearchCase::IgnoreCase)) { bStoryNPC = true; }
	if (bStoryNPC) { bTalksWhenNear = false; }   // story characters wait for you to press E
	SetupSkeletal();
	LoadLines();
}

bool ATakeCoworker::IsBlackCoworker() const
{
	auto Key = [](const AActor* A) { const FVector L = A->GetActorLocation(); return FMath::RoundToInt(L.Y / 250.f) * 100000 + FMath::RoundToInt(L.X); };
	const int32 Mine = Key(this);
	int32 Rank = 0;
	for (TActorIterator<ATakeCoworker> It(GetWorld()); It; ++It)
	{
		if (*It == this || !It->NPCRole.Equals(TEXT("Coworker"), ESearchCase::IgnoreCase)) { continue; }
		// counted within the same kind of job (playtest: "make sure the diversity is mixed between different npc actions"):
		// people working at desks, people walking about, people standing, each get their own 30%
		if (It->bSitsAtDesk != bSitsAtDesk || It->bWanders != bWanders) { continue; }
		const int32 Theirs = Key(*It);
		if (Theirs < Mine || (Theirs == Mine && It->GetName() < GetName())) { ++Rank; }
	}
	// 30%: in each run of ten neighbours, three are Black, at shuffled places (never two next to each other)
	static const bool Pattern[10] = { true, false, false, true, false, false, false, true, false, false };   // (the first of each group, so even a small group has one)
	return Pattern[Rank % 10];
}

void ATakeCoworker::SetupSkeletal()
{
	const TCHAR* MeshName = NPCRole.Equals(TEXT("Guard"), ESearchCase::IgnoreCase) ? TEXT("SK_MannequinGuard")
		: NPCRole.Equals(TEXT("Custodian"), ESearchCase::IgnoreCase) ? TEXT("SK_MannequinCustodian")
		: NPCRole.Equals(TEXT("Chef"), ESearchCase::IgnoreCase) ? TEXT("SK_MannequinChef") : TEXT("SK_Mannequin");
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
	M->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -90.f), FRotator(0.f, MeshYaw, 0.f));
	M->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	M->SetHiddenInGame(false);
	Body->SetHiddenInGame(true);   // still there for E / line-of-sight traces
	// Faces (Sol's ART-001): the head's "M_Face" card shows the calm face; the Fear face when they stare
	FaceCalm = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Props/Materials/MI_Face_Calm.MI_Face_Calm"));
	FaceFear = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Props/Materials/MI_Face_Fear.MI_Face_Fear"));
	// Security and the custodian don't smile (neutral face, when Sol's export is in); a frown when you hit someone
	if (NPCRole.Equals(TEXT("Guard"), ESearchCase::IgnoreCase) || NPCRole.Equals(TEXT("Custodian"), ESearchCase::IgnoreCase))
	{
		if (UMaterialInterface* Neutral = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Props/Materials/MI_Face_Neutral.MI_Face_Neutral"))) { FaceCalm = Neutral; }
	}
	FaceFrown = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Props/Materials/MI_Face_Frown.MI_Face_Frown"));
	// About three in ten of the office are Black (playtest: "30% if possible"): dark skin (the M_Skin slots), same suits as everyone
	// Spread out, not side by side (playtest: "they shouldn't all sit together, they should be randomly placed"): every
	// office coworker is ranked by where they stand (rows of desks, then along the row) and every third or so in that
	// order is Black, with a fixed shuffle so it doesn't read as a pattern. Same people every take.
	if (NPCRole.Equals(TEXT("Coworker"), ESearchCase::IgnoreCase) && IsBlackCoworker())
	{
		// Playtest: "switch black skin tone to previous collectors... the white employees are white and having brown or beige
		// shades does not fit the aesthetic". So: the Collector's dark (M_MetalDark), the same stylised flat colour as the
		// suits. It's already used on the guard's cap, so it's flagged for rigged meshes (no grey grid). M_SkinTone is the fallback.
		UMaterialInterface* Dark = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Props/Meshes/M_MetalDark.M_MetalDark"));
		if (!Dark) { Dark = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Props/Materials/M_SkinTone.M_SkinTone")); }
		if (UMaterialInterface* Skin = Dark)
		{
			const TArray<FName> Slots = M->GetMaterialSlotNames();
			int32 Toned = 0;
			FString AllSlots;
			for (int32 S = 0; S < M->GetNumMaterials(); ++S)
			{
				const FString SlotName = Slots.IsValidIndex(S) ? Slots[S].ToString() : FString();
				AllSlots += SlotName + TEXT(" ");
				if (!SlotName.Contains(TEXT("Skin"))) { continue; }   // head, neck, hands; the suit stays the office's colour
				M->SetMaterial(S, Skin);
				++Toned;
			}
			// (playtest: "still texture map for head" - this says which slots the mesh really has)
			UE_LOG(LogTake, Log, TEXT("[TAKE 4471] %s: brown skin on %d slot(s); slots: %s"), *GetName(), Toned, *AllSlots);
		}
	}
	FaceMode = -1;
	SetFace(false);
	PlayAnim(bSitsAtDesk && AnimSit ? AnimSit.Get() : AnimIdle.Get());
}

void ATakeCoworker::SetFace(bool bFear)
{
	// 0 calm (or neutral), 1 Fear, 2 frown (just hit, outside Fear)
	const int32 Mode = bFear ? 1 : (FrownHold > 0.f && FaceFrown ? 2 : 0);
	if (!bSkeletal || Mode == FaceMode) { return; }
	UMaterialInterface* Mat = Mode == 1 ? FaceFear.Get() : Mode == 2 ? FaceFrown.Get() : FaceCalm.Get();
	if (!Mat) { return; }
	FaceMode = Mode;
	GetMesh()->SetMaterialByName(FName(TEXT("M_Face")), Mat);
}

void ATakeCoworker::SetMeshYaw(float Yaw)
{
	MeshYaw = Yaw;
	if (bSkeletal) { GetMesh()->SetRelativeRotation(FRotator(0.f, MeshYaw, 0.f)); }
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
	StorySets.Reset();
	const UTakeGameInstance* GameInst = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	LinesAct = GameInst ? GameInst->CurrentAct() : 1;
	bLinesClocksFrozen = GameInst && GameInst->bClocksFrozen;
	TArray<FString> MorningLines;   // Act 1 small talk, used in a later act if that act has none for this role
	auto Matches = [this](const FString& RowRole) { return RowRole.Equals(NPCRole, ESearchCase::IgnoreCase); };
	if (const UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_NPCLines.DT_NPCLines")))
	{
		TArray<FTakeNPCLineRow*> Rows;
		Table->GetAllRows<FTakeNPCLineRow>(TEXT("TakeCoworker"), Rows);
		TMap<FString, TArray<TPair<int32, FString>>> Story;   // set -> (order, line)
		for (const FTakeNPCLineRow* R : Rows)
		{
			if (!R || R->Act < 1 || R->Act > LinesAct || !Matches(R->Role) || (R->IsAgent && !bIsAgent)) { continue; }
			// Story conversations ("Story1", "Pass1Story2", "LunchStory1", ...): only story NPCs use them, never as small talk.
			// Every act up to the current one is loaded, so a conversation you're part-way through survives the act change.
			const int32 StoryAt = R->Context.Find(TEXT("Story"), ESearchCase::CaseSensitive);
			if (StoryAt != INDEX_NONE && R->Context.Mid(StoryAt + 5).IsNumeric())
			{
				if (bStoryNPC)
				{
					Story.FindOrAdd(R->Context.Left(StoryAt)).Add(TPair<int32, FString>(FCString::Atoi(*R->Context.Mid(StoryAt + 5)), R->Line));
				}
				continue;
			}
			if (R->Act != LinesAct)
			{
				if (R->Act == 1 && R->Context != TEXT("Greeting") && R->Context != TEXT("BadgeCheck") && R->Context != TEXT("AfterCoffee")) { MorningLines.Add(R->Line); }
				continue;
			}
			// Lines written for a specific scene (the call, the meeting, after a rewind) aren't small talk
			if (R->Context == TEXT("VideoCall") || R->Context == TEXT("Meeting") || R->Context == TEXT("AfterRewind") || R->Context == TEXT("AfterMeeting")) { continue; }
			if (R->Context == TEXT("AfterClocks") && !bLinesClocksFrozen) { continue; }   // only once every clock reads 2:14
			if (R->Context == TEXT("Greeting") || R->Context == TEXT("BadgeCheck")) { Greetings.Add(R->Line); }
			else if (R->Context == TEXT("AfterCoffee")) { CoffeeLines.Add(R->Line); }
			else if (R->Context == TEXT("Hit")) { HitLines.Add(R->Line); }
			else { Lines.Add(R->Line); }
		}
		for (auto& Set : Story)
		{
			Set.Value.Sort([](const TPair<int32, FString>& A, const TPair<int32, FString>& B) { return A.Key < B.Key; });
			TArray<FString>& Ordered = StorySets.FindOrAdd(Set.Key);
			for (const TPair<int32, FString>& L : Set.Value) { Ordered.Add(L.Value); }
		}
	}
	// Overtime (Act 3): the story characters have something new to say (no more lunch talk)
	if (bStoryNPC && LinesAct >= 3)
	{
		TArray<FString>& Over = StorySets.FindOrAdd(TEXT("Overtime"));
		if (NPCRole.Equals(TEXT("Receptionist"), ESearchCase::IgnoreCase))
		{
			Over = { TEXT("Still here, {EMP}? Everyone's still here. Nobody's signed out all day."),
			         TEXT("Team building is in the Wellness Center. End of Corridor B. Bring your pass."),
			         TEXT("After that, there's a party. There's always a party. ...You didn't hear that from me.") };
		}
		else if (NPCRole.Equals(TEXT("Guard"), ESearchCase::IgnoreCase))
		{
			Over = { TEXT("Overtime. Badge. ...Pass 2. Then the Wellness Center's open to you."),
			         TEXT("If you hear footsteps that aren't yours, keep walking. Don't run. Running makes it worse."),
			         TEXT("I'm not supposed to tell you there's a cake. So I didn't.") };
		}
		else if (NPCRole.Equals(TEXT("Custodian"), ESearchCase::IgnoreCase))
		{
			Over = { TEXT("The clocks stopped. I told you not to wait for them."),
			         TEXT("Something's been walking the open office. Heavy. Doesn't wipe its feet."),
			         TEXT("Below the Wellness Center there's a ward. I clean that too. Nobody's ever in the beds. Except when they are.") };
		}
		else { StorySets.Remove(TEXT("Overtime")); }
	}
	if (Lines.Num() == 0) { Lines = MorningLines; }
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

void ATakeCoworker::HitByProp(float Speed)
{
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now - LastHitTime < 1.0 || bVanished) { return; }
	LastHitTime = Now;
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (Player && State != ETakeCoworkerState::Watching && !bSitsAtDesk)   // (seated: the chair doesn't turn with them, so they'd clip through it)
	{
		const FRotator Face = (Player->GetActorLocation() - GetActorLocation()).Rotation();
		SetActorRotation(FRotator(0.f, Face.Yaw, 0.f));   // they turn to see who did it
		if (AAIController* HitAI = Cast<AAIController>(GetController())) { HitAI->StopMovement(); }   // and stop where they are
	}
	if (bSitsAtDesk && SpinLeft <= 0.f && State != ETakeCoworkerState::Watching)
	{
		// Seated: a full spin in the chair, and the chair spins with them, back to where they were
		SpinChair = nullptr;
		float BestD = 130.f;
		for (TActorIterator<AStaticMeshActor> ChairIt(GetWorld()); ChairIt; ++ChairIt)
		{
			UStaticMeshComponent* SMC = ChairIt->GetStaticMeshComponent();
			if (!SMC || !SMC->GetStaticMesh() || !SMC->GetStaticMesh()->GetName().Contains(TEXT("Chair"))) { continue; }
			const float D = FVector::Dist2D(ChairIt->GetActorLocation(), GetActorLocation());
			if (D < BestD) { BestD = D; SpinChair = *ChairIt; }
		}
		if (SpinChair.IsValid())
		{
			if (UStaticMeshComponent* SMC = Cast<AStaticMeshActor>(SpinChair.Get())->GetStaticMeshComponent())
			{
				SMC->SetMobility(EComponentMobility::Movable);
			}
			SpinChairFromYaw = SpinChair->GetActorRotation().Yaw;
		}
		SpinFromYaw = GetActorRotation().Yaw;
		SpinLeft = 0.9f;
	}
	if (State == ETakeCoworkerState::Watching)
	{
		// Fear: no words, and they don't move. The face goes wrong, and the room gets a little worse.
		FaceFearHold = 4.f;
		if (FearDirector.IsValid()) { FearDirector->StartCreep(FMath::Min(1.f, FearDirector->Fear + 0.05f), 0.5f); }
		return;
	}
	UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI)
	{
		++GI->CoworkersHit;
		if (NPCRole.Equals(TEXT("Guard"), ESearchCase::IgnoreCase)) { ++GI->GuardComplaints; }
	}
	CooldownLeft = 0.f;   // they always answer this
	FrownHold = 4.f;      // a frown, for a moment
	if (ActorHasTag(TEXT("Collector")))
	{
		static const TCHAR* Unruffled[] = {
			TEXT("Careful. The hat is a rental."), TEXT("I've been hit by better. By you, actually. Take 4470."),
			TEXT("Noted. Everything is noted."), TEXT("Violence is a choice. So is the hat. I chose the hat."),
			TEXT("You can throw things at me all night, {EMP}. I'll still need your keys."),
		};
		SayLine(Unruffled[FMath::RandRange(0, int32(UE_ARRAY_COUNT(Unruffled)) - 1)]);
		return;
	}
	if (GetFear() >= 0.4f) { FaceFearHold = 2.5f; }   // uneasy already: for a moment, the other face
	if (HitLines.Num() > 0) { SayLine(HitLines[FMath::RandRange(0, HitLines.Num() - 1)]); return; }
	if (NPCRole.Equals(TEXT("Guard"), ESearchCase::IgnoreCase)) { SayLine(TEXT("That's going in my report.")); }
	else if (NPCRole.Equals(TEXT("Custodian"), ESearchCase::IgnoreCase)) { SayLine(TEXT("I'll be cleaning that up, then.")); }
	else { SayLine(TEXT("Hey! I haven't even had my coffee yet.")); }
}

void ATakeCoworker::SplashedByCoffee()
{
	if (State == ETakeCoworkerState::Watching || bVanished) { return; }
	if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { ++GI->CoworkersHit; }   // HR counts hot beverages twice
	FrownHold = 6.f;
	static const TCHAR* SplashLines[] = {
		TEXT("That was a nine-dollar oat latte. Not even mine. Still."),
		TEXT("It's in my collar. It's in my COLLAR."),
		TEXT("I'm filing this under hot beverage incident. There's a form. There's always a form."),
		TEXT("You know what, I was going to be nice to you today."),
		TEXT("This is my only shirt. They gave us one shirt."),
		TEXT("Is that... is that decaf? You threw DECAF at me?"),
	};
	if (NPCRole.Equals(TEXT("Guard"), ESearchCase::IgnoreCase)) { SayLine(TEXT("Assault with a beverage. I'm writing BEVERAGE in capitals.")); return; }
	if (NPCRole.Equals(TEXT("Custodian"), ESearchCase::IgnoreCase)) { SayLine(TEXT("On me AND the floor. Efficient. I'll give you that.")); return; }
	SayLine(SplashLines[FMath::RandRange(0, int32(UE_ARRAY_COUNT(SplashLines)) - 1)]);
}

FString ATakeCoworker::RefusalLine()
{
	static const TCHAR* Haunt[] = {
		TEXT("You were supposed to say yes."), TEXT("4470 said no too. Ask 4470 how that went."), TEXT("We clapped for you."),
		TEXT("Nobody leaves the take, {EMP}."), TEXT("Cut. Cut. Cut."), TEXT("You'll be back at nine. You're always back at nine."),
		TEXT("Your desk is already warm for 4472."), TEXT("The cake was for you."), TEXT("We remember every stapler."),
		TEXT("Smile. Every day is a performance review."), TEXT("There is no outside, {EMP}."), TEXT("Places, everyone. Places."),
		TEXT("We were so proud of you. Were."), TEXT("Who do you think you're keeping them for?"),
	};
	return Haunt[FMath::RandRange(0, int32(UE_ARRAY_COUNT(Haunt)) - 1)];
}

FString ATakeCoworker::NextLine()
{
	if (bAtParty && FearDirector.IsValid() && FearDirector->bForceRed) { return RefusalLine(); }   // the NO ending
	if (bAtParty && State == ETakeCoworkerState::Watching)   // Fear at the party: the smiles stay, the words don't
	{
		static const TCHAR* Wrong[] = {
			TEXT("Great wor- ..."), TEXT("..."), TEXT("{EMP}. {EMP}. {EMP}."), TEXT("(They just look at you. The smile stays exactly where it was.)"),
			TEXT("Places."), TEXT("Again."), TEXT("We were so proud of 4470 too."),
		};
		return Wrong[FMath::RandRange(0, int32(UE_ARRAY_COUNT(Wrong)) - 1)];
	}
	if (bAtParty)   // the wrap party: nobody talks shop (no supply closets, no passes)
	{
		static const TCHAR* Party[] = {
			TEXT("Great work today, {EMP}. Really. Same time tomorrow?"),
			TEXT("Have you tried the cake? I haven't. I'm not sure it's for eating."),
			TEXT("I loved your scene with the coffee machine. Very natural."),
			TEXT("Is it over? It feels over. It never feels over."),
			TEXT("Craft services really outdid themselves. Same cake as last time."),
			TEXT("I'm told they're keeping the take where you looked at the camera."),
			TEXT("Cheers. To 4471. ...Sorry, I always want to say 4470."),
			TEXT("Don't stand on your mark. We're wrapped. ...Old habits."),
		};
		return Party[FMath::RandRange(0, int32(UE_ARRAY_COUNT(Party)) - 1)];
	}
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
	// Now and then, a nudge toward whatever's next on your list (finished tasks -> "don't be late for the 12:30")
	if (GI && !bStoryNPC && FMath::FRand() < 0.3f)
	{
		const FString Hint = TaskHint(GI);
		if (!Hint.IsEmpty() && Hint != LastHint) { LastHint = Hint; return Hint; }
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

FString ATakeCoworker::TaskHint(const UTakeGameInstance* GI) const
{
	if (NPCRole.Equals(TEXT("Chef"), ESearchCase::IgnoreCase))
	{
		static const TCHAR* Chef[] = {
			TEXT("Today's special is the special. Every day."), TEXT("Turkey? You asked for turkey. You always ask for turkey."),
			TEXT("Don't read the sprinkles."), TEXT("Lunch is served. Lunch is served. ...Sorry. Habit."),
		};
		return Chef[FMath::RandRange(0, int32(UE_ARRAY_COUNT(Chef)) - 1)];
	}
	if (GI->CurrentAct() >= 3)   // overtime: point at what's next, a little more out of character each time
	{
		if (GI->SoulBits < 2) { return TEXT("Access pass two's on the supply closet terminal. Everyone has one. Almost everyone."); }
		if (!GI->bTrustFell) { return TEXT("Team building. Wellness Center. Attendance is mandatory. It's always been mandatory."); }
		if (GI->SoulBits < 3) { return TEXT("...You're not supposed to be up here. You're supposed to be downstairs. In the ward."); }
		return TEXT("Elevator. C. Everyone's waiting. Don't tell them I told you.");
	}
	if (GI->CurrentAct() < 2)
	{
		if (!GI->bTrainingComplete) { return TEXT("Done your compliance training yet? It's mandatory. Everything is."); }
		if (!GI->bReadEmail) { return TEXT("Inbox zero yet? HALCYON loves an inbox zero."); }
		if (!GI->bGotCoffee) { return TEXT("Coffee's fresh in the kitchen. It's always fresh."); }
		if (GI->Beat <= ETakeBeat::Meeting) { return TEXT("Ten o'clock, conference room. Don't be late. You're never late."); }
		return FString();
	}
	if (GI->bClocksFrozen) { return TEXT("Is it just me or has it been 2:14 for a while?"); }
	if (!GI->bJoinedCall) { return TEXT("Twelve thirty, conference room. The Chairman's calling in. Camera off, as usual."); }
	if (GI->CoworkersCheckedIn < 3) { return TEXT("Have you said hi to everyone? Say hi to everyone."); }
	if (!GI->bAttendedMeeting) { return TEXT("One o'clock, same room. Same meeting. You'll see."); }
	if (!GI->bRepliedChairman) { return TEXT("Did you answer the Chairman's email? He reads every reply. He writes some of them too."); }
	return FString();
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
		if (bStoryNPC && State != ETakeCoworkerState::Watching && HasNewConversation())   // someone to talk to
		{
			Speech->SetWorldRotation(FRotator(0.f, (Eye - Speech->GetComponentLocation()).Rotation().Yaw, 0.f));
			Speech->SetTextRenderColor(FColor(80, 200, 180));
			Speech->SetWorldSize(16.f);
			Speech->SetText(FText::FromString(TEXT("!")));
		}
		else if (bStoryNPC)
		{
			Speech->SetText(FText::GetEmpty());   // heard everything for now: the "!" goes away
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
	if (CooldownLeft > 0.f || IsHidden()) { return false; }
	const FString Line = NextLine();
	if (Line.IsEmpty()) { return false; }
	SayLine(Line);
	return true;
}

bool ATakeCoworker::CurrentStorySet(FString& OutSet) const
{
	if (StorySets.Num() == 0) { return false; }
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI)
	{
		if (GI->CurrentAct() >= 3 && StorySets.Contains(TEXT("Overtime")))    { OutSet = TEXT("Overtime"); return true; }
		if (GI->Beat >= ETakeBeat::Lunch && StorySets.Contains(TEXT("Lunch")))  { OutSet = TEXT("Lunch"); return true; }
		if (GI->Beat >= ETakeBeat::Corridor && StorySets.Contains(TEXT("Slip"))) { OutSet = TEXT("Slip"); return true; }
		if (GI->MorningTasksDone() && StorySets.Contains(TEXT("Done")))   { OutSet = TEXT("Done"); return true; }
		if (GI->HasPass(1) && StorySets.Contains(TEXT("Pass1")))          { OutSet = TEXT("Pass1"); return true; }
	}
	if (StorySets.Contains(FString())) { OutSet = FString(); return true; }
	return false;
}

bool ATakeCoworker::HasNewConversation() const
{
	if (bAtParty) { return ActorHasTag(TEXT("Collector")) ? bPartyReady : true; }   // at the party anyone will talk; the Collector's "!" waits
	if (StorySets.Num() == 0) { return true; }   // DataTable not re-imported yet: keep the old always-on "!"
	FString Set;
	return CurrentStorySet(Set) && (bInConversation || !FinishedSets.Contains(Set));
}

bool ATakeCoworker::Talk()
{
	FString Set;
	if (bAtParty)   // party small talk only (and the Collector has his own business)
	{
		UTakeGrabComponent* Hands = nullptr;
		if (APawn* P = UGameplayStatics::GetPlayerPawn(this, 0)) { Hands = P->FindComponentByClass<UTakeGrabComponent>(); }
		if (ActorHasTag(TEXT("Collector"))) { if (Hands) { Hands->OnCollectorTalk(this); } return true; }
		SayLine(NextLine());
		if (!bPartyChatted) { bPartyChatted = true; if (Hands) { Hands->OnPartyChat(); } }
		return true;
	}
	if (!bStoryNPC || !CurrentStorySet(Set)) { return Speak(TEXT("Talk")); }
	const TArray<FString>* Convo = StorySets.Find(Set);
	if (!Convo || Convo->Num() == 0) { return Speak(TEXT("Talk")); }
	if (!bInConversation || ActiveSet != Set)
	{
		if (FinishedSets.Contains(Set) && !(bInConversation && ActiveSet != Set))
		{
			// Heard it already: small talk (no cooldown for E), or the last line again if there is none
			const FString Small = NextLine();
			SayLine(Small.IsEmpty() ? Convo->Last() : Small);
			return true;
		}
		ActiveSet = Set;
		StoryIndex = 0;
		bInConversation = true;
		bGreeted = true;   // the conversation is the greeting
	}
	const int32 LineIndex = FMath::Clamp(StoryIndex, 0, Convo->Num() - 1);
	FString Say = (*Convo)[LineIndex];
	const UTakeGameInstance* TalkGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (TalkGI && TalkGI->LastEnding > 0 && Set.IsEmpty() && LineIndex == 0 && NPCRole.Equals(TEXT("Receptionist"), ESearchCase::IgnoreCase))
	{
		Say = TalkGI->LastEnding == 1 ? TEXT("Good morning, {EMP}! Right on time. ...Welcome back. Again. The director says hello.")
			: TEXT("Good morning, {EMP}! Right on time. ...Welcome back. Again. You said no last time. We noticed.");
	}
	if (TalkGI && TalkGI->bJoinedCall && Say.Contains(TEXT("12:30")))   // the call already happened
	{
		Say = TalkGI->bAttendedMeeting ? TEXT("How were the Chairman's meetings? Both of them? He says you were wonderful. He says that about everyone.")
			: TEXT("How was the 12:30? There's a one o'clock too. Same room. Same Chairman. Camera still off.");
	}
	SayLine(Say);
	++StoryIndex;
	if (StoryIndex >= Convo->Num())
	{
		bInConversation = false;
		FinishedSets.Add(Set);
		if (APawn* P = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			if (UTakeGrabComponent* Hands = P->FindComponentByClass<UTakeGrabComponent>()) { Hands->OnStoryTalkFinished(NPCRole, Set); }
		}
	}
	return true;
}

void ATakeCoworker::SayLine(const FString& Line)
{
	FString Said = Line;
	// (playtest: "someone talked about the 12:30 meeting after 12:30") once you've been on the call, nobody invites you to it
	if (const UTakeGameInstance* CallGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (CallGI->bJoinedCall && (Said.Contains(TEXT("12:30")) || Said.Contains(TEXT("Twelve thirty"))))
		{
			Said = CallGI->bAttendedMeeting ? TEXT("How were the Chairman's meetings? Both of them? He says you were wonderful.")
				: TEXT("How was the 12:30? There's a one o'clock too. Same room. Same Chairman. Camera still off.");
		}
	}
	const FString Filled = TakeText::Fill(this, Said);
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
		if (Nav->GetRandomReachablePointInRadius(Home, bAtParty ? 450.f : 900.f, P)) { Out = P.Location; return true; }   // at the party: stay close
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
		if (TheftHold > 0.f) { break; }  // ...unless you just took their computer
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
	// New act (lunch, or loading a save): switch to that act's small talk and conversations
	ActCheckIn -= DeltaSeconds;
	if (ActCheckIn <= 0.f)
	{
		ActCheckIn = 1.f;
		const UTakeGameInstance* GameInst = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
		if (GameInst && (GameInst->CurrentAct() != LinesAct || GameInst->bClocksFrozen != bLinesClocksFrozen)) { LoadLines(); }
		if (ActorHasTag(TEXT("Act2Reveal")) || ActorHasTag(TEXT("Act3Reveal")))   // lunch attendant; the team-building circle
		{
			const int32 FromAct = ActorHasTag(TEXT("Act3Reveal")) ? 3 : 2;
			// The lunch attendant is only there for lunch (Act 2); the team-building circle from Act 3 on
			const bool bShow = bAtParty || (GameInst && (FromAct == 2 ? GameInst->CurrentAct() == 2 : GameInst->CurrentAct() >= FromAct));
			if (IsHidden() == bShow) { SetActorHiddenInGame(!bShow); SetActorEnableCollision(bShow); }
		}
	}
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
		// Bump: extras react (story characters wait for E)
		const bool bClearLine = !GetWorld()->LineTraceTestByChannel(Eye, GetActorLocation() + FVector(0.f, 0.f, 60.f), ECC_Visibility, SightParams);
		if (Dist < 95.f && !bStoryNPC && bClearLine) { Speak(TEXT("Bump")); }   // not through a door
		// Walk past: they talk on their own, one NPC at a time, not too often
		else if (bTalksWhenNear && Dist < NearRange && NearCooldownLeft <= 0.f && GetFear() < WatchFear
		         && GetWorld()->GetTimeSeconds() >= NextAnyoneMayTalk
		         && GetWorld()->LineTraceTestByChannel(Eye, GetActorLocation() + FVector(0.f, 0.f, 60.f), ECC_Visibility, SightParams) == false)
		{
			if (Speak(TEXT("Near"))) { NearCooldownLeft = FMath::FRandRange(NearCooldownMin, NearCooldownMax); }
		}
	}

	TickBumps(DeltaSeconds, Player);

	// Security keeps count. Enough incidents and the guard tells you (once) what it's going to cost you
	if (!bPayWarned && Player && NPCRole.Equals(TEXT("Guard"), ESearchCase::IgnoreCase) && State != ETakeCoworkerState::Watching && !bAtParty)
	{
		const UTakeGameInstance* PayGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
		if (PayGI && PayGI->CoworkersHit >= 5 && FVector::Dist2D(Player->GetActorLocation(), GetActorLocation()) < 450.f)
		{
			bPayWarned = true;
			const FRotator Face = (Player->GetActorLocation() - GetActorLocation()).Rotation();
			SetActorRotation(FRotator(0.f, Face.Yaw, 0.f));
			FrownHold = 6.f;
			SayLine(FString::Printf(TEXT("{EMP}. That's %d incidents on my log. Payroll's docking you. Every single one."), PayGI->CoworkersHit));
		}
	}
	FaceFearHold = FMath::Max(0.f, FaceFearHold - DeltaSeconds);
	FrownHold = FMath::Max(0.f, FrownHold - DeltaSeconds);
	if (SpinLeft > 0.f)
	{
		SpinLeft = FMath::Max(0.f, SpinLeft - DeltaSeconds);
		const float A = FMath::InterpEaseOut(0.f, 360.f, 1.f - SpinLeft / 0.9f, 2.f);   // fast, then slowing to a stop
		SetActorRotation(FRotator(0.f, SpinFromYaw + A, 0.f));
		if (SpinChair.IsValid())
		{
			const FRotator R = SpinChair->GetActorRotation();
			SpinChair->SetActorRotation(FRotator(R.Pitch, SpinChairFromYaw + A, R.Roll), ETeleportType::TeleportPhysics);
		}
	}
	if (TheftHold > 0.f)
	{
		TheftHold -= DeltaSeconds;
		if (Player && State == ETakeCoworkerState::Working)   // the glare: their eyes follow you until they give up
		{
			const FRotator Glare = (Player->GetActorLocation() - GetActorLocation()).Rotation();
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), FRotator(0.f, Glare.Yaw, 0.f), DeltaSeconds, 6.f));
			FrownHold = FMath::Max(FrownHold, 0.5f);
		}
		if (TheftHold <= 0.f && State == ETakeCoworkerState::Working) { SetActorRotation(HomeRot); }   // fine. back to work.
	}
	if (bSkeletal) { GetMesh()->bPauseAnims = TheftHold > 0.f && State != ETakeCoworkerState::Watching; }   // hands off the keyboard
	if (State == ETakeCoworkerState::Working && !bWanders && !bAtParty && TheftHold <= 0.f && FrownHold <= 0.f && SpeechLeft <= 0.f
	    && !GetActorRotation().Equals(HomeRot, 1.f))
	{
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), HomeRot, DeltaSeconds, 2.f));   // back to the desk, back to work
	}
	SetFace(State == ETakeCoworkerState::Watching || FaceFearHold > 0.f);

	AAIController* AI = Cast<AAIController>(GetController());
	const float Fear = GetFear();
	// Hysteresis: once they're staring, Fear has to drop well below the line (and stay there) before they go back
	// to work, so a Fear value hovering around the threshold doesn't make them twitch between staring and walking.
	float Watch = bIsAgent ? WatchFear - 0.15f : WatchFear;
	if (State == ETakeCoworkerState::Watching) { Watch -= 0.2f; }
	if (Fear >= Watch) { WatchReleaseLeft = 2.5f; }
	else if (State == ETakeCoworkerState::Watching && WatchReleaseLeft > 0.f) { WatchReleaseLeft -= DeltaSeconds; Watch = -1.f; }

	// --- Augmented Fear: stop pretending ---------------------------------------
	if (Player && Fear >= Watch)
	{
		if (State != ETakeCoworkerState::Watching)
		{
			State = ETakeCoworkerState::Watching;
			SpokenLine.Reset();
			SpeechLeft = 0.f;
			Speech->SetText(FText::GetEmpty());
			StareSpot = GetActorLocation();
		}
		// Everyone stops, rises where they are and stares at you. They don't walk any more.
		// (Playtest: "in fear mode, if you throw things at your coworkers they follow you around": a hit, a push or
		// a leftover path could still slide them along. Now they're pinned where the staring began.)
		if (AI) { AI->StopMovement(); }
		GetCharacterMovement()->StopMovementImmediately();
		if (FVector::DistSquared2D(GetActorLocation(), StareSpot) > 4.f)
		{
			SetActorLocation(FVector(StareSpot.X, StareSpot.Y, GetActorLocation().Z), false, nullptr, ETeleportType::TeleportPhysics);
		}
		const FRotator Face = (Player->GetActorLocation() - GetActorLocation()).Rotation();
		// They turn to keep you in view, but slowly, the head first (a snap every frame read as "following")
		SetActorRotation(FMath::RInterpConstantTo(GetActorRotation(), FRotator(0.f, Face.Yaw, 0.f), DeltaSeconds, 60.f));   // it was always looking at you
		// Even more Fear: look away and they're gone. Look back: still gone.
		if (Fear >= VanishFear && !bVanished && !PlayerCanSee(Player))
		{
			bVanished = true;
			SetActorHiddenInGame(true);
			SetActorEnableCollision(false);
		}
		Animate(DeltaSeconds, false);
		return;
	}
	if (bVanished)   // Fear passed: back in their place as if nothing happened
	{
		bVanished = false;
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		if (bSitsAtDesk)   // seated people only block you, never the furniture
		{
			GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
			GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		}
		SetActorLocation(Home);
	}
	if (State == ETakeCoworkerState::Watching)   // fear dropped: go back to work as if nothing happened
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		if (bSitsAtDesk || !bWanders)
		{
			SetActorRotation(HomeRot);
			State = ETakeCoworkerState::Working;
			StateTimer = FMath::FRandRange(WorkMin, WorkMax);
		}
		else
		{
			GoTo(Home, ETakeCoworkerState::Returning);
		}
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
		bool bGaveUp = StuckTimer > 20.f;
		if (bGaveUp && !bArrived && Player && PlayerCanSee(Player) && FVector::Dist2D(Player->GetActorLocation(), GetActorLocation()) < 2500.f)
		{
			// Never pop home while you're looking (that read as "they disappeared after I talked to them"): try the path again
			bGaveUp = false;
			GoTo(Target, State);
			StuckTimer = 14.f;
		}
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

void ATakeCoworker::ReactToTheft()
{
	if (bVanished || IsHidden() || State == ETakeCoworkerState::Watching) { return; }
	const double Now = GetWorld()->GetTimeSeconds();
	if (Now - LastHitTime < 3.0) { return; }
	LastHitTime = Now;
	if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		const FRotator Face = (Player->GetActorLocation() - GetActorLocation()).Rotation();
		SetActorRotation(FRotator(0.f, Face.Yaw, 0.f));   // they stop and turn to look at you
	}
	FrownHold = 5.f;
	TheftHold = 6.f;
	CooldownLeft = 0.f;
	static const TCHAR* Snark[] = {
		TEXT("Excuse me. I was using that."), TEXT("That's company property. Specifically, mine."),
		TEXT("Wow. Okay. I'll just... work on nothing, then."), TEXT("Put. It. Back."),
		TEXT("I had seventeen tabs open on that."), TEXT("Is this a team-building exercise? Nobody told me."),
	};
	SayLine(Snark[FMath::RandRange(0, int32(UE_ARRAY_COUNT(Snark)) - 1)]);
}

void ATakeCoworker::StartMingling()
{
	// Walk around the party: to the cake, the kitchen, each other (random spots near where they stood)
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->SetMovementMode(MOVE_Walking);
	Move->GravityScale = 1.f;
	Move->MaxWalkSpeed = WalkSpeed * 0.8f;
	Move->bUseRVOAvoidance = true;   // they bumped into each other a lot
	Move->AvoidanceConsiderationRadius = 160.f;
	Spots.Reset();
	bWanders = true;
	State = ETakeCoworkerState::Working;
	StateTimer = FMath::FRandRange(1.f, 8.f);
}

void ATakeCoworker::JoinParty(const FVector& Where, const FVector& LookAt)
{
	bAtParty = true;
	bWanders = false;
	bTalksWhenNear = false;   // the speeches: nobody talks over them
	bVanished = false;
	FrownHold = FaceFearHold = 0.f;
	State = ETakeCoworkerState::Working;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	if (AAIController* AI = Cast<AAIController>(GetController())) { AI->StopMovement(); }
	SetActorLocation(Where, false, nullptr, ETeleportType::TeleportPhysics);
	const FRotator Face = (LookAt - Where).Rotation();
	SetActorRotation(FRotator(0.f, Face.Yaw, 0.f));
	Home = Where;
	HomeRot = GetActorRotation();
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);   // stand on the floor, not floating or sunk into it
	GetCharacterMovement()->GravityScale = 1.f;
	if (bSitsAtDesk)   // everyone stands for the wrap party
	{
		bSitsAtDesk = false;
		GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
	}
	// Everyone smiles at the party (playtest: "why is this one unhappy and the others are happy?" - security and the
	// custodian wear the neutral face all day; not tonight)
	if (UMaterialInterface* Smile = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Props/Materials/MI_Face_Calm.MI_Face_Calm"))) { FaceCalm = Smile; }
	FaceMode = -1;
	SetFace(false);
}

// Two coworkers walk into each other: one of them says something (only when you're close enough to hear it)
void ATakeCoworker::TickBumps(float DeltaSeconds, const APawn* Player)
{
	BumpCooldown = FMath::Max(0.f, BumpCooldown - DeltaSeconds);
	BumpCheckIn -= DeltaSeconds;
	if (BumpCheckIn > 0.f) { return; }
	BumpCheckIn = 0.25f;
	if (!Player || BumpCooldown > 0.f || IsHidden() || bVanished || State == ETakeCoworkerState::Watching) { return; }
	if (GetVelocity().SizeSquared2D() < 25.f * 25.f) { return; }   // the one walking says it
	if (FVector::DistSquared2D(Player->GetActorLocation(), GetActorLocation()) > 900.f * 900.f) { return; }
	if (GetWorld()->GetTimeSeconds() < NextAnyoneMayTalk) { return; }
	for (TActorIterator<ATakeCoworker> It(GetWorld()); It; ++It)
	{
		ATakeCoworker* Other = *It;
		if (Other == this || Other->IsHidden() || Other->bVanished) { continue; }
		if (FVector::DistSquared2D(Other->GetActorLocation(), GetActorLocation()) > 75.f * 75.f) { continue; }
		static const TCHAR* Bumps[] = {
			TEXT("Sorry. Sorry. After you."), TEXT("Same corridor, same time, every day."), TEXT("We have to stop meeting like this. We won't."),
			TEXT("Excuse me. ...Excuse me."), TEXT("Pardon. We rehearsed this part."), TEXT("Your left. No, your other left."),
			TEXT("Oh! I thought you were 4470."), TEXT("Careful, that's my mark."),
		};
		const FRotator Face = (Other->GetActorLocation() - GetActorLocation()).Rotation();
		SetActorRotation(FRotator(0.f, Face.Yaw, 0.f));
		SayLine(bAtParty ? FString(TEXT("Oops. Cake.")) : FString(Bumps[FMath::RandRange(0, int32(UE_ARRAY_COUNT(Bumps)) - 1)]));
		BumpCooldown = FMath::FRandRange(25.f, 45.f);
		Other->BumpCooldown = FMath::Max(Other->BumpCooldown, 15.f);   // (not both of them at once)
		return;
	}
}
