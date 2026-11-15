// TAKE 4471 - run state that survives level loads: seed, take number, soul bits.
// Also hosts console commands for testing (press ` in Play, type e.g. TakeFear 0.8).
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TakeGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeSoulBitsChanged, int32, NewSoulBits);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeObjectiveChanged, const FString&, NewObjective);

/** Where the story is. Act 1: morning tasks -> 10:00 meeting (the slip) -> Corridor B (loop + wisp) -> back to your desk. */
UENUM(BlueprintType)
enum class ETakeBeat : uint8
{
	Morning,        // badge tasks (check in at Reception first)
	Meeting,        // all three tasks done: go to the 10:00 in the Conference Room
	Corridor,       // the slip happened: something is at the end of Corridor B (the loop is allowed now)
	ReturnToDesk,   // picked up the wisp: go back to your desk (beat 1.10 is next)
	Lunch,          // beat 1.10 done: 12:00, "Lunch is served.", Act 2 task list (beats 2.1-2.2)
	Overtime,       // Act 3 (3.1): clocks stuck at 2:14, "mandatory team building in the Wellness Center"
	Ward,           // 3.2-3.5: you fell through the floor into the hospital ward; Key 3; the stalker
	Wrap,           // 3.6 + ending: back in the elevator, C is lit -> the wrap party
	Count UMETA(Hidden)
};

UCLASS()
class DIRECTORSCUT_58_API UTakeGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	/** Unique per run, never shown. Drives puzzles, NPC agents, clue placement. */
	UPROPERTY(BlueprintReadOnly, Category = "Take") int32 RunSeed = 0;

	/** Dev only: if >= 0, used instead of the clock so bugs can be reproduced. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Take|Dev") int32 DevSeedOverride = -1;

	/** Goes up every time the stalker catches you ("CUT. AGAIN."). */
	UPROPERTY(BlueprintReadWrite, Category = "Take") int32 TakeNumber = 1;

	/** Recovered soul bits (0..3). Each one = an access pass, stronger Soul Sight, more astral logic. */
	UPROPERTY(BlueprintReadOnly, Category = "Take") int32 SoulBits = 0;

	UPROPERTY(BlueprintAssignable, Category = "Take") FTakeSoulBitsChanged OnSoulBitsChanged;

	UFUNCTION(BlueprintCallable, Category = "Take") void SetSoulBits(int32 NewValue);
	UFUNCTION(BlueprintCallable, Category = "Take") void AddSoulBit() { SetSoulBits(SoulBits + 1); }

	/** Compliance Training: current slide, and whether Module 1 (the Key 1 tutorial) is done. */
	UPROPERTY(BlueprintReadWrite, Category = "Take") int32 TrainingSlide = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Take") bool bTrainingComplete = false;

	/** In-game clock, minutes after midnight (starts 9:00). */
	UPROPERTY(BlueprintReadWrite, Category = "Take") float WorkdayMinutes = 540.f;

	/** Act 1 badge task list */
	UPROPERTY(BlueprintReadWrite, Category = "Take") bool bReadEmail = false;
	UPROPERTY(BlueprintReadWrite, Category = "Take") bool bGotCoffee = false;
	/** Things you threw at people (Security and HR keep count). */
	UPROPERTY(BlueprintReadWrite, Category = "Take") int32 CoworkersHit = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Take") int32 GuardComplaints = 0;
	UFUNCTION(BlueprintPure, Category = "Take") bool MorningTasksDone() const { return bReadEmail && bGotCoffee && bTrainingComplete; }

	/** Story progress (see ETakeBeat) and the one-line objective shown under the badge task list. */
	UPROPERTY(BlueprintReadWrite, Category = "Take") ETakeBeat Beat = ETakeBeat::Morning;
	UPROPERTY(BlueprintReadOnly, Category = "Take") FString Objective;
	UPROPERTY(BlueprintAssignable, Category = "Take") FTakeObjectiveChanged OnObjectiveChanged;
	/** Set the objective (the HUD shows it and gives a short "New objective" cue when it changes). */
	UFUNCTION(BlueprintCallable, Category = "Take") void SetObjective(const FString& NewObjective);
	/** The standard objective text for the current beat. */
	UFUNCTION(BlueprintPure, Category = "Take") FString ObjectiveForBeat() const;
	/** You've had the receptionist's morning brief. */
	UPROPERTY(BlueprintReadWrite, Category = "Take") bool bMetReceptionist = false;
	/** You picked up the soul-energy wisp at the end of Corridor B (beat 1.8a). */
	UPROPERTY(BlueprintReadWrite, Category = "Take") bool bGotWisp = false;
	/** Act 1 = the morning, Act 2 = midday (from beat 1.10 on). */
	UFUNCTION(BlueprintPure, Category = "Take") int32 CurrentAct() const { return Beat >= ETakeBeat::Overtime ? 3 : (Beat >= ETakeBeat::Lunch ? 2 : 1); }
	/** Act 3: where a capture rewinds you to (the ward entrance), and story flags. */
	UPROPERTY(BlueprintReadWrite, Category = "Take|Act3") FVector RewindPoint = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite, Category = "Take|Act3") bool bTrustFell = false;
	UPROPERTY(BlueprintReadWrite, Category = "Take|Act3") bool bEndingSeen = false;
	/** The title screen shows once per launch (Start / Continue). */
	UPROPERTY(Transient) bool bTitleDone = false;
	/** Start a new run from the morning (Title > Start after an ending). Keeps settings and the take count. */
	void ResetRun();

	/** Act 2 badge tasks (beat 2.2). */
	UPROPERTY(BlueprintReadWrite, Category = "Take|Act2") bool bJoinedCall = false;
	UPROPERTY(BlueprintReadWrite, Category = "Take|Act2") bool bAttendedMeeting = false;
	/** Story NPCs you've had a lunchtime conversation with (0..3). */
	UPROPERTY(BlueprintReadWrite, Category = "Take|Act2") int32 CoworkersCheckedIn = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Take|Act2") bool bRepliedChairman = false;
	/** Every clock stopped at 2:14 (beat 2.6). */
	UPROPERTY(BlueprintReadWrite, Category = "Take|Act2") bool bClocksFrozen = false;
	UFUNCTION(BlueprintPure, Category = "Take|Act2") bool Act2TasksDone() const { return bJoinedCall && bAttendedMeeting && CoworkersCheckedIn >= 3 && bRepliedChairman; }

	/** Mail: which emails you've opened (unread ones are bold) and what you replied (Id -> reply). */
	UPROPERTY(BlueprintReadWrite, Category = "Take|Mail") TArray<FString> ReadMail;
	UPROPERTY(BlueprintReadWrite, Category = "Take|Mail") TMap<FString, FString> MailReplies;
	UFUNCTION(BlueprintPure, Category = "Take|Mail") bool HasReplied(const FString& Id) const { return MailReplies.Contains(Id); }
	/** Glyph notes: every Key Lock signature you've found, one line each ("Key 2 · Energy poster · glyphs"). */
	UPROPERTY(BlueprintReadWrite, Category = "Take|Notes") TArray<FString> GlyphNotes;
	/** Adds a note once per source. Returns true if it was new. */
	bool AddGlyphNote(int32 Key, const FString& Source, int32 Value, int32 Bits);
	static FString GlyphString(int32 Value, int32 Bits);
	/** HR: Respectful Workplace modules done (one is due for every 3 hits). */
	UPROPERTY(BlueprintReadWrite, Category = "Take|HR") int32 HRModulesDone = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Take|HR") int32 HRSlide = 0;
	UFUNCTION(BlueprintPure, Category = "Take|HR") int32 HRModulesDue() const { return FMath::Min(CoworkersHit / 3, 2); }
	UFUNCTION(BlueprintPure, Category = "Take|HR") bool HRModuleDue() const { return HRModulesDone < HRModulesDue(); }
	/** Fun: cups poured, vending snacks, paper toss baskets (best streak). */
	UPROPERTY(BlueprintReadWrite, Category = "Take|Fun") int32 CoffeesPoured = 0;
	/** Energy (0..100): drains slowly; coffee and food bring it back; low = a little slower. Never a fail state. */
	UPROPERTY(BlueprintReadWrite, Category = "Take") float Energy = 100.f;
	void AddEnergy(float Amount) { Energy = FMath::Clamp(Energy + Amount, 0.f, 100.f); }
	UPROPERTY(BlueprintReadWrite, Category = "Take|Fun") int32 Baskets = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Take|Fun") int32 BestTossStreak = 0;
	/** You replied to the payroll email with your key (it wasn't payroll). */
	UPROPERTY(BlueprintReadWrite, Category = "Take|Mail") bool bRepliedPhish = false;

	/** The badge task list for the current act, one "[x]  Task" per line (HUD + pause menu). */
	UFUNCTION(BlueprintPure, Category = "Take") FString TaskListText() const;
	/** Changes whenever anything on the task list changes (the HUD redraws only then). */
	int32 TaskSignature() const;

	/** Dev: TakeBeat 4 starts beat 1.10 right away instead of waiting for you to reach your desk. */
	UPROPERTY(BlueprintReadWrite, Category = "Take|Dev") bool bDevLunchNow = false;
	/** Dev: start Act 3 now (TakeBeat 5, or F8 once Act 2 is done). */
	UPROPERTY(BlueprintReadWrite, Category = "Take|Dev") bool bDevAct3Now = false;
	/** Dev: TakeBeat 6 / 7 go on from Act 3's start straight to the ward / the wrap party. */
	UPROPERTY(BlueprintReadWrite, Category = "Take|Dev") ETakeBeat DevAct3Target = ETakeBeat::Overtime;
	/** Dev: TakeLoop 1 forces the Corridor B loop on outside its beat. */
	UPROPERTY(BlueprintReadWrite, Category = "Take|Dev") bool bDevForceLoop = false;

	/** Access pass N (1..3) is held once you have N soul bits. */
	UFUNCTION(BlueprintPure, Category = "Take") bool HasPass(int32 PassNumber) const { return SoulBits >= PassNumber; }

	// ---- Console commands (testing) ----
	UFUNCTION(Exec) void TakeFear(float Value);          // set Fear instantly (0..1)
	UFUNCTION(Exec) void TakeCreep(float Target, float Seconds = 10.f);
	UFUNCTION(Exec) void TakeSoulBits(int32 Value);
	UFUNCTION(Exec) void TakeStalker();                  // activate the stalker now
	UFUNCTION(Exec) void TakeRewind();                   // "CUT. AGAIN."
	UFUNCTION(Exec) void TakeLoop(bool bEnable);         // force the Corridor B loop on/off
	UFUNCTION(Exec) void TakeSeed();
	UFUNCTION(Exec) void TakeNPCYaw(float Yaw);          // turn every rigged NPC's body (facing test)
	UFUNCTION(Exec) void TakeBeat(int32 NewBeat = -1);   // jump to a story beat: 0 morning, 1 meeting, 2 corridor, 3 return to desk, 4 lunch (Act 2), 5 overtime (Act 3), 6 the ward, 7 wrap party; no number = the list
	UFUNCTION(Exec) void TakeTime(int32 HHMM);
	UFUNCTION(Exec) void TakeSkip();                     // playtest: finish the tasks on the current list (morning -> lunch -> 2:14)           // set the clock, e.g. TakeTime 1410 (four minutes before the clocks freeze)
};
