// TAKE 4471 - the stalker. Dormant until activated (Fear stage 4 or TakeStalker), then
// stalks at the edge of your vision, chases when you look at it, staggers when hit by a
// thrown prop, and triggers "CUT. AGAIN." when it catches you. Speed scales with soul bits.
// Make a Blueprint child (BP_Stalker), give it SM_Stalker as a Static Mesh child, place it on
// GB_Stalker_Spawn_Act1. Needs the NavMeshBoundsVolume (press P to check).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TakeStalker.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ETakeStalkerState : uint8 { Dormant, Stalk, Chase, Stunned };

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeStalkerStateChanged, ETakeStalkerState, NewState);

UCLASS()
class DIRECTORSCUT_58_API ATakeStalker : public ACharacter
{
	GENERATED_BODY()

public:
	ATakeStalker();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stalker") TObjectPtr<USphereComponent> CatchSphere;
	/** Visual: set this to SM_Stalker (pivot at the feet). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stalker") TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(BlueprintReadOnly, Category = "Stalker") ETakeStalkerState State = ETakeStalkerState::Dormant;
	UPROPERTY(BlueprintAssignable, Category = "Stalker") FTakeStalkerStateChanged OnStateChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stalker|Tuning") float StalkSpeed = 180.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stalker|Tuning") float ChaseBaseSpeed = 350.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stalker|Tuning") float ChaseSpeedPerSoulBit = 80.f;
	/** How far behind the player it lurks while stalking (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stalker|Tuning") float StalkDistance = 600.f;
	/** Seconds of stalking before it gives chase anyway. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stalker|Tuning") float StalkDuration = 6.f;
	/** Looking straight at it (dot product above this) makes it chase. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stalker|Tuning") float LookAtDot = 0.9f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stalker|Tuning") float StunSeconds = 1.5f;
	/** A thrown prop must be moving at least this fast (cm/s) to stagger it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stalker|Tuning") float MinHitSpeed = 400.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stalker|Tuning") bool bHiddenWhileDormant = true;

	/** Upstairs in Act 3 (the foreshadowing): catching you doesn't rewind; it just sets bSoftCaught for the story to handle. */
	bool bSoftCatch = false;
	bool bSoftCaught = false;
	UFUNCTION(BlueprintCallable, Category = "Stalker") void Activate();
	UFUNCTION(BlueprintCallable, Category = "Stalker") void Deactivate();
	UFUNCTION(BlueprintCallable, Category = "Stalker") void Stun(float Seconds);
	/** Slow for a while (fire extinguisher). */
	UFUNCTION(BlueprintCallable, Category = "Stalker") void Slow(float Multiplier, float Seconds);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
	                       FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

private:
	FVector SpawnLocation;
	float StateTime = 0.f;
	float RepathTimer = 0.f;
	float SlowMultiplier = 1.f;
	float SlowTimeLeft = 0.f;
	float StunFor = 1.5f;
	float StuckTime = 0.f;          // how long it has wanted to move without getting anywhere
	FVector StuckCheckFrom = FVector::ZeroVector;
	void SetState(ETakeStalkerState NewState);
	float ChaseSpeed() const;
	APawn* GetPlayerPawn() const;
	UFUNCTION() void OnCatchOverlap(UPrimitiveComponent* OverlappedComp, AActor* Other, UPrimitiveComponent* OtherComp,
	                                int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);
	UPROPERTY() TObjectPtr<class UAnimSequence> AnimWalk;
	UPROPERTY() TObjectPtr<class UAnimSequence> AnimStare;
	UPROPERTY() TObjectPtr<class UAnimSequence> AnimPlaying;
	bool bRigged = false;
};
