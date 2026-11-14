// TAKE 4471 - save slot: workday progress + where you are. Settings live in their own slot.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "TakeSaveGame.generated.h"

UCLASS()
class DIRECTORSCUT_58_API UTakeSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	static const FString ProgressSlot;   // "Take4471"
	static const FString SettingsSlot;   // "Take4471_Settings"

	// progress
	UPROPERTY() int32 TakeNumber = 1;
	UPROPERTY() int32 SoulBits = 0;
	UPROPERTY() int32 TrainingSlide = 0;
	UPROPERTY() bool bTrainingComplete = false;
	UPROPERTY() bool bReadEmail = false;
	UPROPERTY() bool bGotCoffee = false;
	UPROPERTY() int32 RunSeed = 0;
	UPROPERTY() float Fear = 0.f;
	UPROPERTY() FVector PlayerLocation = FVector::ZeroVector;
	UPROPERTY() FRotator PlayerRotation = FRotator::ZeroRotator;
	UPROPERTY() FDateTime SavedAt;
	// story (overnight run 1)
	UPROPERTY() int32 BeatIndex = 0;          // ETakeBeat as a number
	UPROPERTY() FString Objective;
	UPROPERTY() bool bMetReceptionist = false;
	UPROPERTY() bool bGotWisp = false;

	// settings
	UPROPERTY() float MasterVolume = 1.f;
	UPROPERTY() int32 DialogueMode = 0;   // 0 subtitles, 1 overhead, 2 both
};
