// TAKE 4471 - a mannequin coworker. Looks busy: types at its desk, wanders to the coffee
// machine / printer / water cooler (TargetPoints tagged WorkSpot), and comes back.
// Bump into it or press E and it says a random line from /Game/Data/DT_NPCLines for its Role.
// Above Fear 0.75 it stops pretending: it freezes while you watch and creeps closer when you don't.
// place_cpp_actors.py swaps the dressed NPC mannequins for these.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TakeCoworker.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;
class UStaticMesh;

UENUM(BlueprintType)
enum class ETakeCoworkerState : uint8 { Working, Walking, AtSpot, Returning, Watching };

UCLASS()
class DIRECTORSCUT_58_API ATakeCoworker : public ACharacter
{
	GENERATED_BODY()

public:
	ATakeCoworker();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coworker") TObjectPtr<UStaticMeshComponent> Body;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coworker") TObjectPtr<UTextRenderComponent> Speech;

	/** SM_Mannequin / SM_MannequinGuard / SM_MannequinCustodian (pivot at the feet). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") TObjectPtr<UStaticMesh> BodyMesh;
	/** Coworker, Guard, Custodian, Receptionist (matches the Role column of DT_NPCLines). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") FString NPCRole = TEXT("Coworker");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") bool bWanders = true;
	/** A malware agent: its lines get stranger, and it watches you sooner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") bool bIsAgent = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") float WalkSpeed = 120.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") float CreepSpeed = 70.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") float WorkMin = 8.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") float WorkMax = 25.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") float SpeechSeconds = 3.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") float SpeechCooldown = 4.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") float WatchFear = 0.75f;

	UPROPERTY(BlueprintReadOnly, Category = "Coworker") ETakeCoworkerState State = ETakeCoworkerState::Working;

	/** Say a random line (bump, E, or scripted). Returns false while on cooldown. */
	UFUNCTION(BlueprintCallable, Category = "Coworker") bool Speak(const FString& Context = TEXT(""));
	UFUNCTION(BlueprintCallable, Category = "Coworker") void SayLine(const FString& Line);

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	FVector Home;
	FRotator HomeRot;
	FVector Target;
	float StateTimer = 0.f;
	float SpeechLeft = 0.f;
	float CooldownLeft = 0.f;
	float Phase = 0.f;
	float StuckTimer = 0.f;
	FVector LastPos;
	TArray<FString> Lines;          // small talk, cycled without repeats
	TArray<FString> Greetings;      // first time you meet
	TArray<FString> CoffeeLines;    // once you've had your coffee
	TArray<int32> Bag;              // shuffled order of Lines still to say
	int32 LastLine = -1;
	bool bGreeted = false;
	FString NextLine();
	TArray<TWeakObjectPtr<AActor>> Spots;
	TWeakObjectPtr<class ATakeFearDirector> FearDirector;

	void LoadLines();
	void GoTo(const FVector& Where, ETakeCoworkerState NewState);
	bool PickSpot(FVector& Out);
	float GetFear() const;
	bool PlayerCanSee(const APawn* Player) const;
	void Animate(float DeltaSeconds, bool bMoving);
};
