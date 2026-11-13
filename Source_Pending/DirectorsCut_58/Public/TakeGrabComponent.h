// TAKE 4471 - grab, carry and throw physics props; reach becomes telekinesis with 1+ soul bits.
// Add to BP_FirstPersonCharacter (Add Component > Take Grab). Keys: E = grab / drop, Right Mouse = throw.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TakeGrabComponent.generated.h"

class UPhysicsHandleComponent;
class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeGrabEvent, UPrimitiveComponent*, Component);

UCLASS(ClassGroup = (Take), meta = (BlueprintSpawnableComponent, DisplayName = "Take Grab"))
class DIRECTORSCUT_58_API UTakeGrabComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTakeGrabComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab") float GrabRange = 300.f;
	/** Reach with 1+ soul bits (astral logic). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab") float TelekinesisRange = 800.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab") float HoldDistance = 180.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab") float ThrowStrength = 1800.f;
	/** Heavier than this (kg) needs 3 soul bits. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab") float MaxMassWithoutTelekinesis = 40.f;
	/** Bind E / Right Mouse automatically. Turn off if you wire your own input actions to TryGrab / Throw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab") bool bAutoBindKeys = true;

	UPROPERTY(BlueprintAssignable, Category = "Grab") FTakeGrabEvent OnGrabbed;
	UPROPERTY(BlueprintAssignable, Category = "Grab") FTakeGrabEvent OnThrown;

	/** Grab what you're looking at, or drop what you're holding. Returns true if now holding something. */
	UFUNCTION(BlueprintCallable, Category = "Grab") bool TryGrabOrDrop();
	UFUNCTION(BlueprintCallable, Category = "Grab") void Drop();
	UFUNCTION(BlueprintCallable, Category = "Grab") void Throw();
	UFUNCTION(BlueprintPure, Category = "Grab") bool IsHolding() const;
	UFUNCTION(BlueprintPure, Category = "Grab") float CurrentRange() const;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY() TObjectPtr<UPhysicsHandleComponent> Handle;
	int32 GetSoulBits() const;
	bool GetView(FVector& OutLoc, FRotator& OutRot) const;
	void BindKeys();
	void TryGrabOrDropInput() { TryGrabOrDrop(); }
};
