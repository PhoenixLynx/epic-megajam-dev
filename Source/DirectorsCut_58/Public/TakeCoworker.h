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
class UAnimSequence;

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") float WatchFear = 0.35f;
	/** Above this Fear they vanish the moment you look away (and come back when it passes). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") float VanishFear = 0.85f;
	/** Part of a quest / story beat: shows a "!" and is the only kind you talk to with E. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") bool bStoryNPC = false;
	/** Turn of the rigged mannequin inside the character (degrees). If they face the wrong way, change this
	 *  (or type TakeNPCYaw 90 / 180 / 0 in the console during Play to try it on everyone). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") float MeshYaw = -90.f;
	UFUNCTION(BlueprintCallable, Category = "Coworker") void SetMeshYaw(float Yaw);
	/** Sits at a desk typing (never wanders). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") bool bSitsAtDesk = false;
	/** Talk on their own when you walk past (no E needed). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") bool bTalksWhenNear = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") float NearRange = 260.f;
	/** Seconds before this NPC talks on its own again. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") float NearCooldownMin = 18.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coworker") float NearCooldownMax = 35.f;

	UPROPERTY(BlueprintReadOnly, Category = "Coworker") ETakeCoworkerState State = ETakeCoworkerState::Working;

	/** Say a random line (bump, E, or scripted). Returns false while on cooldown. */
	UFUNCTION(BlueprintCallable, Category = "Coworker") bool Speak(const FString& Context = TEXT(""));
	UFUNCTION(BlueprintCallable, Category = "Coworker") void SayLine(const FString& Line);
	/** Something you threw hit them. */
	void HitByProp(float Speed);
	/** After HitByProp, when the thing was a full mug: they get coffee on them, and HR counts it twice. */
	void SplashedByCoffee();
	/** E on a story NPC: the next line of its current conversation (DT_NPCLines Context Story1, Story2, ...;
	 *  Pass1Story1.. once you hold pass 1, DoneStory1.. once the morning is done, SlipStory1.. after the 10:00 slip).
	 *  Once a conversation is finished, E gives small talk. Non-story NPCs just Speak(). */
	UFUNCTION(BlueprintCallable, Category = "Coworker") bool Talk();
	/** Part-way through a conversation (the prompt says "E  Continue"). */
	UFUNCTION(BlueprintPure, Category = "Coworker") bool IsInConversation() const { return bInConversation; }
	/** Has a conversation you haven't heard yet (shows the "!"). */
	UFUNCTION(BlueprintPure, Category = "Coworker") bool HasNewConversation() const;
	/** You picked up their monitor / keyboard / mug: they turn, frown and say something. */
	void ReactToTheft();
	/** The wrap party: stand here, look at the guest of honour, smile. */
	void JoinParty(const FVector& Where, const FVector& LookAt);
	bool bAtParty = false;
	bool bPartyChatted = false;   // you've talked to them at the party
	bool bPartyReady = false;     // the Collector: ready for his question
	void StartMingling();         // after the speeches: walk around the party

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
	float NearCooldownLeft = 0.f;
	FString SpokenLine;
	static double NextAnyoneMayTalk;   // only one NPC talks at a time
	static FString WrapLine(const FString& In, int32 Width);
	void UpdateOverhead(const FVector& Eye);
	// Rigged mannequin (if /Game/Characters/Mannequin was imported); otherwise the static Body is used
	bool bSkeletal = false;
	UPROPERTY() TObjectPtr<UAnimSequence> AnimIdle;
	UPROPERTY() TObjectPtr<UAnimSequence> AnimWalk;
	UPROPERTY() TObjectPtr<UAnimSequence> AnimSit;
	UPROPERTY() TObjectPtr<UAnimSequence> AnimStare;
	UPROPERTY() TObjectPtr<UAnimSequence> AnimPlaying;
	void SetupSkeletal();
	void PlayAnim(UAnimSequence* Anim);
	float Phase = 0.f;
	float StuckTimer = 0.f;
	FVector LastPos;
	TArray<FString> Lines;          // small talk, cycled without repeats
	TArray<FString> Greetings;      // first time you meet
	TArray<FString> CoffeeLines;    // once you've had your coffee
	TArray<FString> HitLines;       // when you throw something at them
	double LastHitTime = -10.0;
	TArray<int32> Bag;              // shuffled order of Lines still to say
	int32 LastLine = -1;
	bool bGreeted = false;
	bool bVanished = false;
	int32 LinesAct = 1;             // which act's small talk is loaded (switches at lunch, beat 1.10)
	float ActCheckIn = 0.f;
	bool bLinesClocksFrozen = false;  // the 2:14 lines are in
	// Story conversations: set name ("", "Pass1", "Done", "Slip", "Lunch") -> lines in order
	TMap<FString, TArray<FString>> StorySets;
	TSet<FString> FinishedSets;
	FString ActiveSet;
	int32 StoryIndex = 0;
	bool bInConversation = false;
	bool CurrentStorySet(FString& OutSet) const;
	FString NextLine();
	TArray<TWeakObjectPtr<AActor>> Spots;
	TWeakObjectPtr<class ATakeFearDirector> FearDirector;

	void LoadLines();
	void GoTo(const FVector& Where, ETakeCoworkerState NewState);
	bool PickSpot(FVector& Out);
	float GetFear() const;
	bool PlayerCanSee(const APawn* Player) const;
	void Animate(float DeltaSeconds, bool bMoving);
	float WatchReleaseLeft = 0.f;   // seconds Fear must stay low before staring coworkers go back to work
	FVector StareSpot = FVector::ZeroVector;   // where they stopped when the staring began: they stay exactly there
	// Faces: calm normally, the Fear face while staring or just after being hit in Fear
	UPROPERTY() TObjectPtr<class UMaterialInterface> FaceCalm;
	UPROPERTY() TObjectPtr<class UMaterialInterface> FaceFear;
	UPROPERTY() TObjectPtr<class UMaterialInterface> FaceFrown;
	int32 FaceMode = -1;
	float FrownHold = 0.f;
	float TheftHold = 0.f;
	bool bPayWarned = false;   // security: "that's coming out of your pay"   // you took their computer: they stop typing and stare, then turn back to the desk
	float FaceFearHold = 0.f;
	void SetFace(bool bFear);
	FString TaskHint(const class UTakeGameInstance* GI) const;
	FString LastHint;
};
