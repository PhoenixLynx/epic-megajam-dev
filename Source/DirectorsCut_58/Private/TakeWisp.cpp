// TAKE 4471 - soul-energy wisp
#include "TakeWisp.h"
#include "DirectorsCut_58.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ATakeWisp::ATakeWisp()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	Orb = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Orb"));
	Orb->SetupAttachment(SceneRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded()) { Orb->SetStaticMesh(SphereMesh.Object); }
	Orb->SetRelativeScale3D(FVector(OrbScale));
	Orb->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Orb->SetCollisionResponseToAllChannels(ECR_Ignore);
	Orb->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);   // the look trace can find it
	Orb->SetCastShadow(false);

	Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
	Glow->SetupAttachment(SceneRoot);   // not under the scaled orb
	Glow->IntensityUnits = ELightUnits::Lumens;
	Glow->Intensity = GlowLumens;
	Glow->SetLightColor(GlowColor);
	Glow->SetAttenuationRadius(650.f);
	Glow->SetCastShadows(false);
}

void ATakeWisp::BeginPlay()
{
	Super::BeginPlay();
	if (!OrbMaterial) { OrbMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Fear/M_Wisp.M_Wisp")); }
	if (OrbMaterial) { Orb->SetMaterial(0, OrbMaterial); }
	Glow->SetLightColor(GlowColor);
	Glow->SetIntensity(GlowLumens);
	Clock = FMath::FRandRange(0.f, 5.f);
	bShown = true;
	ApplyShown(IsAvailable());
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Wisp ready (%s material)"), OrbMaterial ? TEXT("glow") : TEXT("default"));
}

bool ATakeWisp::IsAvailable() const
{
	if (bCollected) { return false; }
	const UTakeGameInstance* GI = Cast<UTakeGameInstance>(UGameplayStatics::GetGameInstance(this));
	return !GI || GI->Beat == VisibleDuring;   // no story state: always there (testing)
}

void ATakeWisp::Collect()
{
	bCollected = true;
	ApplyShown(false);
}

void ATakeWisp::ApplyShown(bool bNewShown)
{
	if (bShown == bNewShown) { return; }
	bShown = bNewShown;
	SetActorHiddenInGame(!bNewShown);
	SetActorEnableCollision(bNewShown);
}

void ATakeWisp::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ApplyShown(IsAvailable());
	if (!bShown) { return; }
	Clock += DeltaSeconds;
	const FVector Bob(0.f, 0.f, BobHeight * FMath::Sin(Clock * BobSpeed));
	Orb->SetRelativeLocation(Bob);
	Glow->SetRelativeLocation(Bob);
	Orb->SetRelativeScale3D(FVector(OrbScale * (1.f + 0.07f * FMath::Sin(Clock * 4.3f))));
	Glow->SetIntensity(GlowLumens * (0.8f + 0.2f * FMath::Sin(Clock * 3.1f)));
}
