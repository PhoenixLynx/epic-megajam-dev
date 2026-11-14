// TAKE 4471 - UI built entirely in C++ (no Widget Blueprints needed):
//   UTakeIndexedButton  - a button that reports its index when clicked
//   UTakePromptWidget   - "E  Use terminal" prompt under the crosshair
//   UTakeTerminalWidget - HALCYON Mail on desk #4471 (reads /Game/Data/DT_Emails)
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "TakeKeyLockLibrary.h"
#include "TakeWidgets.generated.h"

class UTextBlock;
class UScrollBox;
class UVerticalBox;
class UHorizontalBox;
class UWidgetSwitcher;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTakeIndexedClicked, int32, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTakeLogoutRequested);

UCLASS()
class DIRECTORSCUT_58_API UTakeIndexedButton : public UButton
{
	GENERATED_BODY()
public:
	UPROPERTY() int32 Index = 0;
	UPROPERTY(BlueprintAssignable) FTakeIndexedClicked OnIndexedClicked;
	void Init(int32 InIndex);
private:
	UFUNCTION() void HandleClicked();
};

UCLASS()
class DIRECTORSCUT_58_API UTakePromptWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetPrompt(const FString& Text);
	/** Bottom-of-screen subtitle for whoever is talking. Empty clears it. */
	void SetSubtitle(const FString& Text);
	/** Badge task list, top left. Empty hides it. */
	void SetTasks(const FString& Text);
	/** Clock / take / passes, top right. */
	void SetClock(const FString& Text);
	/** Full-screen slate for the cold open and "CUT. AGAIN.": black (0..1) with big white text on top (0..1). */
	void SetSlate(float BlackAlpha, const FString& Text, float TextAlpha);
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
private:
	UPROPERTY() TObjectPtr<UTextBlock> Label;
	UPROPERTY() TObjectPtr<UTextBlock> Subtitle;
	UPROPERTY() TObjectPtr<UTextBlock> TaskText;
	UPROPERTY() TObjectPtr<UTextBlock> ClockText;
	FString CurrentClock;
	UPROPERTY() TObjectPtr<class UBorder> TaskPanel;
	UPROPERTY() TObjectPtr<class UBorder> SlateBlack;
	UPROPERTY() TObjectPtr<UTextBlock> SlateText;
	FString CurrentSlate;
	FString Current;
	FString CurrentTasks;
};

UCLASS()
class DIRECTORSCUT_58_API UTakeTerminalWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable) FTakeLogoutRequested OnLogoutRequested;
	UFUNCTION() void ShowEmail(int32 Index);
	UFUNCTION() void ShowTab(int32 Tab);            // 0 = Inbox, 1 = Training
	UFUNCTION() void TrainingNav(int32 Direction);  // -1 back, +1 next
	UFUNCTION() void ToggleFragment(int32 Fragment);
	UFUNCTION() void AnswerQuiz(int32 Option);
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
private:
	struct FMail { FString Sender, Subject, Body; bool bPhishing = false; };
	struct FSlide { FString Title, Body, Kind; TArray<FString> Options; };
	TArray<FSlide> Slides;
	FTakeKeyLock Lock;
	TArray<int32> ActiveFragments;
	bool bLockSolved = false;
	FString QuizFeedback;
	UPROPERTY() TObjectPtr<UWidgetSwitcher> Pages;
	UPROPERTY() TObjectPtr<UVerticalBox> TrainingBox;
	UPROPERTY() TObjectPtr<UTextBlock> TrainingTabLabel;
	UPROPERTY() TArray<TObjectPtr<UTexture2D>> GlyphLit;
	UPROPERTY() TArray<TObjectPtr<UTexture2D>> GlyphUnlit;
	void LoadTraining();
	void RebuildSlide();
	void FinishTraining();
	int32 GetSlide() const;
	void SetSlide(int32 Slide);
	UHorizontalBox* MakeGlyphRow(int32 Value, int32 Bits, float Size, float Opacity = 1.f);
	UTakeIndexedButton* MakeButton(const FString& Text, int32 Index, const FLinearColor& Fill, const FLinearColor& TextColor);
	TArray<FMail> Mails;
	UPROPERTY() TObjectPtr<UTextBlock> Reader;
	UPROPERTY() TObjectPtr<UTextBlock> ReaderHeader;
	void LoadMail();
	UTextBlock* MakeText(const FString& Text, int32 Size, const FLinearColor& Color, bool bBold = false);
};

/** Full-screen look at a picture or paper on the wall (the Chairman's portrait, posters, the call sheet, fridge notes). */
UCLASS()
class DIRECTORSCUT_58_API UTakeInspectWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable) FTakeLogoutRequested OnCloseRequested;
	void Setup(UTexture2D* InTexture, const FString& InTitle, const FString& InCaption);
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
private:
	UPROPERTY() TObjectPtr<UTexture2D> Picture;
	FString TitleText;
	FString CaptionText;
};
