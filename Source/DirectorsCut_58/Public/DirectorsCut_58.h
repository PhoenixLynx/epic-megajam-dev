// TAKE 4471 (Director's Cut) - 2026 Epic AI MegaJam
#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTake, Log, All);

class USoundAttenuation;

namespace TakeAudio
{
	/** Shared distance falloff for sounds placed in the world (impacts, phones, footsteps, the stalker):
	 *  full volume within 2.5 m, silent by about 27 m, panned left/right. Without it, Unreal plays them at
	 *  the same volume everywhere (a phone upstairs rang just as loud in the ward). */
	DIRECTORSCUT_58_API USoundAttenuation* World3D();
}
