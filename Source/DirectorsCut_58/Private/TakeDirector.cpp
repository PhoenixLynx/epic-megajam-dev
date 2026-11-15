// TAKE 4471 (Director's Cut) - 2026 Epic AI MegaJam
// The two endings, after the wrap party's question "HAND OVER YOUR KEYS?"
//
//   YES (the theatrical cut): the elevator dings, C is lit. Up to the director's floor: a penthouse after the party,
//        half a cake, a wall of monitors playing your takes, an empty chair: DIRECTOR: ______. Sit (E) and your number
//        fills in; on the centre monitor someone new walks up to your old desk (a live camera on it), badge 4472.
//        "Ready when you are, Director." E: ACTION. Handing over your keys didn't free you. It promoted you.
//   NO  (the director's cut): the applause stops mid-clap, every face turns, the lights go red. From above: "CUT."
//        The walls lift away (they were flats on a soundstage), rigging and lights overhead. "...Reset. From the top."
//
// Everything up there is built here from engine shapes and the game's own props, so it needs no level edits.
#include "TakeGrabComponent.h"
#include "DirectorsCut_58.h"
#include "TakeGameInstance.h"
#include "TakeCoworker.h"
#include "TakeFearDirector.h"
#include "TakeStalker.h"
#include "TakeTypes.h"
#include "TakeCake.h"
#include "ProceduralMeshComponent.h"
#include "Components/AudioComponent.h"
#include "TakeWidgets.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TextRenderActor.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/TextRenderComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	// The director's floor: high above the lobby, open to the sky on the west side
	const FVector Penthouse(2100.f, 1050.f, 3000.f);   // centre of the floor's top surface
	const float PW = 1400.f, PD = 1000.f, PH = 380.f;   // width (x), depth (y), wall height
	const FVector PenthouseArrival(2650.f, 1300.f, 3095.f);   // by the east wall, where the elevator lets you out

	UStaticMesh* Mesh(const TCHAR* Path) { return LoadObject<UStaticMesh>(nullptr, Path); }
	UStaticMesh* Prop(const TCHAR* Name) { return LoadObject<UStaticMesh>(nullptr, *FString::Printf(TEXT("/Game/Props/Meshes/%s.%s"), Name, Name)); }
	UMaterialInterface* Mat(const TCHAR* Path) { return LoadObject<UMaterialInterface>(nullptr, Path); }
	// A monitor feed: Sol's dailies (ART-012, /Game/Ending) first, then the office screens
	UMaterialInterface* Feed(const TCHAR* Name)
	{
		if (UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, *FString::Printf(TEXT("/Game/Ending/%s.%s"), Name, Name))) { return M; }
		return LoadObject<UMaterialInterface>(nullptr, *FString::Printf(TEXT("/Game/Props/Materials/%s.%s"), Name, Name));
	}

	// Cake in hand (playtest: "employees should have cake in hands", "they should hold the cake and take bites too,
	// cake should be wedged sliced here also"): a real wedge of the cake (TakeCake) on a small white plate, one actor,
	// attached to the character's root (the capsule centre), held in front at hand height (HandZ cm above the floor)
	// and a little to the right. No collision, no shadow. bBitten = someone already started on it (a shorter wedge).
	ATakeCake* GivePlateOfCake(ACharacter* Who, float HandZ, bool bBitten)
	{
		if (!Who || !Who->GetRootComponent() || !Who->GetWorld() || Who->ActorHasTag(TEXT("HasCake"))) { return nullptr; }
		UStaticMesh* PlateCyl = Mesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		const float HalfH = Who->GetCapsuleComponent() ? Who->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 90.f;
		const FVector Hand(32.f, 14.f, HandZ - HalfH);   // relative to the capsule centre: front (+X), right (+Y)
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ATakeCake* Slice = Who->GetWorld()->SpawnActor<ATakeCake>(Who->GetActorLocation(), Who->GetActorRotation(), Params);
		if (!Slice) { return nullptr; }
		Slice->BuildSlice(bBitten ? 15.f : 19.f, bBitten ? 9.f : 11.f, 8, FMath::RandRange(0, 7));
		Slice->Cake->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Slice->Cake->SetCastShadow(false);
		if (PlateCyl)
		{
			UStaticMeshComponent* Plate = NewObject<UStaticMeshComponent>(Slice, TEXT("Plate"));
			Plate->SetStaticMesh(PlateCyl);
			Plate->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Plate->SetCastShadow(false);
			Plate->SetupAttachment(Slice->GetRootComponent());
			Plate->SetRelativeLocation(FVector(0.f, 0.f, -0.6f));
			Plate->SetRelativeScale3D(FVector(0.18f, 0.18f, 0.01f));
			if (UMaterialInterface* White = Mat(TEXT("/Game/Props/Meshes/M_PlasticWhite.M_PlasticWhite"))) { Plate->SetMaterial(0, White); }
			Plate->RegisterComponent();
		}
		Slice->AttachToComponent(Who->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		Slice->SetActorRelativeLocation(Hand + FVector(0.f, 0.f, 0.6f));
		Slice->SetActorRelativeRotation(FRotator(0.f, 25.f, 0.f));
		Slice->Tags.Add(TEXT("DirectorsFloor"));
		Who->Tags.AddUnique(TEXT("HasCake"));
		return Slice;
	}
}

AStaticMeshActor* UTakeGrabComponent::SpawnSetPiece(UStaticMesh* M, const FVector& At, const FRotator& Rot, const FVector& Scale, UMaterialInterface* Material, bool bCollide)
{
	if (!M) { return nullptr; }
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AStaticMeshActor* A = GetWorld()->SpawnActor<AStaticMeshActor>(At, Rot, Params);
	if (!A) { return nullptr; }
	UStaticMeshComponent* SM = A->GetStaticMeshComponent();
	SM->SetMobility(EComponentMobility::Movable);
	SM->SetStaticMesh(M);
	SM->SetWorldScale3D(Scale);
	if (Material) { SM->SetMaterial(0, Material); }
	SM->SetCollisionProfileName(bCollide ? TEXT("BlockAll") : TEXT("NoCollision"));
	SM->CanCharacterStepUpOn = ECB_No;
	A->Tags.Add(TEXT("DirectorsFloor"));
	return A;
}

static ATextRenderActor* SpawnLabel(UWorld* World, const FString& Text, const FVector& At, const FRotator& Rot, float Size, const FColor& Color)
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATextRenderActor* T = World->SpawnActor<ATextRenderActor>(At, Rot, Params);
	if (!T) { return nullptr; }
	UTextRenderComponent* TR = T->GetTextRender();
	TR->SetHorizontalAlignment(EHTA_Center);
	TR->SetVerticalAlignment(EVRTA_TextCenter);
	TR->SetWorldSize(Size);
	TR->SetTextRenderColor(Color);
	TR->SetText(FText::FromString(Text));
	return T;
}

// ------------------------------------------------------------------ the wrap party, downstairs
void UTakeGrabComponent::BuildLobbyParty()
{
	if (bLobbyPartyBuilt) { return; }
	bLobbyPartyBuilt = true;
	UWorld* World = GetWorld();
	UStaticMesh* Cube = Mesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Cyl = Mesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Ball = Mesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UMaterialInterface* Shape = Mat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	auto Tint = [&](const FLinearColor& C) -> UMaterialInterface*
	{
		if (!Shape) { return nullptr; }
		UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(Shape, this);
		M->SetVectorParameterValue(TEXT("Color"), C);
		return M;
	};
	UMaterialInterface* Mint = Tint(FLinearColor(0.32f, 0.78f, 0.64f));
	UMaterialInterface* White = Tint(FLinearColor(0.92f, 0.93f, 0.92f));
	UMaterialInterface* Gold = Tint(FLinearColor(0.85f, 0.66f, 0.25f));
	UMaterialInterface* Frosting = Mat(TEXT("/Game/Props/Meshes/M_Frosting.M_Frosting"));
	UMaterialInterface* FabricMint = Mat(TEXT("/Game/Props/Meshes/M_FabricMint.M_FabricMint"));

	// The cake table: east of the party, between it and the kitchen
	const FVector Table(2620.f, 1000.f, 0.f);
	SpawnSetPiece(Cube, Table + FVector(0.f, 0.f, 37.f), FRotator::ZeroRotator, FVector(1.8f, 0.9f, 0.74f), White, true);
	SpawnSetPiece(Cube, Table + FVector(0.f, 0.f, 75.f), FRotator::ZeroRotator, FVector(1.86f, 0.96f, 0.02f), Mint, false);   // tablecloth
	// The cake: the same one that sat in the kitchen all overtime ("for later"). Later is now. Whole, until you cut it.
	if (ATakeCake* OldCake = OvertimeCake.Get()) { OldCake->Destroy(); }
	if (ATakeCake* Cake = SpawnCake(Table + FVector(0.f, 0.f, 76.f), 0.f, 27.f, 15.f, 0, TEXT("Cake")))
	{
		Cake->Tags.Add(TEXT("PartyCake"));
	}
	(void)Frosting; (void)FabricMint;
	for (int32 I = 0; I < 6; ++I)   // cups and cans, a little everywhere
	{
		const FVector At = Table + FVector(-70.f + I * 28.f, (I % 2 == 0) ? -28.f : 30.f, 76.f);
		// real props (playtest: "coffee should still be slurpable at party"): pick a mug up, F to sip; cans too
		SpawnProp(I % 3 == 2 ? TEXT("SM_SodaCan") : TEXT("SM_Mug"), At + FVector(0.f, 0.f, 1.f), FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f));
	}
	SpawnSetPiece(Prop(TEXT("SM_CupStack")), Table + FVector(75.f, 0.f, 76.f), FRotator::ZeroRotator, FVector(1.f), nullptr, false);

	// The banner: over the elevator doors, on the lobby side
	if (UMaterialInterface* BannerArt = Mat(TEXT("/Game/Ending/MI_WrapBanner.MI_WrapBanner")))   // Sol's banner (ART-010), 4:1
	{
		const FVector BannerAt(2100.f, 905.f, 240.f);   // across the lobby, a step in front of the elevator (it covered the ELEVATOR sign)
		SpawnSetPiece(Cube, BannerAt, FRotator::ZeroRotator, FVector(2.4f, 0.02f, 0.6f), BannerArt, false);
		for (const float Side : { -110.f, 110.f })   // two strings up to the ceiling
		{
			SpawnSetPiece(Mesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder")), BannerAt + FVector(Side, 0.f, 42.f), FRotator::ZeroRotator, FVector(0.006f, 0.006f, 0.25f), White, false);
		}
	}
	else   // (not imported yet: 3D text)
	{
		SpawnLabel(World, TakeText::Fill(this, TEXT("THAT'S A WRAP, {EMP}!")), FVector(2000.f, 518.f, 272.f), FRotator(0.f, 90.f, 0.f), 34.f, FColor(70, 190, 160));
		SpawnSetPiece(Cube, FVector(2000.f, 512.f, 272.f), FRotator::ZeroRotator, FVector(4.2f, 0.02f, 0.5f), White, false);
	}

	// Bunting: two strings of little flags across the lobby, mint / white / gold
	for (int32 Row = 0; Row < 2; ++Row)
	{
		const FVector From(1650.f, 760.f + Row * 520.f, 285.f), To(3300.f, 760.f + Row * 520.f, 285.f);
		for (int32 I = 0; I < 22; ++I)
		{
			const float T = (I + 0.5f) / 22.f;
			const float Sag = 26.f * FMath::Sin(T * PI);
			const FVector At = FMath::Lerp(From, To, T) - FVector(0.f, 0.f, Sag);
			UMaterialInterface* C = (I % 3 == 0) ? Mint : (I % 3 == 1) ? White : Gold;
			SpawnSetPiece(Cube, At, FRotator(0.f, 0.f, 45.f), FVector(0.22f, 0.01f, 0.22f), C, false);
		}
	}

	// Balloons: clusters tied to the cake table and the reception desk, plus a few loose ones on the ceiling
	auto Balloon = [&](const FVector& Anchor, const FVector& Offset, UMaterialInterface* C)
	{
		const FVector At = Anchor + Offset;
		SpawnSetPiece(Ball, At, FRotator::ZeroRotator, FVector(0.32f, 0.32f, 0.38f), C, false);
		const float Len = FVector::Dist(Anchor, At - FVector(0.f, 0.f, 18.f));
		const FVector Mid = (Anchor + At - FVector(0.f, 0.f, 18.f)) * 0.5f;
		const FRotator Along = (At - Anchor).Rotation() + FRotator(-90.f, 0.f, 0.f);
		SpawnSetPiece(Cyl, Mid, Along, FVector(0.004f, 0.004f, Len / 100.f), White, false);   // the string
	};
	const FVector Knots[] = { Table + FVector(-85.f, 0.f, 76.f), Table + FVector(85.f, 0.f, 76.f), FVector(1960.f, 1330.f, 110.f), FVector(2240.f, 1330.f, 110.f) };
	int32 K = 0;
	for (const FVector& Knot : Knots)
	{
		for (int32 I = 0; I < 3; ++I, ++K)
		{
			const FVector Off(FMath::FRandRange(-30.f, 30.f), FMath::FRandRange(-30.f, 30.f), 120.f + I * 22.f);
			Balloon(Knot, Off, (K % 3 == 0) ? Mint : (K % 3 == 1) ? White : Gold);
		}
	}
	for (int32 I = 0; I < 7; ++I)
	{
		const FVector At(FMath::FRandRange(1750.f, 3200.f), FMath::FRandRange(700.f, 1350.f), 268.f);
		SpawnSetPiece(Ball, At, FRotator::ZeroRotator, FVector(0.32f, 0.32f, 0.38f), (I % 2 == 0) ? Mint : Gold, false);
	}
	// A warm light over the cake
	FActorSpawnParameters Params;
	if (APointLight* Lamp = World->SpawnActor<APointLight>(Table + FVector(0.f, 0.f, 230.f), FRotator::ZeroRotator, Params))
	{
		Lamp->PointLightComponent->SetMobility(EComponentMobility::Movable);
		Lamp->PointLightComponent->SetIntensity(3500.f);
		Lamp->PointLightComponent->SetAttenuationRadius(500.f);
		Lamp->PointLightComponent->SetLightColor(FLinearColor(1.f, 0.85f, 0.65f));
	}
	SpawnCollector();
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] The wrap party is dressed (lobby)"));
}

// ------------------------------------------------------------------ the Collector: a man in a top hat and a red tie
void UTakeGrabComponent::SpawnCollector()
{
	if (Collector.IsValid()) { return; }
	const FVector Where(2420.f, 740.f, 92.f);   // by the elevator, a little apart from everyone
	const FTransform T(FRotator(0.f, 120.f, 0.f), Where);
	ATakeCoworker* Col = GetWorld()->SpawnActorDeferred<ATakeCoworker>(ATakeCoworker::StaticClass(), T, nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (!Col) { return; }
	Col->NPCRole = TEXT("Collector");
	Col->bWanders = false;
	Col->bStoryNPC = true;
	Col->bTalksWhenNear = false;
	Col->Tags = { FName(TEXT("NPC")), FName(TEXT("Collector")) };
	if (UStaticMesh* Body = Prop(TEXT("SM_Mannequin"))) { Col->BodyMesh = Body; }
	Col->FinishSpawning(T);
	Col->JoinParty(Where, FVector(2100.f, 1050.f, 92.f));
	// A black suit, a top hat, a red tie; his skin the same dark as the suit (the Black coworkers use it too: TakeCoworker.cpp)
	UMaterialInterface* Black = Mat(TEXT("/Game/Props/Meshes/M_MetalDark.M_MetalDark"));
	UMaterialInterface* Red = Mat(TEXT("/Game/Props/Meshes/M_RedPaint.M_RedPaint"));
	TArray<UMeshComponent*> Meshes;
	Col->GetComponents<UMeshComponent>(Meshes);
	for (UMeshComponent* MC : Meshes)
	{
		for (int32 S = 0; S < MC->GetNumMaterials(); ++S)
		{
			const FName SlotName = MC->GetMaterialSlotNames().IsValidIndex(S) ? MC->GetMaterialSlotNames()[S] : NAME_None;
			if (SlotName.ToString().Contains(TEXT("Face"))) { continue; }
			if (Black) { MC->SetMaterial(S, Black); }
		}
	}
	UStaticMesh* Cyl = Mesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Cube = Mesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	auto Wear = [&](UStaticMesh* M, const FVector& Rel, const FVector& Scale, UMaterialInterface* Mtl)
	{
		UStaticMeshComponent* Piece = NewObject<UStaticMeshComponent>(Col);
		Piece->SetStaticMesh(M);
		Piece->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Piece->SetupAttachment(Col->GetRootComponent());
		Piece->SetRelativeLocation(Rel);
		Piece->SetRelativeScale3D(Scale);
		if (Mtl) { Piece->SetMaterial(0, Mtl); }
		Piece->RegisterComponent();
	};
	Wear(Cyl, FVector(0.f, 0.f, 99.f), FVector(0.36f, 0.36f, 0.015f), Black);    // the brim
	Wear(Cyl, FVector(0.f, 0.f, 112.f), FVector(0.24f, 0.24f, 0.26f), Black);    // the crown
	Wear(Cyl, FVector(0.f, 0.f, 103.f), FVector(0.245f, 0.245f, 0.03f), Red);    // the band
	Wear(Cube, FVector(15.f, 0.f, 42.f), FVector(0.015f, 0.07f, 0.34f), Red);    // the tie (on his front: the way he faces)
	Collector = Col;
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] The Collector is at the party (top hat, red tie)"));
}

void UTakeGrabComponent::CutTheCake(AActor* Cake)
{
	// The first slice at the party: now that it's cut, about half the people mingling are holding a plate
	// (every other one, so it looks the same each take)
	if (!Cake) { return; }
	int32 Plates = 0, Seen = 0;
	for (TActorIterator<ATakeCoworker> CakeIt(GetWorld()); CakeIt; ++CakeIt)
	{
		if (!CakeIt->bAtParty || CakeIt->ActorHasTag(TEXT("Collector")) || CakeIt->IsHidden()) { continue; }
		if ((Seen++ % 2) != 0) { continue; }
		if (ATakeCake* Held = GivePlateOfCake(*CakeIt, 102.f, Plates % 3 == 1))   // some have had a bite
		{
			FHeldCake H;
			H.Slice = Held;
			H.Rel = Held->GetRootComponent()->GetRelativeLocation();
			H.Next = FMath::FRandRange(2.f, 9.f);
			HeldCakes.Add(H);
		}
		++Plates;
	}
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Cake cut: %d coworker(s) holding a plate"), Plates);
}

// People at the party eat their cake: now and then the plate comes up to the face, a bite, back down. Four bites and it's gone.
void UTakeGrabComponent::TickHeldCakes(float DeltaTime)
{
	if (bRefused) { return; }   // (nobody eats while they're staring at you)
	for (FHeldCake& H : HeldCakes)
	{
		ATakeCake* S = Cast<ATakeCake>(H.Slice.Get());
		if (!S || H.Bites >= 4) { continue; }
		if (H.Lift < 0.f)
		{
			if ((H.Next -= DeltaTime) <= 0.f) { H.Lift = 0.f; H.bBit = false; }
			continue;
		}
		H.Lift += DeltaTime;
		const float T = FMath::Clamp(H.Lift / 1.6f, 0.f, 1.f);
		const float Up = FMath::Sin(PI * T);
		S->SetActorRelativeLocation(H.Rel + FVector(-18.f * Up, -10.f * Up, 54.f * Up));   // hand to mouth
		if (!H.bBit && T >= 0.5f)
		{
			H.bBit = true;
			++H.Bites;
			const float Left = 1.f - 0.2f * H.Bites;   // the wedge gets smaller (the plate doesn't)
			S->Cake->SetRelativeScale3D(FVector(Left, Left, FMath::Max(0.5f, Left)));
			if (UStaticMeshComponent* Plate = S->FindComponentByClass<UStaticMeshComponent>())
			{
				Plate->SetRelativeScale3D(FVector(0.18f / Left, 0.18f / Left, 0.01f / FMath::Max(0.5f, Left)));
				Plate->SetRelativeLocation(FVector(0.f, 0.f, -0.6f / FMath::Max(0.5f, Left)));
			}
			if (H.Bites >= 4) { S->Cake->SetVisibility(false); }   // just the plate (and crumbs)
			if (const AActor* Me = GetOwner())
			{
				if (FVector::DistSquared(Me->GetActorLocation(), S->GetActorLocation()) < 400.f * 400.f) { PlaySfx(TEXT("sfx_eat_crunch"), S->GetActorLocation(), 0.25f); }
			}
		}
		if (T >= 1.f)
		{
			H.Lift = -1.f;
			H.Next = FMath::FRandRange(5.f, 12.f);
			S->SetActorRelativeLocation(H.Rel);
		}
	}
}

// ------------------------------------------------------------------ the cake you can cut (TakeCake)
ATakeCake* UTakeGrabComponent::SpawnCake(const FVector& Base, float Yaw, float Radius, float Height, int32 Missing, const TCHAR* Key)
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATakeCake* Cake = GetWorld()->SpawnActor<ATakeCake>(Base, FRotator(0.f, Yaw, 0.f), Params);
	if (!Cake) { return nullptr; }
	Cake->BuildCake(Radius, Height, 8, Missing);
	if (Key) { Cake->Tags.Add(FName(*FString::Printf(TEXT("Use=%s"), Key))); }
	Cake->Tags.Add(TEXT("DirectorsFloor"));   // (the refusal's set-lift leaves it alone)
	return Cake;
}

void UTakeGrabComponent::UseCake(ATakeCake* Cake, int32 N)
{
	if (!Cake) { return; }
	if (ATakeCake* Waiting = PlatedSlice.Get())   // playtest: "can't take a new slice until you eat the previous one"
	{
		if (!Waiting->IsHidden())
		{
			ShowSubtitle(TEXT(""), TEXT("(Your slice is right there on the plate. Finish it first.)"), 2.5f);
			return;
		}
	}
	const FVector C = Cake->GetActorLocation();
	int32 Wedge = 0;
	FVector Dir = FVector::ForwardVector;
	if (!Cake->CutSlice(Wedge, Dir))
	{
		ShowSubtitle(TEXT(""), TEXT("(Crumbs, a knife, and a smear of mint frosting that used to say 4471.)"), 3.5f);
		return;
	}
	PlaySfx(TEXT("sfx_pickup"), C + FVector(0.f, 0.f, 20.f), 0.45f);
	// The slice goes on a plate just past the cake, on the side you're standing (always on the table)
	FVector ToMe = FVector::ForwardVector;
	if (const AActor* Me = GetOwner()) { ToMe = (Me->GetActorLocation() - C).GetSafeNormal2D(); }
	if (ToMe.IsNearlyZero()) { ToMe = FVector::ForwardVector; }
	const FVector Side(-ToMe.Y, ToMe.X, 0.f);
	const int32 Cuts = Cake->CutsSoFar() - 1;   // 0 for the first slice off this cake
	const float Spread = (Cuts % 3 - 1) * 24.f;   // a second plate doesn't land on the first
	const FVector PlateAt = C + ToMe * (Cake->Radius + 14.f) + Side * Spread;
	UMaterialInterface* White = Mat(TEXT("/Game/Props/Meshes/M_PlasticWhite.M_PlasticWhite"));
	SpawnSetPiece(Mesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder")), PlateAt + FVector(0.f, 0.f, 0.5f), FRotator::ZeroRotator, FVector(0.2f, 0.2f, 0.01f), White, false);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (ATakeCake* Slice = GetWorld()->SpawnActor<ATakeCake>(PlateAt + FVector(0.f, 0.f, 1.1f), Cake->GetActorRotation(), Params))
	{
		Slice->BuildSlice(Cake->Radius, Cake->Height, Cake->Slices, Wedge);
		Slice->Tags.Add(TEXT("Use=CakeSlice"));
		PlatedSlice = Slice;
		Slice->Tags.Add(TEXT("DirectorsFloor"));
	}
	const bool bPenthouse = bPenthouseBuilt && C.Z > 2000.f;
	if (Cake->ActorHasTag(TEXT("PartyCake")) && !bPartyCakeCut) { bPartyCakeCut = true; CutTheCake(Cake); }
	static const TCHAR* PartyCuts[] = {
		TEXT("You cut a slice. The knife goes straight through the 4471."),
		TEXT("Another slice. The cake is getting smaller. Something here finally is."),
		TEXT("Half a cake left. Nobody else has cut any. They're waiting for you to finish it."),
		TEXT("You cut another. The frosting says THAT'S A W."),
	};
	static const TCHAR* PenthouseCuts[] = {
		TEXT("The director's cake. Someone saved the director a slice. You cut another anyway."),
		TEXT("It's still cold. It was always going to be here, waiting for whoever sat in that chair."),
		TEXT("Another slice. Up here, nobody counts."),
	};
	const int32 Pick = FMath::Min(Cuts, bPenthouse ? 2 : 3);
	(void)N;
	ShowSubtitle(TEXT(""), bPenthouse ? PenthouseCuts[Pick] : PartyCuts[Pick], 3.5f);
	QueueSubtitle(TEXT(""), TEXT("(E: eat it.)"));
}

void UTakeGrabComponent::EatSlice(AActor* Slice)
{
	if (!Slice) { return; }
	PlaySfx(TEXT("sfx_eat_crunch"), Slice->GetActorLocation(), 0.7f);
	if (UTakeGameInstance* CakeGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { CakeGI->AddEnergy(25.f); }
	if (ATakeFearDirector* CakeFear = FindFearDirector()) { if (CakeFear->Fear > 0.f && !bRefused) { CakeFear->StartCreep(FMath::Max(0.f, CakeFear->TargetFear - 0.1f), 2.f); } }
	static const TCHAR* Bites[] = {
		TEXT("Mint frosting. It tastes like the first day of something."),
		TEXT("Exactly as good as the first one. Exactly."),
		TEXT("You've had a lot of cake, Employee {EMP}. HR has been notified. HR has also had cake."),
		TEXT("It's the best cake you've ever had. It's the only cake you've ever had."),
	};
	ShowSubtitle(TEXT(""), TakeText::Fill(this, Bites[FMath::Min(SlicesEaten, int32(UE_ARRAY_COUNT(Bites)) - 1)]), 3.5f);
	++SlicesEaten;
	bAteCake = true;
	Slice->Destroy();   // the plate stays, with crumbs
}

void UTakeGrabComponent::ShowScreenPeek(UTexture2D* Tex, const FString& Title, const FString& Caption)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC || Inspecting) { return; }
	Inspecting = CreateWidget<UTakeInspectWidget>(PC, UTakeInspectWidget::StaticClass());
	if (!Inspecting) { return; }
	Drop();
	Inspecting->Setup(Tex, Title, Caption);
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

void UTakeGrabComponent::StartMingle()
{
	EndStep = 30;
	MingleT = 0.f;
	PlaySfx2D(TEXT("sfx_pa_chime"), 0.6f);
	ShowSubtitle(TEXT("PA"), TEXT("Cake is served. Mingle. You've all earned it."), 4.f);
	if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { GI->SetObjective(TEXT("Mingle. (Cake's by the kitchen.)")); }
	for (TActorIterator<ATakeCoworker> It(GetWorld()); It; ++It)
	{
		if (It->bAtParty && !It->ActorHasTag(TEXT("Collector"))) { It->StartMingling(); }
	}
}

void UTakeGrabComponent::OnPartyChat()
{
	++PartyChats;
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Party: talked to %d coworker(s)"), PartyChats);
}

void UTakeGrabComponent::OnCollectorTalk(ATakeCoworker* Who)
{
	if (!Who) { return; }
	if (EndStep != 30 || !bCollectorReady)
	{
		static const TCHAR* NotYet[] = {
			TEXT("Not yet, {EMP}. Have some cake. Talk to people. They've been looking forward to this."),
			TEXT("Enjoy it. Parties are the only scenes nobody rehearses."),
			TEXT("I'll find you. I always find you."),
		};
		Who->SayLine(NotYet[FMath::RandRange(0, int32(UE_ARRAY_COUNT(NotYet)) - 1)]);
		return;
	}
	Who->SayLine(TEXT("Lovely party. You've earned it. Now, about your keys."));
	EndStep = 31;   // TickAct3 opens the question
}

// ------------------------------------------------------------------ the director's floor
void UTakeGrabComponent::BuildPenthouse()
{
	if (bPenthouseBuilt) { return; }
	bPenthouseBuilt = true;
	UWorld* World = GetWorld();
	UStaticMesh* Cube = Mesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Cyl = Mesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UMaterialInterface* Wood = Mat(TEXT("/Game/Props/Meshes/M_Wood.M_Wood"));
	UMaterialInterface* Walls = Mat(TEXT("/Game/Fear/MI_Greybox_Walls.MI_Greybox_Walls"));
	UMaterialInterface* Steel = Mat(TEXT("/Game/Props/Materials/M_BrushedSteel.M_BrushedSteel"));
	UMaterialInterface* Frosting = Mat(TEXT("/Game/Props/Meshes/M_Frosting.M_Frosting"));
	UMaterialInterface* Fabric = Mat(TEXT("/Game/Props/Meshes/M_FabricMint.M_FabricMint"));
	const FVector P = Penthouse;
	auto Box = [&](const FVector& Centre, const FVector& SizeCm, UMaterialInterface* M, bool bCollide = true)
	{
		return SpawnSetPiece(Cube, Centre, FRotator::ZeroRotator, SizeCm / 100.f, M, bCollide);
	};

	// Floor, walls (north = the monitor wall, south, east), a low glass rail on the open west side, a ceiling over the back half
	Box(P - FVector(0.f, 0.f, 10.f), FVector(PW, PD, 20.f), Wood);
	Box(P + FVector(0.f, -PD * 0.5f, PH * 0.5f), FVector(PW, 20.f, PH), Walls);
	Box(P + FVector(0.f, PD * 0.5f, PH * 0.5f), FVector(PW, 20.f, PH), Walls);
	Box(P + FVector(PW * 0.5f, 0.f, PH * 0.5f), FVector(20.f, PD, PH), Walls);
	Box(P + FVector(-PW * 0.5f, 0.f, 55.f), FVector(8.f, PD, 110.f), Mat(TEXT("/Game/Props/Meshes/M_Glass.M_Glass")));
	Box(P + FVector(-PW * 0.5f, 0.f, 112.f), FVector(10.f, PD, 6.f), Steel);
	Box(P + FVector(PW * 0.15f, 0.f, PH + 5.f), FVector(PW * 0.7f, PD, 10.f), Mat(TEXT("/Game/Fear/M_CeilingPanel.M_CeilingPanel")), false);
	// The city at dusk (house-style ART-014) as a framed print on the east wall (playtest: "artwork can go on this blank
	// wall"), away from the elevator; outside, past the open west side, the city is built from simple blocks instead
	if (UMaterialInterface* Sky = Mat(TEXT("/Game/Ending/MI_PenthouseSkyline.MI_PenthouseSkyline")))
	{
		const FVector Print(P.X + PW * 0.5f - 12.f, P.Y - 170.f, PH * 0.5f + P.Z + 10.f);
		SpawnSetPiece(Cube, Print + FVector(2.f, 0.f, 0.f), FRotator(0.f, 90.f, 0.f), FVector(5.4f, 0.03f, 1.5f), Mat(TEXT("/Game/Props/Meshes/M_MetalDark.M_MetalDark")), false);   // the frame
		UMaterialInstanceDynamic* Lit = UMaterialInstanceDynamic::Create(Sky, this);
		Lit->SetScalarParameterValue(TEXT("Glow"), 0.35f);   // a print, not a screen
		SpawnSetPiece(Cube, Print - FVector(0.5f, 0.f, 0.f), FRotator(0.f, 90.f, 0.f), FVector(5.f, 0.03f, 1.25f), Lit, false);   // (+Y face, unmirrored, into the room)
	}
	{
		FRandomStream City(2214);
		auto Shade = [&](const FLinearColor& C) -> UMaterialInterface*
		{
			UMaterialInterface* Base = Mat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
			if (!Base) { return nullptr; }
			UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(Base, this);
			M->SetVectorParameterValue(TEXT("Color"), C);
			return M;
		};
		UMaterialInterface* Blocks[] = { Shade(FLinearColor(0.10f, 0.15f, 0.18f)), Shade(FLinearColor(0.16f, 0.22f, 0.26f)), Shade(FLinearColor(0.24f, 0.31f, 0.35f)) };
		for (int32 I = 0; I < 26; ++I)
		{
			const float Dist = City.FRandRange(2600.f, 7000.f);   // (well clear of the office building, which ends at x 0)
			const float W = City.FRandRange(300.f, 900.f), D = City.FRandRange(300.f, 900.f);
			const float Top = City.FRandRange(1600.f, 4300.f) * (Dist > 4500.f ? 1.15f : 0.9f);
			const FVector At(P.X - PW * 0.5f - Dist, P.Y + City.FRandRange(-4500.f, 4500.f), (Top - 300.f) * 0.5f);
			SpawnSetPiece(Cube, At, FRotator(0.f, City.FRandRange(-8.f, 8.f), 0.f), FVector(W, D, Top + 300.f) / 100.f, Blocks[(I + (Dist > 4500.f ? 2 : 0)) % 3], false);
		}
		// HALCYON HQ: the tallest, a mint band at the top
		const FVector HQ(P.X - PW * 0.5f - 3800.f, P.Y + 900.f, 0.f);
		SpawnSetPiece(Cube, HQ + FVector(0.f, 0.f, 2600.f), FRotator::ZeroRotator, FVector(7.f, 7.f, 52.f), Blocks[0], false);
		SpawnSetPiece(Cube, HQ + FVector(0.f, 0.f, 5150.f), FRotator::ZeroRotator, FVector(7.1f, 7.1f, 0.6f), Shade(FLinearColor(0.32f, 0.78f, 0.64f)), false);
	}
	// A rug under the chair
	Box(P + FVector(0.f, 40.f, 1.f), FVector(520.f, 380.f, 2.f), Mat(TEXT("/Game/Props/Materials/MI_Rug.MI_Rug")), false);

	// The monitor wall: two rows of three, the dailies. Bottom middle is live (set up when you sit).
	const bool bSolDailies = Feed(TEXT("MI_Dailies_Desk")) != nullptr && Feed(TEXT("MI_Dailies_Desk"))->GetPathName().StartsWith(TEXT("/Game/Ending"));
	static const TCHAR* OfficeFeeds[] = { TEXT("MI_Screen_CCTV"), TEXT("MI_Screen_Call"), TEXT("MI_Screen_Chart"),
	                                      TEXT("MI_Screen_Fear"), nullptr, TEXT("MI_Screen_Promo") };
	static const TCHAR* SolFeeds[] = { TEXT("MI_Dailies_Desk"), TEXT("MI_Dailies_Conference"), TEXT("MI_Dailies_CorridorB"),
	                                   TEXT("MI_Dailies_Ward"), nullptr, TEXT("MI_Screen_CCTV") };
	const TCHAR* const* Feeds = bSolDailies ? SolFeeds : OfficeFeeds;
	for (int32 I = 0; I < 6; ++I)
	{
		const int32 Col = I % 3, Row = I / 3;
		const FVector At = P + FVector(-260.f + Col * 260.f, -PD * 0.5f + 40.f, Row == 0 ? 200.f : 70.f);
		AStaticMeshActor* Mon = SpawnSetPiece(Prop(TEXT("SM_Monitor")), At, FRotator(0.f, 0.f, 0.f), FVector(2.4f), nullptr, false);   // screens toward the chair
		if (!Mon) { continue; }
		UStaticMeshComponent* SM = Mon->GetStaticMeshComponent();
		const int32 Slot = SM->GetMaterialIndex(FName(TEXT("M_Screen")));
		if (Slot == INDEX_NONE) { continue; }
		if (Feeds[I])
		{
			if (UMaterialInterface* FeedMat = Feed(Feeds[I])) { SM->SetMaterial(Slot, FeedMat); }
		}
		else { LiveMonitor = SM; LiveMonitorSlot = Slot; continue; }
		FScreen Daily;   // the others keep cutting between takes (TickDailies)
		Daily.Comp = SM;
		Daily.Slot = Slot;
		Daily.Next = FMath::FRandRange(1.f, 3.f);
		Dailies.Add(Daily);
	}
	SpawnLabel(World, TakeText::Fill(this, TEXT("DAILIES  \u00B7  TAKES 1 - {EMP}")), P + FVector(0.f, -PD * 0.5f + 14.f, PH - 40.f), FRotator(0.f, 90.f, 0.f), 22.f, FColor(150, 225, 205));

	// The chair: empty, facing the monitors. Its name card is blank until you sit.
	if (AStaticMeshActor* Chair = SpawnSetPiece(Prop(TEXT("SM_DirectorsChair")), P + FVector(0.f, 60.f, 0.f), FRotator(0.f, 180.f, 0.f), FVector(1.f), nullptr, true))
	{
		Chair->Tags.Add(TEXT("Use=DirectorsChair"));
		DirectorChair = Chair;
	}
	// Sol's name card (ART-013) on the chair back: blank until you sit (her "4471" card is used when you are 4471)
	if (UMaterialInterface* CardArt = Mat(TEXT("/Game/Ending/MI_DirectorCard_Blank.MI_DirectorCard_Blank")))
	{
		if (AStaticMeshActor* Card = SpawnSetPiece(Cube, P + FVector(0.f, 106.f, 118.f), FRotator::ZeroRotator, FVector(0.6f, 0.015f, 0.15f), CardArt, false))
		{
			DirectorCard = Card->GetStaticMeshComponent();
		}
	}
	if (ATextRenderActor* Name = SpawnLabel(World, DirectorCard.IsValid() ? FString() : FString(TEXT("DIRECTOR: ______")), P + FVector(0.f, 112.f, 118.f), FRotator(0.f, 90.f, 0.f), 13.f, FColor(245, 245, 240)))
	{
		ChairName = Name->GetTextRender();
	}

	// After the party: the cake (half gone), cups, a banner, a sofa nobody sat on
	const FVector Table = P + FVector(430.f, 300.f, 0.f);
	Box(Table + FVector(0.f, 0.f, 37.f), FVector(160.f, 90.f, 74.f), Mat(TEXT("/Game/Props/Meshes/M_PlasticWhite.M_PlasticWhite")));
	// The director's cake: three slices already gone. You can cut another (playtest: "I should be able to cut another cake slice")
	SpawnCake(Table + FVector(-25.f, 0.f, 74.f), 180.f, 27.f, 15.f, 3, TEXT("Cake"));
	(void)Frosting; (void)Fabric;
	SpawnSetPiece(Prop(TEXT("SM_CupStack")), Table + FVector(-68.f, -30.f, 74.f), FRotator::ZeroRotator, FVector(1.f), nullptr, false);
	SpawnSetPiece(Prop(TEXT("SM_SodaCan")), Table + FVector(-68.f, 30.f, 74.f), FRotator::ZeroRotator, FVector(1.f), nullptr, false);
	SpawnSetPiece(Prop(TEXT("SM_LobbySofa")), P + FVector(430.f, -250.f, 0.f), FRotator(0.f, -90.f, 0.f), FVector(1.f), nullptr, true);
	SpawnSetPiece(Prop(TEXT("SM_PlantTall")), P + FVector(640.f, -440.f, 0.f), FRotator::ZeroRotator, FVector(1.f), nullptr, true);
	SpawnSetPiece(Prop(TEXT("SM_PlantTall")), P + FVector(640.f, 440.f, 0.f), FRotator::ZeroRotator, FVector(1.f), nullptr, true);
	for (int32 I = 0; I < 18; ++I)   // confetti that already fell
	{
		const FVector At = P + FVector(FMath::FRandRange(-500.f, 600.f), FMath::FRandRange(-350.f, 420.f), 1.5f);
		SpawnSetPiece(Prop(I % 3 == 0 ? TEXT("SM_Brochure") : TEXT("SM_PaperBall")), At, FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f), FVector(1.f), nullptr, false);
	}
	SpawnLabel(World, TEXT("THAT'S A WRAP!"), P + FVector(0.f, PD * 0.5f - 14.f, PH - 60.f), FRotator(0.f, -90.f, 0.f), 70.f, FColor(110, 215, 185));

	// Light: warm practicals inside; the sky does the rest
	for (const FVector& L : { P + FVector(-300.f, 0.f, PH - 30.f), P + FVector(300.f, 0.f, PH - 30.f), P + FVector(450.f, 300.f, 200.f) })
	{
		FActorSpawnParameters Params;
		if (APointLight* Lamp = World->SpawnActor<APointLight>(L, FRotator::ZeroRotator, Params))
		{
			Lamp->PointLightComponent->SetMobility(EComponentMobility::Movable);
			Lamp->PointLightComponent->SetIntensity(5000.f);
			Lamp->PointLightComponent->SetAttenuationRadius(900.f);
			Lamp->PointLightComponent->SetLightColor(FLinearColor(1.f, 0.86f, 0.7f));
		}
	}

	// Downstairs, at your desk: someone new. A camera on them, for the live monitor.
	AActor* MyMonitor = nullptr;
	for (TActorIterator<AActor> It(World); It; ++It) { if (It->ActorHasTag(TEXT("Monitor4471"))) { MyMonitor = *It; break; } }
	if (MyMonitor)
	{
		AActor* Chair = nullptr;
		float Best = 220.f;
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			const UStaticMeshComponent* SM = It->GetStaticMeshComponent();
			if (!SM || !SM->GetStaticMesh() || !SM->GetStaticMesh()->GetName().Contains(TEXT("OfficeChair"))) { continue; }
			const float D = FVector::Dist2D(It->GetActorLocation(), MyMonitor->GetActorLocation());
			if (D < Best) { Best = D; Chair = *It; }
		}
		const FVector Seat = Chair ? Chair->GetActorLocation() : MyMonitor->GetActorLocation() + FVector(0.f, 120.f, 0.f);
		const FVector Face = (MyMonitor->GetActorLocation() - Seat).GetSafeNormal2D();
		const FVector Stand(Seat.X - Face.X * 25.f, Seat.Y - Face.Y * 25.f, Seat.Z - 2.f);
		if (AStaticMeshActor* New = SpawnSetPiece(Prop(TEXT("SM_Mannequin")), Stand, FRotator(0.f, Face.Rotation().Yaw - 90.f, 0.f), FVector(1.f), nullptr, false))
		{
			New->SetActorHiddenInGame(true);   // they arrive when you sit down
			Employee4472 = New;
			if (ATextRenderActor* Badge = SpawnLabel(World, TakeText::Fill(this, TEXT("{NEXT}")), Stand + FVector(0.f, 0.f, 210.f), FRotator(0.f, Face.Rotation().Yaw + 180.f, 0.f), 26.f, FColor(110, 215, 185)))
			{
				Badge->SetActorHiddenInGame(true);
				Badge4472 = Badge;
			}
		}
		// The camera: over their shoulder, the way the security cameras see you
		const FVector CamAt = Stand - Face * 230.f + FVector(60.f, 40.f, 200.f);
		if (ASceneCapture2D* Cam = World->SpawnActor<ASceneCapture2D>(CamAt, (Stand + FVector(0.f, 0.f, 110.f) - CamAt).Rotation()))
		{
			UTextureRenderTarget2D* RT = NewObject<UTextureRenderTarget2D>(this);
			RT->InitAutoFormat(640, 360);
			USceneCaptureComponent2D* Cap = Cam->GetCaptureComponent2D();
			Cap->TextureTarget = RT;
			Cap->FOVAngle = 70.f;
			Cap->bCaptureEveryFrame = false;
			Cap->bCaptureOnMovement = false;
			DailiesCam = Cap;
			DailiesTarget = RT;
		}
	}
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] The director's floor is built (%s)"), MyMonitor ? TEXT("with a live feed of desk 4471") : TEXT("no desk 4471 found: no live feed"));
}

void UTakeGrabComponent::TickDailies(float DeltaTime)
{
	static const TCHAR* Feeds[] = { TEXT("MI_Dailies_Desk"), TEXT("MI_Dailies_Conference"), TEXT("MI_Dailies_CorridorB"), TEXT("MI_Dailies_Ward"),
	                                TEXT("MI_Dailies_Desk"), TEXT("MI_Dailies_CorridorB"),   // (Sol's takes come up most)
	                                TEXT("MI_Screen_CCTV"), TEXT("MI_Screen_Call"), TEXT("MI_Screen_Fear"), TEXT("MI_Screen_Glyphs"), TEXT("MI_Screen_Code") };
	for (FScreen& S : Dailies)
	{
		UStaticMeshComponent* SM = S.Comp.Get();
		if (!SM || (S.Next -= DeltaTime) > 0.f) { continue; }
		S.Next = FMath::FRandRange(1.2f, 3.5f);
		const TCHAR* Pick = Feeds[FMath::RandRange(0, int32(UE_ARRAY_COUNT(Feeds)) - 1)];
		if (UMaterialInterface* FeedMat = Feed(Pick)) { SM->SetMaterial(S.Slot, FeedMat); }
	}
}

void UTakeGrabComponent::GoToPenthouse()
{
	if (EndStep != 11) { return; }
	BuildPenthouse();
	EndStep = 12;
	PlaySfx2D(TEXT("sfx_badge_ok"), 0.8f);
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{
			if (PC->PlayerCameraManager) { PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, 2.f, FLinearColor::Black, false, false); }
		}
	}
	SubQueue.Reset();   // whatever was still being said downstairs stays downstairs
	SubQueueWho.Reset();
	SubtitleLeft = 0.f;
	TeleportPlayer(PenthouseArrival, -160.f);
	// Playtest: "happy music in penthouse please" (the party's cue carries on up here, or starts if it had stopped)
	if (!(MusicAudio && MusicAudio->IsPlaying() && !bMusicFading && MusicTrack == TEXT("mus_halcyon_positive")))
	{
		PlayMusicCue(TEXT("the director's floor"), TEXT("mus_halcyon_positive"), true);
	}
	bMusicLoop = true;
	ShowTimeCard(TEXT("??:??"), TEXT("SCENE 7  \u00B7  C"));
	if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { GI->SetObjective(TEXT("The director's chair.")); }
	QueueSubtitle(TEXT(""), TEXT("(The doors open on the top floor. The party happened up here too. Everyone's gone.)"));
	QueueSubtitle(TEXT(""), TEXT("(A wall of screens. All of them are you.)"));
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Ending: the director's floor"));
}

void UTakeGrabComponent::SitDirector()
{
	if (EndStep != 12) { return; }
	EndStep = 13;
	AStaticMeshActor* Chair = Cast<AStaticMeshActor>(DirectorChair.Get());
	if (Chair) { Chair->SetActorEnableCollision(false); }
	const FVector Seat = (Chair ? Chair->GetActorLocation() : Penthouse) + FVector(0.f, 10.f, 92.f);
	TeleportPlayer(Seat, -90.f);   // facing the monitors
	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController())) { PC->SetIgnoreMoveInput(true); }
	}
	const UTakeGameInstance* CardGI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	UMaterialInterface* Named = Mat(TEXT("/Game/Ending/MI_DirectorCard_4471.MI_DirectorCard_4471"));
	UStaticMeshComponent* CardSM = DirectorCard.Get();
	if (CardSM && Named && CardGI && CardGI->EmployeeNumber == 4471) { CardSM->SetMaterial(0, Named); }   // Sol's card says 4471
	else
	{
		if (CardSM) { CardSM->SetVisibility(false); }   // a later take: the number is yours, not 4471
		if (UTextRenderComponent* Name = ChairName.Get()) { Name->SetText(FText::FromString(TakeText::Fill(this, TEXT("DIRECTOR: {EMP}")))); }
	}
	// The live monitor: your desk, now
	if (AActor* New = Employee4472.Get()) { New->SetActorHiddenInGame(false); }
	if (AActor* Badge = Badge4472.Get()) { Badge->SetActorHiddenInGame(false); }
	if (USceneCaptureComponent2D* Cap = DailiesCam.Get()) { Cap->bCaptureEveryFrame = true; }
	if (UStaticMeshComponent* Mon = LiveMonitor.Get())
	{
		UMaterialInterface* Base = Mat(TEXT("/Game/Props/Materials/M_ScreenTexture.M_ScreenTexture"));
		if (Base && DailiesTarget)
		{
			UMaterialInstanceDynamic* Live = UMaterialInstanceDynamic::Create(Base, this);
			Live->SetTextureParameterValue(TEXT("Tex"), DailiesTarget);
			Mon->SetMaterial(LiveMonitorSlot, Live);
		}
	}
	PlaySfx2D(TEXT("sfx_pickup"), 0.5f);
	ShowSubtitle(TEXT(""), TakeText::Fill(this, TEXT("(You sit. The name card says DIRECTOR: {EMP}. It always did.)")), 4.f);
	QueueSubtitle(TEXT(""), TakeText::Fill(this, TEXT("(The centre screen: your desk, downstairs. Someone new walks up to it. Their badge says {NEXT}.)")));
	QueueSubtitle(TEXT("PA"), TEXT("Ready when you are, Director."));
	QueueSubtitle(TEXT(""), TEXT("(E: call it.)"));
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Ending: in the director's chair"));
}

void UTakeGrabComponent::CallAction()
{
	if (EndStep != 13) { return; }
	PlaySfx2D(TEXT("sfx_clapper_snap"), 1.f);
	ShowSubtitle(TEXT("You"), TEXT("ACTION."), 2.5f);
	QueueSubtitle(TEXT(""), TakeText::Fill(this, TEXT("(Downstairs, {NEXT} sits down at your desk, reaches for the mug, and smiles at nobody.)")));
	if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this))) { GI->LastEnding = 1; }
	bEndingYes = true;
	EndStep = 4;   // the end card when the lines are done, then the credits
}

// ------------------------------------------------------------------ NO: the set comes down
void UTakeGrabComponent::StartRefusal()
{
	bRefused = true;
	RefuseT = 0.f;
	EndStep = 20;
	if (ATakeFearDirector* FearDir = FindFearDirector())
	{
		FearDir->FlickerBurst(3.f);
		FearDir->StartCreep(0.8f, 3.f);   // they stare (0.35+), but nobody vanishes (0.85)
		FearDir->bForceRed = true;
	}
	PlaySfx2D(TEXT("sfx_fear_hit"), 1.f);
	PlayMusicCue(TEXT("refusal"), TEXT("mus_halcyon_fear1"), true);
	ShowSubtitle(TEXT(""), TEXT("(The applause stops. Mid-clap. Every face turns to you.)"), 4.f);
	// And in the corner by the reception desk, where the light doesn't reach: it's back. It doesn't come closer. It doesn't have to.
	for (TActorIterator<ATakeStalker> It(GetWorld()); It; ++It)
	{
		FVector Lurk(1620.f, 1400.f, 119.f);   // capsule half height 117 (at 100 its feet were in the floor: no collision, so nothing lifted it)
		GetWorld()->FindTeleportSpot(*It, Lurk, It->GetActorRotation());
		It->SetActorLocation(Lurk, false, nullptr, ETeleportType::TeleportPhysics);
		const FVector Me = GetOwner()->GetActorLocation();
		It->SetActorRotation(FRotator(0.f, (Me - Lurk).Rotation().Yaw, 0.f));
		It->SetActorHiddenInGame(false);
		It->SetActorEnableCollision(false);
		GivePlateOfCake(*It, 125.f, false);   // it was allowed a slice. It hasn't touched it.
		break;
	}
	QueueSubtitle(TEXT(""), TEXT("(By the reception desk, in the dark: something tall. Watching. It doesn't come closer.)"));
	if (UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->LastEnding = 2;
		GI->SetSoulBits(0);
	}
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Ending: refused (the director's cut)"));
}

void UTakeGrabComponent::TickRefusal(float DeltaTime)
{
	if (EndStep != 20) { return; }
	const float Before = RefuseT;
	RefuseT += DeltaTime;
	auto Crossed = [Before, this](float T) { return Before < T && RefuseT >= T; };
	// Playtest: "in No ending, coworkers make meaner comments... or haunting remarks". Every few seconds someone near you says one.
	if (RefuseT > 2.5f && SubQueue.Num() == 0 && SubtitleLeft <= 0.f && FMath::FloorToInt(Before / 1.5f) != FMath::FloorToInt(RefuseT / 1.5f))   // (between the story's own lines)
	{
		TArray<ATakeCoworker*> Near;
		const FVector Me = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
		for (TActorIterator<ATakeCoworker> It(GetWorld()); It; ++It)
		{
			if (It->bAtParty && !It->IsHidden() && !It->ActorHasTag(TEXT("Collector")) && FVector::Dist2D(It->GetActorLocation(), Me) < 1100.f) { Near.Add(*It); }
		}
		if (Near.Num() > 0) { Near[FMath::RandRange(0, Near.Num() - 1)]->SayLine(ATakeCoworker::RefusalLine()); }
	}
	if (Crossed(4.f))
	{
		PlaySfx2D(TEXT("sfx_pa_chime"), 0.8f);
		ShowSubtitle(TEXT("From above"), TEXT("CUT."), 3.f);
	}
	if (Crossed(6.f))
	{
		LiftTheSet();
		QueueSubtitle(TEXT(""), TEXT("(The walls are going up. They were never walls.)"));
	}
	if (Crossed(11.f))
	{
		QueueSubtitle(TEXT("From above"), TEXT("...Reset. From the top."));
		QueueSubtitle(TEXT(""), TEXT("(Above the lights: a long row of badges on hooks. 4466. 4467. 4468. 4469. 4470. One empty hook.)"));
	}
	// The flats rise, staggered, eased
	for (FLifted& L : Lifted)
	{
		AActor* A = L.Actor.Get();
		if (!A) { continue; }
		const float T = FMath::Clamp((RefuseT - 6.f - L.Delay) / 5.f, 0.f, 1.f);
		const float E = T * T * (3.f - 2.f * T);
		A->SetActorLocation(L.From + FVector(0.f, 0.f, 1100.f * E));
	}
	if (RefuseT > 16.f && ((SubQueue.Num() == 0 && SubtitleLeft <= 0.f) || RefuseT > 24.f))
	{
		bEndingYes = false;
		EndStep = 4;
	}
}

void UTakeGrabComponent::LiftTheSet()
{
	// The lobby and kitchen's walls and ceiling: anything big in that box that isn't the floor
	const FBox Stage(FVector(1350.f, 250.f, -50.f), FVector(3900.f, 1700.f, 600.f));
	for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(TEXT("DirectorsFloor")) || It->ActorHasTag(TEXT("ElevatorDoor"))) { continue; }
		UStaticMeshComponent* SM = It->GetStaticMeshComponent();
		if (!SM || !SM->GetStaticMesh() || SM->IsSimulatingPhysics()) { continue; }
		FVector O, E;
		It->GetActorBounds(true, O, E);
		if (!Stage.IsInside(O)) { continue; }
		const bool bWall = E.Z >= 100.f && FMath::Min(E.X, E.Y) <= 30.f;   // tall and thin
		const bool bCeiling = O.Z >= 250.f && FMath::Max(E.X, E.Y) >= 100.f && E.Z <= 30.f;
		const bool bDecor = O.Z >= 100.f && FMath::Min(E.X, E.Y) <= 8.f && FMath::Max(E.X, E.Y) >= 12.f && E.Z >= 12.f;   // posters, signs, boards on the walls
		if (!bWall && !bCeiling && !bDecor) { continue; }
		SM->SetMobility(EComponentMobility::Movable);
		FLifted L;
		L.Actor = *It;
		L.From = It->GetActorLocation();
		L.Delay = bCeiling ? 0.f : FMath::FRandRange(0.3f, 1.8f);
		Lifted.Add(L);
	}
	for (TActorIterator<ATextRenderActor> It(GetWorld()); It; ++It)   // the clock, the banner: up with the wall they're on
	{
		const FVector O = It->GetActorLocation();
		if (!Stage.IsInside(O) || O.Z < 150.f) { continue; }
		FLifted L;
		L.Actor = *It;
		L.From = O;
		L.Delay = FMath::FRandRange(0.3f, 1.8f);
		Lifted.Add(L);
	}
	// What was above them all along: lighting rigs
	UStaticMesh* Cube = Mesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	UMaterialInterface* Dark = Mat(TEXT("/Game/Props/Meshes/M_MetalDark.M_MetalDark"));
	for (int32 I = 0; I < 3; ++I)
	{
		const float Y = 650.f + I * 350.f;
		SpawnSetPiece(Cube, FVector(2600.f, Y, 820.f), FRotator::ZeroRotator, FVector(26.f, 0.25f, 0.25f), Dark, false);   // a truss across the stage
		for (int32 J = 0; J < 3; ++J)
		{
			FActorSpawnParameters Params;
			if (ASpotLight* Spot = GetWorld()->SpawnActor<ASpotLight>(FVector(1700.f + J * 800.f, Y, 800.f), FRotator(-90.f, 0.f, 0.f), Params))
			{
				Spot->SpotLightComponent->SetMobility(EComponentMobility::Movable);
				Spot->SpotLightComponent->SetIntensity(60000.f);
				Spot->SpotLightComponent->SetAttenuationRadius(1600.f);
				Spot->SpotLightComponent->SetOuterConeAngle(32.f);
				Spot->SpotLightComponent->SetLightColor(FLinearColor(1.f, 0.95f, 0.85f));
			}
		}
	}
	PlaySfx2D(TEXT("sfx_impact_heavy"), 0.7f);
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Ending: lifting %d flat(s)"), Lifted.Num());
}
