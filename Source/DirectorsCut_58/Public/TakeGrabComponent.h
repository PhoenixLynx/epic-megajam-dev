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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeGrabEvent, UPrimitiveComponent*, Component);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeActorEvent, AActor*, Actor);

UENUM()
enum class ETakeLookKind : uint8 { None, Coworker, Terminal, Openable, Coffee, Phone, Prop, TooHeavy };

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

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	struct FSwing { float ClosedYaw = 0.f; float OpenYaw = 90.f; float Alpha = 0.f; bool bOpen = false; float JiggleLeft = 0.f; bool bUnlocked = false; };

	UPROPERTY() TObjectPtr<UPhysicsHandleComponent> Handle;
	UPROPERTY() TObjectPtr<UTakePromptWidget> Prompt;
	UPROPERTY() TObjectPtr<UTakeTerminalWidget> Terminal;
	UPROPERTY() TObjectPtr<UTakePauseMenu> PauseMenu;
	UFUNCTION() void HandleResume() { if (PauseMenu) { TogglePause(); } }
	TMap<TWeakObjectPtr<AActor>, FSwing> Swings;
	bool bSeated = false;
	float SubtitleLeft = 0.f;
	float SipLeft = 0.f;
	int32 Sips = 0;
	FRotator SeatReturnRotation = FRotator::ZeroRotator;
	bool IsHoldingMug() const;
	void Sip();
	int32 PassNeeded(const AActor* Door) const;   // 0 = not locked by a pass
	bool IsLockedForMe(const AActor* Door) const;
	void PlaySfx(const TCHAR* Name, const FVector& Where, float Volume = 1.f);
	void TickClock(float DeltaTime);
	// Game feel: footsteps, pick up / throw sounds, props clunking when they land
	UPROPERTY() TArray<TObjectPtr<class USoundBase>> StepSounds;
	float StepAccum = 0.f;
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
	// Queued subtitles
	TArray<FString> SubQueue;
	FString SubWho;
	void BindPropImpacts();
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
	void Fear0() { SetFearDebug(0.f); }
	void Fear1() { SetFearDebug(0.25f); }
	void Fear2() { SetFearDebug(0.5f); }
	void Fear3() { SetFearDebug(0.75f); }
	void Fear4() { SetFearDebug(0.95f); }
};
