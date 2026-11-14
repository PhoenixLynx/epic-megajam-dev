// TAKE 4471 - title screen, the ending's choice, and the credits (built in C++, no Widget Blueprints)
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TakeWidgets.h"
#include "TakeAct3Widgets.generated.h"

class UTextBlock;
class UVerticalBox;

/** Title: Sol's lobby key art, TAKE 4471, Start / Continue / Quit. Broadcasts 0 start, 1 continue, 2 quit. */
UCLASS()
class DIRECTORSCUT_58_API UTakeTitleWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable) FTakeIndexedClicked OnChoice;
	UFUNCTION() void Pick(int32 Index) { OnChoice.Broadcast(Index); }
	bool bHasSave = false;
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};

/** HAND OVER YOUR KEYS?  [ YES ] [ NO ]  Broadcasts 0 yes, 1 no. */
UCLASS()
class DIRECTORSCUT_58_API UTakeChoiceWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable) FTakeIndexedClicked OnChoice;
	UFUNCTION() void Pick(int32 Index) { OnChoice.Broadcast(Index); }
	FString Question;
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};

/** Credits: scroll up over black; "Directed by" is left blank. Broadcasts 0 when done (or on click / key). */
UCLASS()
class DIRECTORSCUT_58_API UTakeCreditsWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable) FTakeIndexedClicked OnChoice;
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
private:
	UPROPERTY() TObjectPtr<class UCanvasPanelSlot> RollSlot;
	float T = 0.f;
	bool bDone = false;
	void Finish();
};
