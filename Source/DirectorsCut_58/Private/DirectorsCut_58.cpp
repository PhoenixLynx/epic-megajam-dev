// TAKE 4471 (Director's Cut) - 2026 Epic AI MegaJam
#include "DirectorsCut_58.h"
#include "Modules/ModuleManager.h"
#include "Sound/SoundAttenuation.h"
#include "UObject/Package.h"

DEFINE_LOG_CATEGORY(LogTake);

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, DirectorsCut_58, "DirectorsCut_58");

USoundAttenuation* TakeAudio::World3D()
{
	static USoundAttenuation* Att = nullptr;
	if (!Att)
	{
		Att = NewObject<USoundAttenuation>(GetTransientPackage(), TEXT("TakeWorldAttenuation"));
		Att->AddToRoot();   // one for the whole session, never garbage-collected
		FSoundAttenuationSettings& S = Att->Attenuation;
		S.bAttenuate = true;
		S.bSpatialize = true;
		S.AttenuationShape = EAttenuationShape::Sphere;
		S.AttenuationShapeExtents = FVector(250.f, 0.f, 0.f);   // full volume within 2.5 m
		S.FalloffDistance = 2500.f;                              // then fades out over 25 m
		S.DistanceAlgorithm = EAttenuationDistanceModel::NaturalSound;
	}
	return Att;
}
