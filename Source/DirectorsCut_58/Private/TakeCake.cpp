// TAKE 4471 - the cake (see TakeCake.h)
#include "TakeCake.h"
#include "DirectorsCut_58.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
	constexpr int32 ArcSteps = 6;   // segments along the curved edge of each wedge

	// Every triangle goes in twice (both windings), so the cake reads right from any side whatever the engine's
	// facing convention; the normal always points out of the cake.
	void AddTri(TArray<int32>& Tris, int32 A, int32 B, int32 C)
	{
		Tris.Add(A); Tris.Add(B); Tris.Add(C);
		Tris.Add(A); Tris.Add(C); Tris.Add(B);
	}
}

ATakeCake::ATakeCake()
{
	PrimaryActorTick.bCanEverTick = false;
	Cake = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Cake"));
	RootComponent = Cake;
	Cake->bUseComplexAsSimpleCollision = true;   // E finds it with the ordinary look trace
	Cake->SetCollisionProfileName(TEXT("BlockAll"));
	Cake->SetCastShadow(true);
	Cake->SetMobility(EComponentMobility::Movable);
}

UMaterialInterface* ATakeCake::TopMat() const
{
	if (UMaterialInterface* Top = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Ending/MI_CakeTop.MI_CakeTop"))) { return Top; }   // Sol's ART-011
	return LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Props/Meshes/M_FabricMint.M_FabricMint"));   // (not imported yet: plain mint)
}

UMaterialInterface* ATakeCake::SideMat() const
{
	if (CachedSide) { return CachedSide; }
	UMaterialInterface* Shape = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (!Shape) { return nullptr; }
	UMaterialInstanceDynamic* M = UMaterialInstanceDynamic::Create(Shape, const_cast<ATakeCake*>(this));
	M->SetVectorParameterValue(TEXT("Color"), FLinearColor(FColor(160, 208, 172)));   // the mint of Sol's top, a shade darker
	const_cast<ATakeCake*>(this)->CachedSide = M;
	return M;
}

UMaterialInterface* ATakeCake::SpongeMat() const
{
	return LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Props/Meshes/M_Bread.M_Bread"));
}

void ATakeCake::AddWedge(int32 Wedge, const FVector& Offset, int32 SectionBase)
{
	const float Step = 2.f * PI / float(Slices);
	const float A0 = Wedge * Step, A1 = (Wedge + 1) * Step;
	const float R = Radius, H = Height;
	auto Rim = [R](float A, float Z) { return FVector(R * FMath::Cos(A), R * FMath::Sin(A), Z); };
	// Sol's top is mapped flat over the whole cake: seen from above, +X is up in the picture and +Y is right
	auto TopUV = [R](const FVector& P) { return FVector2D(0.5f + P.Y / (2.f * R), 0.5f - P.X / (2.f * R)); };
	const TArray<FLinearColor> NoColors;
	const TArray<FProcMeshTangent> NoTangents;

	{   // the top: a fan from the centre
		TArray<FVector> V; TArray<int32> T; TArray<FVector> N; TArray<FVector2D> UV;
		V.Add(FVector(0.f, 0.f, H) + Offset); N.Add(FVector::UpVector); UV.Add(TopUV(FVector(0.f, 0.f, H)));
		for (int32 I = 0; I <= ArcSteps; ++I)
		{
			const FVector P = Rim(FMath::Lerp(A0, A1, float(I) / ArcSteps), H);
			V.Add(P + Offset); N.Add(FVector::UpVector); UV.Add(TopUV(P));
		}
		for (int32 I = 1; I <= ArcSteps; ++I) { AddTri(T, 0, I, I + 1); }
		Cake->CreateMeshSection_LinearColor(SectionBase + 0, V, T, N, UV, NoColors, NoTangents, true);
		Cake->SetMaterial(SectionBase + 0, TopMat());
	}
	{   // the frosted outside
		TArray<FVector> V; TArray<int32> T; TArray<FVector> N; TArray<FVector2D> UV;
		for (int32 I = 0; I <= ArcSteps; ++I)
		{
			const float A = FMath::Lerp(A0, A1, float(I) / ArcSteps);
			const FVector Out(FMath::Cos(A), FMath::Sin(A), 0.f);
			V.Add(Rim(A, 0.f) + Offset); N.Add(Out); UV.Add(FVector2D(A * R / 100.f, 1.f));
			V.Add(Rim(A, H) + Offset);   N.Add(Out); UV.Add(FVector2D(A * R / 100.f, 0.f));
		}
		for (int32 I = 0; I < ArcSteps; ++I)
		{
			const int32 B = I * 2;
			AddTri(T, B, B + 2, B + 1);
			AddTri(T, B + 1, B + 2, B + 3);
		}
		Cake->CreateMeshSection_LinearColor(SectionBase + 1, V, T, N, UV, NoColors, NoTangents, true);
		Cake->SetMaterial(SectionBase + 1, SideMat());
	}
	{   // the two cut faces: sponge (only seen once a neighbour is gone, or on a slice)
		TArray<FVector> V; TArray<int32> T; TArray<FVector> N; TArray<FVector2D> UV;
		const FVector N0(FMath::Sin(A0), -FMath::Cos(A0), 0.f), N1(-FMath::Sin(A1), FMath::Cos(A1), 0.f);
		for (const TPair<float, FVector>& Face : { TPair<float, FVector>(A0, N0), TPair<float, FVector>(A1, N1) })
		{
			const int32 B = V.Num();
			V.Add(FVector(0.f, 0.f, 0.f) + Offset); V.Add(FVector(0.f, 0.f, H) + Offset);
			V.Add(Rim(Face.Key, H) + Offset);       V.Add(Rim(Face.Key, 0.f) + Offset);
			for (int32 K = 0; K < 4; ++K) { N.Add(Face.Value); }
			UV.Add(FVector2D(0.f, 1.f)); UV.Add(FVector2D(0.f, 0.f)); UV.Add(FVector2D(R / 30.f, 0.f)); UV.Add(FVector2D(R / 30.f, 1.f));
			AddTri(T, B, B + 1, B + 2);
			AddTri(T, B, B + 2, B + 3);
		}
		Cake->CreateMeshSection_LinearColor(SectionBase + 2, V, T, N, UV, NoColors, NoTangents, true);
		Cake->SetMaterial(SectionBase + 2, SpongeMat());
	}
}

void ATakeCake::BuildCake(float InRadius, float InHeight, int32 InSlices, int32 Missing)
{
	Radius = InRadius; Height = InHeight; Slices = FMath::Max(3, InSlices);
	StartMissing = FMath::Clamp(Missing, 0, Slices);
	Cake->ClearAllMeshSections();
	Present.Init(true, Slices);
	for (int32 W = 0; W < Slices; ++W)
	{
		if (W < Missing) { Present[W] = false; continue; }   // already eaten (the director's floor)
		AddWedge(W, FVector::ZeroVector, W * 3);
	}
}

void ATakeCake::BuildSlice(float InRadius, float InHeight, int32 InSlices, int32 Wedge)
{
	Radius = InRadius; Height = InHeight; Slices = FMath::Max(3, InSlices);
	Cake->ClearAllMeshSections();
	Present.Init(false, Slices);
	Present[Wedge % Slices] = true;
	const float Mid = (Wedge + 0.5f) * 2.f * PI / float(Slices);
	const FVector Centre(Radius * 0.55f * FMath::Cos(Mid), Radius * 0.55f * FMath::Sin(Mid), 0.f);   // about where its middle is
	AddWedge(Wedge % Slices, -Centre, 0);
}

bool ATakeCake::CutSlice(int32& OutWedge, FVector& OutDir)
{
	// Cut next to the gap if there is one (that's how people cut a cake), otherwise anywhere
	int32 Pick = INDEX_NONE;
	for (int32 W = 0; W < Present.Num() && Pick == INDEX_NONE; ++W)
	{
		if (Present[W] && !Present[(W + Present.Num() - 1) % Present.Num()]) { Pick = W; }
	}
	for (int32 W = 0; W < Present.Num() && Pick == INDEX_NONE; ++W) { if (Present[W]) { Pick = W; } }
	if (Pick == INDEX_NONE) { return false; }
	Present[Pick] = false;
	for (int32 K = 0; K < 3; ++K) { Cake->ClearMeshSection(Pick * 3 + K); }
	OutWedge = Pick;
	const float Mid = (Pick + 0.5f) * 2.f * PI / float(Slices);
	OutDir = GetActorRotation().RotateVector(FVector(FMath::Cos(Mid), FMath::Sin(Mid), 0.f));
	UE_LOG(LogTake, Log, TEXT("[TAKE 4471] Cake: slice %d cut, %d left"), Pick, SlicesLeft());
	return true;
}

int32 ATakeCake::SlicesLeft() const
{
	int32 N = 0;
	for (const bool B : Present) { if (B) { ++N; } }
	return N;
}
