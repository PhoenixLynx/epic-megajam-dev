// TAKE 4471 - the player's hands. One key does everything:
//   E            look at a coworker = talk, your desk monitor = sit and log in,
//                a door / fridge (tag Openable) = open or close, a loose prop = pick up / drop
//   Right Mouse  throw what you're holding
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
enum class ETakeLookKind : uint8 { None, Coworker, Terminal, Openable, Coffee, Prop, TooHeavy };

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

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	struct FSwing { float ClosedYaw = 0.f; float OpenYaw = 90.f; float Alpha = 0.f; bool bOpen = false; };

	UPROPERTY() TObjectPtr<UPhysicsHandleComponent> Handle;
	UPROPERTY() TObjectPtr<UTakePromptWidget> Prompt;
	UPROPERTY() TObjectPtr<UTakeTerminalWidget> Terminal;
	UPROPERTY() TObjectPtr<UTakePauseMenu> PauseMenu;
	UFUNCTION() void HandleResume() { if (PauseMenu) { TogglePause(); } }
	TMap<TWeakObjectPtr<AActor>, FSwing> Swings;
	bool bSeated = false;
	float SubtitleLeft = 0.f;
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
	UFUNCTION() void HandleTerminalLogout() { LogOut(); }
	void Fear0() { SetFearDebug(0.f); }
	void Fear1() { SetFearDebug(0.25f); }
	void Fear2() { SetFearDebug(0.5f); }
	void Fear3() { SetFearDebug(0.75f); }
	void Fear4() { SetFearDebug(0.95f); }
};
