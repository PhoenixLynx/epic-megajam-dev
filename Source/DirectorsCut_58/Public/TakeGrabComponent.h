// TAKE 4471 - the player's hands. One key does everything:
//   E            look at a coworker = talk, your desk monitor = sit and log in,
//                a door / fridge (tag Openable) = open or close, a loose prop = pick up / drop
//   Right Mouse  throw what you're holding
//   F            sip (while holding a mug)
//   1-4 / 0      debug: Fear 0.25 / 0.5 / 0.75 / 0.95 / calm   (bDebugFearKeys)
// Reach becomes telekinesis with 1+ soul bits. Added to the player automatically by the Fear Director.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TakeGrabComponent.generated.h"

class UPhysicsHandleComponent;
class UPrimitiveComponent;
class UTakePromptWidget;
class UTakeTerminalWidget;
class UTakePauseMenu;
class UTakeInspectWidget;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeGrabEvent, UPrimitiveComponent*, Component);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeActorEvent, AActor*, Actor);

UENUM()
enum class ETakeLookKind : uint8 { None, Coworker, Terminal, Openable, Coffee, Phone, Inspect, Prop, TooHeavy, Wisp, Call, Vending, LockPanel, Use };

UCLASS(ClassGroup = (Take), meta = (BlueprintSpawnableComponent, DisplayName = "Take Grab"))
class DIRECTORSCUT_58_API UTakeGrabComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTakeGrabComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab") float GrabRange = 300.f;
	/** Reach with 1+ soul bits (astral logic). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab") float TelekinesisRange = 800.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab") float HoldDistance = 150.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab") float ThrowStrength = 1800.f;
	/** Heavier than this (kg) needs 3 soul bits. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab") float MaxMassWithoutTelekinesis = 40.f;
	/** Doors, fridge, coworkers and the terminal: how close you must be. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab") float UseRange = 250.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab") float DoorSwingSeconds = 0.6f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab") bool bAutoBindKeys = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab") bool bShowPrompt = true;
	/** Hide the template's first-person arms / body meshes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab") bool bHideTemplateMeshes = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab") bool bDebugFearKeys = true;

	UPROPERTY(BlueprintAssignable, Category = "Grab") FTakeGrabEvent OnGrabbed;
	UPROPERTY(BlueprintAssignable, Category = "Grab") FTakeGrabEvent OnThrown;
	UPROPERTY(BlueprintAssignable, Category = "Grab") FTakeActorEvent OnLoggedIn;
	UPROPERTY(BlueprintAssignable, Category = "Grab") FTakeActorEvent OnLoggedOut;

	/** E: talk / log in / open / grab / drop, depending on what you're looking at. */
	UFUNCTION(BlueprintCallable, Category = "Grab") void Interact();
	/** Grab what you're looking at, or drop what you're holding. Returns true if now holding something. */
	UFUNCTION(BlueprintCallable, Category = "Grab") bool TryGrabOrDrop();
	UFUNCTION(BlueprintCallable, Category = "Grab") void Drop();
	UFUNCTION(BlueprintCallable, Category = "Grab") void Throw();
	UFUNCTION(BlueprintCallable, Category = "Grab") void ToggleOpen(AActor* Openable);
	UFUNCTION(BlueprintCallable, Category = "Grab") void SitAndLogIn(AActor* TerminalActor);
	UFUNCTION(BlueprintCallable, Category = "Grab") void LogOut();
	UFUNCTION(BlueprintPure, Category = "Grab") bool IsHolding() const;
	UFUNCTION(BlueprintPure, Category = "Grab") bool IsSeated() const { return bSeated; }
	UFUNCTION(BlueprintPure, Category = "Grab") float CurrentRange() const;
	/** Esc (packaged build) or P: pause with the HALCYON break menu. */
	UFUNCTION(BlueprintCallable, Category = "Grab") void TogglePause();
	/** Show "Who: line" at the bottom of the screen for a few seconds. */
	UFUNCTION(BlueprintCallable, Category = "Grab") void ShowSubtitle(const FString& Who, const FString& Line, float Seconds = 4.f);
	/** Several lines one after another (phone calls, PA announcements). */
	void ShowSubtitleSequence(const FString& Who, const TArray<FString>& Lines);
	/** Make a desk phone ring now (nullptr = your phone at #4471). */
	UFUNCTION(BlueprintCallable, Category = "Grab") void RingPhone(AActor* Phone = nullptr);

	// ---- Story (Act 1 progression, TakeStory.cpp) ----
	/** Black, "TAKE n", PA chime + "Good morning, team." when Play starts. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story") bool bPlayColdOpen = true;
	/** A TakeBeatTrigger box was entered (ConferenceRoom, CorridorB, ...). */
	UFUNCTION(BlueprintCallable, Category = "Story") void OnBeatTrigger(FName TriggerName);
	/** A story NPC finished a conversation ("" = its first one, "Pass1", "Done", "Slip"). */
	void OnStoryTalkFinished(const FString& RoleName, const FString& SetName);
	/** Show this subtitle after whatever is showing now (and anything already queued). */
	void QueueSubtitle(const FString& Who, const FString& Line);

	// ---- Act 2 doorway (overnight run 2, TakeStory.cpp) ----
	/** Beat 1.10 starts when you come back within this distance of desk #4471 (cm), after the wisp and training. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story") float DeskArriveRadius = 260.f;
	/** Beat 2.1: "Lunch is served." is repeated word for word this many extra times, this many seconds apart. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story") int32 LunchPAExtraRepeats = 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story") float LunchPARepeatSeconds = 6.5f;
	/** After that, the same announcement comes back now and then during Act 2 (random gap between these, seconds). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story") float LunchPAGapMin = 120.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story") float LunchPAGapMax = 200.f;
	/** Beat 2.6: every clock stops at this time (minutes after midnight; 2:14 PM = 854). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story") float FrozenClockMinutes = 854.f;
	/** When the clocks freeze, Fear never drops below this again in Act 2 ("Fear begins faster"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story") float FrozenFearFloor = 0.15f;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	struct FSwing
	{
		float ClosedYaw = 0.f; float OpenYaw = 90.f; float Alpha = 0.f; bool bOpen = false; float JiggleLeft = 0.f; bool bUnlocked = false;
		float Slide = 0.f; FVector ClosedLoc = FVector::ZeroVector; FVector SlideDir = FVector::ZeroVector;   // drawers
	};

	UPROPERTY() TObjectPtr<UPhysicsHandleComponent> Handle;
	UPROPERTY() TObjectPtr<UTakePromptWidget> Prompt;
	UPROPERTY() TObjectPtr<UTakeTerminalWidget> Terminal;
	UPROPERTY() TObjectPtr<UTakePauseMenu> PauseMenu;
	UPROPERTY() TObjectPtr<UTakeInspectWidget> Inspecting;
	void Inspect(AActor* Target);
	UFUNCTION() void CloseInspect();
	UFUNCTION() void HandleResume() { if (PauseMenu) { TogglePause(); } }
	TMap<TWeakObjectPtr<AActor>, FSwing> Swings;
	bool bSeated = false;
	float SubtitleLeft = 0.f;
	float SipLeft = 0.f;
	int32 Sips = 0;
	FRotator SeatReturnRotation = FRotator::ZeroRotator;
	bool IsHoldingMug() const;
	bool IsHoldingSnack() const;
	void Eat();
	int32 Eaten = 0;
	bool bMailChimed = false;
	float MugHoldDistance = 55.f;
	void Sip();
	int32 PassNeeded(const AActor* Door) const;   // 0 = not locked by a pass
	bool IsLockedForMe(const AActor* Door) const;
	void PlaySfx(const TCHAR* Name, const FVector& Where, float Volume = 1.f);
	void TickClock(float DeltaTime);
	// Game feel: footsteps, pick up / throw sounds, props clunking when they land
	UPROPERTY() TArray<TObjectPtr<class USoundBase>> StepSounds;
	float StepAccum = 0.f;   // seconds since the last step
	int32 LastStep = -1;
	double PlayStartTime = 0.0;
	TMap<TWeakObjectPtr<UPrimitiveComponent>, double> LastImpact;
	void TickFootsteps(float DeltaTime);
	// Desk phones
	TArray<TWeakObjectPtr<AActor>> Phones;
	TWeakObjectPtr<AActor> MyPhone;
	TWeakObjectPtr<AActor> RingingPhone;
	UPROPERTY() TObjectPtr<class UAudioComponent> RingAudio;
	float RingLeft = 0.f;
	float RingCycle = 0.f;
	float NextRingIn = 45.f;
	int32 MyCalls = 0;
	void TickPhones(float DeltaTime);
	void AnswerPhone(AActor* Phone);
	bool PhoneRingingNear() const;
	void StopRinging();
	// Queued subtitles (each line has its own speaker)
	TArray<FString> SubQueue;
	TArray<FString> SubQueueWho;
	// Story (TakeStory.cpp)
	float SlateT = -1.f;          // < 0 = no slate showing
	bool bSlateRewind = false;
	bool bSlateClap = false;
	bool bSlatePA = false;
	int32 SlateTake = 1;
	bool bCorridorEntered = false;
	FString ObjectiveShown;
	void StartSlate(bool bRewind);
	void SetColdOpenFade(float Amount);
	void TickSlate(float DeltaTime);
	void TickStory();
	void PlaySfx2D(const TCHAR* Name, float Volume = 1.f);
	void CollectWisp(class ATakeWisp* Wisp);
	class ATakeWisp* WispInView() const;
	class ATakeFearDirector* FindFearDirector() const;
	bool TryBreakLoopAt(AActor* Poster);
	void BindStoryEvents();
	// Act 2 doorway (TakeStory.cpp)
	TWeakObjectPtr<AActor> MyDesk;          // the Monitor4471 actor (or the Desk4471 PlayerStart)
	int32 LunchPALeft = 0;                  // word-for-word repeats still to play (beat 2.1)
	float LunchPAIn = -1.f;                 // seconds until the next one (< 0 = none planned)
	int32 LunchPAPlayed = 0;
	bool bLunchRoomNoted = false;
	float FloorCheckIn = 0.f;
	void StartLunch();
	void TickAct2(float DeltaTime);
	void PlayLunchPA();
	void FreezeClocks();
	bool IsNearMyDesk();
	UFUNCTION() void HandleRewind(int32 NewTakeNumber);
	UFUNCTION() void HandleLooped(int32 Count);
	void BindPropImpacts();
	// Monitors flip between work screens (and something else at high Fear)
	struct FScreen { TWeakObjectPtr<UStaticMeshComponent> Comp; int32 Slot = 0; float Next = 0.f; bool bMine = false; };
	TArray<FScreen> Screens;
	UPROPERTY() TArray<TObjectPtr<class UMaterialInterface>> WorkScreens;
	UPROPERTY() TArray<TObjectPtr<class UMaterialInterface>> FearScreens;
	UPROPERTY() TObjectPtr<class UMaterialInterface> LoginScreen;
	UPROPERTY() TObjectPtr<class UMaterialInterface> InboxScreen;
	void TickScreens(float DeltaTime);
	// Desk #4471 stays tidy; high Fear throws it around, and it puts itself back afterwards
	struct FDeskItem { TWeakObjectPtr<AActor> Actor; FTransform Home; bool bPhysics = false; };
	TArray<FDeskItem> DeskItems;
	bool bDeskThrown = false;
	void TickDesk();
	UFUNCTION() void OnPropHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	int32 TasksShown = -1;          // bitmask of finished tasks last time we drew the list
	void PourCoffee();
	void UpdateTasks();
	class ATakeCoworker* CoworkerInView() const;
	void ThrowInput() { if (IsHolding()) { Throw(); } }
	float HeldYawOffset = 0.f;
	FRotator HeldPitchRoll = FRotator::ZeroRotator;
	FVector SeatReturnLocation;
	TWeakObjectPtr<AActor> SeatedAt;

	int32 GetSoulBits() const;
	bool GetView(FVector& OutLoc, FRotator& OutRot) const;
	ETakeLookKind Look(FHitResult& OutHit) const;
	void BindKeys();
	void HideTemplateMeshes();
	void UpdatePrompt();
	void TickSwings(float DeltaTime);
	void SetFearDebug(float Value);
	UFUNCTION() void HandleTerminalLogout();
	// ---- Saturday batch (TakeAct2.cpp) ----
	// The 12:30 call and the 1:00 meeting (the same call, word for word)
	bool CanJoinCall() const;
	void JoinCall();
	void TickCall(float DeltaTime);
	float CallLeft = -1.f;          // seconds until the call ends (< 0 = not in a call)
	bool bCallIsMeeting = false;
	// Phone: reminders for the call/meeting, and calling back when you miss it
	bool bCallReminded = false;
	bool bMeetingReminded = false;
	int32 ReminderRinging = 0;      // 1 = call reminder, 2 = meeting reminder is what's ringing now
	bool bCallBackMine = false;     // you missed your phone: it rings again soon
	bool TickReminders();
	// Coffee machine, vending machine, food that comes back, paper toss
	void Vend(AActor* Machine);
	AActor* SpawnProp(const TCHAR* MeshName, const FVector& Where, const FRotator& Rot);
	struct FFood { TWeakObjectPtr<AActor> Actor; FTransform Home; float Back = -1.f; };
	TArray<FFood> Foods;
	int32 Vended = 0;
	// Restroom and other small "Use=<Key>" things (sink, hand dryer, toilet); Fear-only props (shoes under the stall)
	void UseThing(AActor* Thing);
	static FString UseKey(const AActor* Thing);
	TMap<FString, int32> UseCounts;
	int32 Knocks = 0;
	TArray<TWeakObjectPtr<AActor>> FearReveals;
	bool bFearRevealShown = true;
	void TickFearReveals();
	// Act2Reveal: things that only exist from lunch on (the lunch spread, the director's chair)
	struct FReveal { TWeakObjectPtr<AActor> Actor; bool bPhysics = false; };
	TArray<FReveal> Act2Things;
	bool bAct2Revealed = false;
	void RevealAct2();
	TArray<TWeakObjectPtr<AActor>> Spawned;   // vended snacks and poured mugs (oldest recycled)
	TArray<TWeakObjectPtr<AActor>> Bins;
	TWeakObjectPtr<UPrimitiveComponent> TossProp;
	float TossLeft = 0.f;
	int32 TossStreak = 0;
	void CollectFunProps();
	void TickFun(float DeltaTime);
	void FoodEaten(AActor* Food);
	// Key Lock 2: the "Employee 4471" login panel in the supply closet
	UPROPERTY() TObjectPtr<class UTakeLockPanelWidget> LockPanel;
	void OpenLockPanel(int32 Key);
	UFUNCTION() void CloseLockPanel();
	// New mail chime
	int32 MailSeen = -1;
	float MailCheckIn = 1.f;
	void TickMail(float DeltaTime);
	void NoteGlyphs(int32 Key, const FString& Source);
	void SkipTasks();
	void OpenNotes();
	// Energy (GameInstance->Energy): drains, coffee / food / the wisp refill it, low = a bit slower
	float BaseWalkSpeed = -1.f;
	bool bLowEnergyNoted = false;
	void TickEnergy(float DeltaTime);
	// Security cameras turn to follow you
	TArray<TWeakObjectPtr<AActor>> Cams;
	void TickCams(float DeltaTime);
	void Fear0() { SetFearDebug(0.f); }
	void Fear1() { SetFearDebug(0.25f); }
	void Fear2() { SetFearDebug(0.5f); }
	void Fear3() { SetFearDebug(0.75f); }
	void Fear4() { SetFearDebug(0.95f); }
};
