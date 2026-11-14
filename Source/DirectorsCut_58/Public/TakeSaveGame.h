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
	// Act 2 (overnight run 2)
	UPROPERTY() float WorkdayMinutes = 540.f;
	UPROPERTY() bool bJoinedCall = false;
	UPROPERTY() bool bAttendedMeeting = false;
	UPROPERTY() int32 CoworkersCheckedIn = 0;
	UPROPERTY() bool bRepliedChairman = false;
	UPROPERTY() bool bClocksFrozen = false;
	// Saturday batch: mail, notes, HR, fun
	UPROPERTY() TArray<FString> ReadMail;
	UPROPERTY() TMap<FString, FString> MailReplies;
	UPROPERTY() TArray<FString> GlyphNotes;
	UPROPERTY() int32 CoworkersHit = 0;
	UPROPERTY() int32 GuardComplaints = 0;
	UPROPERTY() int32 HRModulesDone = 0;
	UPROPERTY() int32 CoffeesPoured = 0;
	UPROPERTY() int32 BestTossStreak = 0;
	UPROPERTY() bool bRepliedPhish = false;

	// settings
	UPROPERTY() float MasterVolume = 1.f;
	UPROPERTY() int32 DialogueMode = 0;   // 0 subtitles, 1 overhead, 2 both
};
