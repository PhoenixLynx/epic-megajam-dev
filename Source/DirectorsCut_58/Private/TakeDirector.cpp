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
#include "Engine/World.h"
#include "EngineUtils.h"
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
	SpawnSetPiece(Cyl, Table + FVector(0.f, 0.f, 76.f + 10.f), FRotator::ZeroRotator, FVector(0.55f, 0.55f, 0.2f), Frosting, false);     // the cake, whole (for now)
	SpawnSetPiece(Cyl, Table + FVector(0.f, 0.f, 76.f + 28.f), FRotator::ZeroRotator, FVector(0.36f, 0.36f, 0.16f), FabricMint, false);
	SpawnSetPiece(Cyl, Table + FVector(0.f, 0.f, 76.f + 41.f), FRotator::ZeroRotator, FVector(0.02f, 0.02f, 0.1f), White, false);       // one candle
	SpawnLabel(World, TEXT("THAT'S A WRAP, 4471"), Table + FVector(0.f, -29.f, 76.f + 12.f), FRotator(0.f, -90.f, 0.f), 4.5f, FColor(40, 140, 115));
	SpawnLabel(World, TEXT("THAT'S A WRAP, 4471"), Table + FVector(0.f, 29.f, 76.f + 12.f), FRotator(0.f, 90.f, 0.f), 4.5f, FColor(40, 140, 115));
	for (int32 I = 0; I < 6; ++I)   // cups and cans, a little everywhere
	{
		const FVector At = Table + FVector(-70.f + I * 28.f, (I % 2 == 0) ? -28.f : 30.f, 76.f);
		SpawnSetPiece(Prop(I % 3 == 2 ? TEXT("SM_SodaCan") : TEXT("SM_Mug")), At, FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f), FVector(1.f), nullptr, false);
	}
	SpawnSetPiece(Prop(TEXT("SM_CupStack")), Table + FVector(75.f, 0.f, 76.f), FRotator::ZeroRotator, FVector(1.f), nullptr, false);

	// The banner: over the elevator doors, on the lobby side
	SpawnLabel(World, TEXT("THAT'S A WRAP, 4471!"), FVector(2000.f, 518.f, 272.f), FRotator(0.f, 90.f, 0.f), 34.f, FColor(70, 190, 160));
	SpawnSetPiece(Cube, FVector(2000.f, 512.f, 272.f), FRotator::ZeroRotator, FVector(4.2f, 0.02f, 0.5f), White, false);

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
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] The wrap party is dressed (lobby)"));
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
	// A rug under the chair
	Box(P + FVector(0.f, 40.f, 1.f), FVector(520.f, 380.f, 2.f), Mat(TEXT("/Game/Props/Materials/MI_Rug.MI_Rug")), false);

	// The monitor wall: two rows of three, the dailies. Bottom middle is live (set up when you sit).
	static const TCHAR* Feeds[] = { TEXT("MI_Screen_CCTV"), TEXT("MI_Screen_Call"), TEXT("MI_Screen_Chart"),
	                                TEXT("MI_Screen_Fear"), nullptr, TEXT("MI_Screen_Promo") };
	for (int32 I = 0; I < 6; ++I)
	{
		const int32 Col = I % 3, Row = I / 3;
		const FVector At = P + FVector(-260.f + Col * 260.f, -PD * 0.5f + 40.f, Row == 0 ? 200.f : 70.f);
		AStaticMeshActor* Mon = SpawnSetPiece(Prop(TEXT("SM_Monitor")), At, FRotator(0.f, 180.f, 0.f), FVector(2.4f), nullptr, false);
		if (!Mon) { continue; }
		UStaticMeshComponent* SM = Mon->GetStaticMeshComponent();
		const int32 Slot = SM->GetMaterialIndex(FName(TEXT("M_Screen")));
		if (Slot == INDEX_NONE) { continue; }
		if (Feeds[I])
		{
			if (UMaterialInterface* Feed = Mat(*FString::Printf(TEXT("/Game/Props/Materials/%s.%s"), Feeds[I], Feeds[I]))) { SM->SetMaterial(Slot, Feed); }
		}
		else { LiveMonitor = SM; LiveMonitorSlot = Slot; }
	}
	SpawnLabel(World, TEXT("DAILIES  ·  TAKES 1 - 4471"), P + FVector(0.f, -PD * 0.5f + 14.f, PH - 40.f), FRotator(0.f, 90.f, 0.f), 22.f, FColor(150, 225, 205));

	// The chair: empty, facing the monitors. Its name card is blank until you sit.
	if (AStaticMeshActor* Chair = SpawnSetPiece(Prop(TEXT("SM_DirectorsChair")), P + FVector(0.f, 60.f, 0.f), FRotator(0.f, 180.f, 0.f), FVector(1.f), nullptr, true))
	{
		Chair->Tags.Add(TEXT("Use=DirectorsChair"));
		DirectorChair = Chair;
	}
	if (ATextRenderActor* Name = SpawnLabel(World, TEXT("DIRECTOR: ______"), P + FVector(0.f, 112.f, 118.f), FRotator(0.f, 90.f, 0.f), 13.f, FColor(245, 245, 240)))
	{
		ChairName = Name->GetTextRender();
	}

	// After the party: the cake (half gone), cups, a banner, a sofa nobody sat on
	const FVector Table = P + FVector(430.f, 300.f, 0.f);
	Box(Table + FVector(0.f, 0.f, 37.f), FVector(160.f, 90.f, 74.f), Mat(TEXT("/Game/Props/Meshes/M_PlasticWhite.M_PlasticWhite")));
	SpawnSetPiece(Cyl, Table + FVector(-20.f, 0.f, 74.f + 9.f), FRotator::ZeroRotator, FVector(0.5f, 0.5f, 0.18f), Frosting, false);    // bottom tier
	SpawnSetPiece(Cyl, Table + FVector(-20.f, 0.f, 74.f + 25.f), FRotator::ZeroRotator, FVector(0.32f, 0.32f, 0.14f), Fabric, false);  // top tier (mint)
	Box(Table + FVector(26.f, 0.f, 74.f + 9.f), FVector(10.f, 30.f, 16.f), Mat(TEXT("/Game/Props/Meshes/M_Bread.M_Bread")), false);  // the cut: you can see the sponge
	SpawnLabel(World, TEXT("THAT'S A WRAP, 4471"), Table + FVector(-20.f, -27.f, 74.f + 12.f), FRotator(0.f, -90.f, 0.f), 4.f, FColor(40, 140, 115));
	SpawnSetPiece(Prop(TEXT("SM_CupStack")), Table + FVector(55.f, -25.f, 74.f), FRotator::ZeroRotator, FVector(1.f), nullptr, false);
	SpawnSetPiece(Prop(TEXT("SM_SodaCan")), Table + FVector(60.f, 25.f, 74.f), FRotator::ZeroRotator, FVector(1.f), nullptr, false);
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
			if (ATextRenderActor* Badge = SpawnLabel(World, TEXT("4472"), Stand + FVector(0.f, 0.f, 210.f), FRotator(0.f, Face.Rotation().Yaw + 180.f, 0.f), 26.f, FColor(110, 215, 185)))
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
	TeleportPlayer(PenthouseArrival, -160.f);
	ShowTimeCard(TEXT("??:??"), TEXT("SCENE 7  ·  C"));
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
	if (UTextRenderComponent* Name = ChairName.Get()) { Name->SetText(FText::FromString(TEXT("DIRECTOR: 4471"))); }
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
	ShowSubtitle(TEXT(""), TEXT("(You sit. The name card says DIRECTOR: 4471. It always did.)"), 4.f);
	QueueSubtitle(TEXT(""), TEXT("(The centre screen: your desk, downstairs. Someone new walks up to it. Their badge says 4472.)"));
	QueueSubtitle(TEXT("PA"), TEXT("Ready when you are, Director."));
	QueueSubtitle(TEXT(""), TEXT("(E: call it.)"));
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Ending: in the director's chair"));
}

void UTakeGrabComponent::CallAction()
{
	if (EndStep != 13) { return; }
	PlaySfx2D(TEXT("sfx_clapper_snap"), 1.f);
	ShowSubtitle(TEXT("You"), TEXT("ACTION."), 2.5f);
	QueueSubtitle(TEXT(""), TEXT("(Downstairs, 4472 sits down at your desk. The coffee is exactly the right temperature.)"));
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
	ShowSubtitle(TEXT(""), TEXT("(The applause stops. Mid-clap. Every face turns to you.)"), 4.f);
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
	if (RefuseT > 16.f && SubQueue.Num() == 0 && SubtitleLeft <= 0.f)
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
		if (!bWall && !bCeiling) { continue; }
		SM->SetMobility(EComponentMobility::Movable);
		FLifted L;
		L.Actor = *It;
		L.From = It->GetActorLocation();
		L.Delay = bCeiling ? 0.f : FMath::FRandRange(0.3f, 1.8f);
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
